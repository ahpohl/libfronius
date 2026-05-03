#include "fronius_bus.h"
#include "fronius_device.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <modbus/modbus.h>
#include <mutex>
#include <thread>

/* -------------------------------------------------------------------------
   Construction / destruction
   ------------------------------------------------------------------------- */

FroniusBus::FroniusBus(const ModbusBusConfig &cfg) : cfg_(cfg) {
  cfg.validate();
}

FroniusBus::~FroniusBus() {
  // Signal the bus thread to stop and wake it up in case it is waiting
  // on the condition variable.
  {
    std::lock_guard<std::mutex> lock(mtx_);
    running_.store(false);
    cv_.notify_all();
  }

  if (busThread_.joinable())
    busThread_.join();

  // Cancel any transactions that were queued but never executed, so that
  // callers blocked on future.get() are unblocked immediately.
  cancelPendingTransactions();

  if (ctx_) {
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }
}

/* -------------------------------------------------------------------------
   Public API
   ------------------------------------------------------------------------- */

void FroniusBus::connect() {
  // Guard against double-connect (e.g. shared bus used by multiple masters)
  if (running_.exchange(true))
    return; // already started

  busThread_ = std::thread(&FroniusBus::busLoop, this);
}

void FroniusBus::triggerReconnect() {
  std::lock_guard<std::mutex> lock(mtx_);

  if (!connected_.load())
    return; // already disconnected, nothing to do

  connected_.store(false);
  cv_.notify_all(); // wake bus thread to re-enter connection logic
}

void FroniusBus::registerDevice(std::weak_ptr<FroniusDevice> device) {
  std::lock_guard<std::mutex> lock(mtx_);
  devices_.push_back(std::move(device));
}

std::future<std::expected<void, ModbusError>>
FroniusBus::submit(Transaction t) {
  // Extract the future from the promise before moving the transaction
  // into the queue — the promise is consumed by the move.
  auto future = t.promise.get_future();

  {
    std::lock_guard<std::mutex> lock(mtx_);

    if (!running_.load()) {
      // Bus is shutting down: fulfill immediately with a shutdown error
      // rather than queuing a transaction that will never execute.
      t.promise.set_value(std::unexpected(ModbusError::custom(
          EINTR, "submit(): Bus is shutting down, transaction cancelled")));
      return future;
    }

    txQueue_.push(std::move(t));
  }

  // Wake the bus thread so it picks up the new transaction promptly.
  cv_.notify_one();

  return future;
}

/* -------------------------------------------------------------------------
   Bus thread — busLoop
   ------------------------------------------------------------------------- */

void FroniusBus::busLoop() {
  int reconnectDelay = cfg_.reconnectDelay;

  while (running_.load()) {

    // -----------------------------------------------------------------
    // Phase 1: connect if not already connected
    // -----------------------------------------------------------------
    if (!connected_.load()) {
      auto res = tryConnect();

      if (res) {
        // Successful connection — update state and notify
        {
          std::lock_guard<std::mutex> lock(mtx_);
          connected_.store(true);
          cv_.notify_all();
        }

        // Fire all registered bus-level connect callbacks
        for (auto &cb : onBusConnect_)
          cb();

        // Reset backoff delay after a successful connection
        if (cfg_.exponential)
          reconnectDelay = cfg_.reconnectDelay;

        // Notify devices on a separate thread so the bus thread proceeds
        // directly to drainQueue(). onBusConnected() submits transactions
        // and blocks on future.get() — it must not run on the bus thread.
        std::thread([this] { notifyDevicesConnected(); }).detach();

      } else {
        // Connection failed
        connected_.store(false);

        // Report the bus error
        for (auto &cb : onBusError_)
          cb(res.error());

        // Notify with current reconnect delay before backing off
        for (auto &cb : onBusDisconnect_)
          cb(reconnectDelay);

        // Wait for the backoff period or until shutdown is requested
        {
          std::unique_lock<std::mutex> lock(mtx_);
          cv_.wait_for(lock, std::chrono::seconds(reconnectDelay),
                       [this] { return !running_.load(); });
        }

        // Apply exponential backoff for the next attempt
        if (cfg_.exponential)
          reconnectDelay = std::min(reconnectDelay * 2, cfg_.reconnectDelayMax);

        continue;
      }
    }

    // -----------------------------------------------------------------
    // Phase 2: drain the transaction queue while connected
    // -----------------------------------------------------------------
    drainQueue();

    // drainQueue() returns when either:
    // a) the bus disconnected (connected_ == false), or
    // b) the bus thread is being shut down (running_ == false)
    //
    // If the bus dropped while connected, notify devices and fire the
    // disconnect callback before looping back to Phase 1.
    if (!connected_.load() && running_.load()) {
      cancelPendingTransactions();
      notifyDevicesDisconnected();

      if (onBusDisconnect_.empty())
        return;
      for (auto &cb : onBusDisconnect_)
        cb(reconnectDelay);
    }
  }

  // Shut down: notify devices one final time so they can clean up
  if (connected_.load()) {
    connected_.store(false);
    cancelPendingTransactions();
    notifyDevicesDisconnected();
  }
}

/* -------------------------------------------------------------------------
   Connection helpers — called only from the bus thread
   ------------------------------------------------------------------------- */

std::expected<void, ModbusError> FroniusBus::tryConnect() {
  // Clean up any leftover context from a previous failed attempt
  if (ctx_) {
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }

  // Create the transport context
  if (cfg_.isTcp()) {
    const auto &t = cfg_.tcp();
    if (auto res = connectTcp(t.host, t.port); !res)
      return res;
  } else {
    const auto &r = cfg_.rtu();
    if (auto res =
            connectRtu(r.device, r.baud, r.parity, r.dataBits, r.stopBits);
        !res)
      return res;
  }

  // Enable debug logging if configured
  if (cfg_.debug) {
    if (modbus_set_debug(ctx_, true) == -1) {
      modbus_free(ctx_);
      ctx_ = nullptr;
      return std::unexpected(ModbusError::fromErrno(
          "tryConnect(): Unable to set the libmodbus debug flag"));
    }
  }

  // Open the connection
  if (modbus_connect(ctx_) == -1) {
    const std::string target =
        cfg_.isTcp() ? cfg_.tcp().host : cfg_.rtu().device;
    modbus_free(ctx_);
    ctx_ = nullptr;
    return std::unexpected(ModbusError::fromErrno(
        "tryConnect(): Connection to '{}' failed", target));
  }

  // Capture remote endpoint info for TCP connections
  if (cfg_.isTcp()) {
    int sock = modbus_get_socket(ctx_);
    if (sock == -1) {
      modbus_free(ctx_);
      ctx_ = nullptr;
      return std::unexpected(ModbusError::fromErrno(
          "tryConnect(): Failed to get socket from libmodbus context"));
    }
    remoteEndpoint_ = ModbusUtils::getSocketInfo(sock);
  }

  // Flush stale bytes from the RTU receive buffer before the first
  // transaction, then wait briefly for the bus to settle.
  if (cfg_.isRtu()) {
    modbus_flush(ctx_);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return {};
}

std::expected<void, ModbusError> FroniusBus::connectTcp(const std::string &host,
                                                        int port) {
  ctx_ = modbus_new_tcp_pi(host.c_str(), std::to_string(port).c_str());

  if (!ctx_)
    return std::unexpected(ModbusError::custom(
        ENOMEM, "connectTcp(): Unable to create libmodbus TCP context"));

  return {};
}

std::expected<void, ModbusError>
FroniusBus::connectRtu(const std::string &device, int baud, char parity,
                       int dataBits, int stopBits) {
  ctx_ = modbus_new_rtu(device.c_str(), baud, parity, dataBits, stopBits);

  if (!ctx_)
    return std::unexpected(ModbusError::custom(
        ENOMEM, "connectRtu(): Unable to create libmodbus RTU context for '{}'",
        device));

  return {};
}

/* -------------------------------------------------------------------------
   Transaction queue — called only from the bus thread
   ------------------------------------------------------------------------- */

void FroniusBus::drainQueue() {
  while (running_.load() && connected_.load()) {

    // Wait for a transaction to arrive or for a state change
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] {
      return !txQueue_.empty() || !connected_.load() || !running_.load();
    });

    // Exit immediately if the bus dropped or a shutdown was requested
    if (!connected_.load() || !running_.load())
      return;

    // Pop the next transaction under the lock, then release before
    // executing so that submit() can enqueue new transactions concurrently
    // while the bus is busy with the current one.
    Transaction t = std::move(txQueue_.front());
    txQueue_.pop();

    busLog("[queue] depth={} -> dequeued slave={} addr={}", txQueue_.size(),
           t.slaveId, t.startAddr);

    lock.unlock();

    executeTransaction(t);
  }
}

void FroniusBus::executeTransaction(Transaction &t) {
  if (cfg_.isRtu() && lastSlaveId_ != 0 && lastSlaveId_ != t.slaveId) {
    const int millis = 500;
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    busLog("[switch] slave id [{}->{}], sleep [{}ms]", lastSlaveId_, t.slaveId,
           millis);
  }
  lastSlaveId_ = t.slaveId;

  if (modbus_set_slave(ctx_, t.slaveId) == -1) {
    t.promise.set_value(std::unexpected(ModbusError::fromErrno(
        "executeTransaction(): modbus_set_slave({}) failed", t.slaveId)));
    return;
  }

  modbus_set_response_timeout(ctx_, t.secTimeout, t.usecTimeout);

  busLog("[tx] slave={} addr={} count={} -> sending", t.slaveId, t.startAddr,
         t.count);

  auto tStart = std::chrono::steady_clock::now();

  int rc =
      modbus_read_registers(ctx_, t.startAddr, t.count, t.dest + t.startAddr);

  auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - tStart)
                       .count();

  if (rc == -1) {
    busLog("[rx] slave={} addr={} -> FAIL ({}) [{}ms]", t.slaveId, t.startAddr,
           modbus_strerror(errno), elapsedMs);
  } else {
    busLog("[rx] slave={} addr={} -> ok [{}ms]", t.slaveId, t.startAddr,
           elapsedMs);
  }

  busLog("[--] slave={} addr={} guard done, queue free", t.slaveId,
         t.startAddr);

  if (rc == -1) {
    auto err = ModbusError::fromErrno(
        "executeTransaction(): modbus_read_registers() failed "
        "[slave={}, addr={}, count={}]",
        t.slaveId, t.startAddr, t.count);

    if (err.severity == ModbusError::Severity::FATAL ||
        err.severity == ModbusError::Severity::SHUTDOWN) {
      connected_.store(false);
      cv_.notify_all();
    }

    for (auto &cb : onBusError_)
      cb(err);

    t.promise.set_value(std::unexpected(std::move(err)));
    return;
  }

  t.promise.set_value({});
}

void FroniusBus::cancelPendingTransactions() {
  std::lock_guard<std::mutex> lock(mtx_);

  while (!txQueue_.empty()) {
    auto &t = txQueue_.front();
    t.promise.set_value(std::unexpected(ModbusError::custom(
        EINTR,
        "cancelPendingTransactions(): Transaction cancelled "
        "[slave={}, addr={}, count={}]",
        t.slaveId, t.startAddr, t.count)));
    txQueue_.pop();
  }
}

/* -------------------------------------------------------------------------
   Device registry notifications — called only from the bus thread
   -------------------------------------------------------------------------
 */

void FroniusBus::notifyDevicesConnected() {
  // Snapshot the live device pointers under the lock, then call callbacks
  // outside it — onBusConnected() calls submit() which also acquires mtx_.
  std::vector<std::shared_ptr<FroniusDevice>> live;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    devices_.erase(
        std::remove_if(devices_.begin(), devices_.end(),
                       [&live](const std::weak_ptr<FroniusDevice> &wp) {
                         auto device = wp.lock();
                         if (!device)
                           return true;
                         live.push_back(device);
                         return false;
                       }),
        devices_.end());
  }
  for (auto &device : live)
    scheduleDeviceRetry(device);
}

void FroniusBus::notifyDevicesDisconnected() {
  std::vector<std::shared_ptr<FroniusDevice>> live;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    devices_.erase(
        std::remove_if(devices_.begin(), devices_.end(),
                       [&live](const std::weak_ptr<FroniusDevice> &wp) {
                         auto device = wp.lock();
                         if (!device)
                           return true;
                         live.push_back(device);
                         return false;
                       }),
        devices_.end());
  }
  for (auto &device : live)
    device->onBusDisconnected();
}

void FroniusBus::scheduleDeviceRetry(std::shared_ptr<FroniusDevice> device) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (!running_.load() || !connected_.load())
    return;
  auto [it, inserted] = retriesInProgress_.insert(device.get());
  if (!inserted)
    return; // retry already in flight for this device
  std::thread([this, device] { deviceConnectLoop(device); }).detach();
}

void FroniusBus::deviceConnectLoop(std::shared_ptr<FroniusDevice> device) {
  const auto &cfg = device->getDeviceConfig();
  int delay = cfg.reconnectDelay;

  while (running_.load() && connected_.load()) {
    if (!device->isReady())
      device->onBusConnected();
    if (device->isReady())
      break;

    device->fireDeviceRetry(delay);

    {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait_for(lock, std::chrono::seconds(delay),
                   [this] { return !running_.load() || !connected_.load(); });
    }

    if (cfg.exponential)
      delay = std::min(delay * 2, cfg.reconnectDelayMax);
  }

  std::lock_guard<std::mutex> lock(mtx_);
  retriesInProgress_.erase(device.get());
}
