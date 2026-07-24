#include "fronius_bus.h"
#include "fronius_device.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <modbus.h>
#include <mutex>
#include <thread>

/* -------------------------------------------------------------------------
   Construction / destruction
   ------------------------------------------------------------------------- */

FroniusBus::FroniusBus(const ModbusBusConfig &cfg) : cfg_(cfg) {
  cfg.validate();
}

FroniusBus::~FroniusBus() {
  // Stop the bus thread and cancel every retry loop, then wake both — they
  // share cv_.
  {
    std::lock_guard<std::mutex> lock(mtx_);
    running_.store(false);
    for (auto &entry : retries_)
      entry->cancelled.store(true);
    cv_.notify_all();
  }

  if (busThread_.joinable())
    busThread_.join();

  // Join before touching retries_: an unjoined notify thread could lock mtx_
  // after it is destroyed, and joining now also captures any retries it
  // scheduled.
  if (notifyConnectThread_.joinable())
    notifyConnectThread_.join();

  // Move the list out so the jthread destructors join with no member lock
  // held — the loops still take mtx_ for their final predicate check.
  decltype(retries_) toJoin;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    toJoin = std::move(retries_);
  }
  toJoin.clear();

  // Unblock callers waiting on transactions that were queued but never ran.
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
  // Guard against double-connect on a bus shared by several masters.
  if (running_.exchange(true))
    return;

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

FroniusBus::CallbackId
FroniusBus::addBusConnectCallback(std::function<void()> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusConnect_.push_back({id, std::move(cb)});
  return id;
}

FroniusBus::CallbackId
FroniusBus::addBusDisconnectCallback(std::function<void()> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusDisconnect_.push_back({id, std::move(cb)});
  return id;
}

FroniusBus::CallbackId
FroniusBus::addBusErrorCallback(std::function<void(const ModbusError &)> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusError_.push_back({id, std::move(cb)});
  return id;
}

FroniusBus::CallbackId
FroniusBus::addBusRetryCallback(std::function<void(int)> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusRetry_.push_back({id, std::move(cb)});
  return id;
}

void FroniusBus::removeBusCallback(CallbackId id) {
  if (id == 0)
    return;

  // Blocks if the bus thread is inside a fire loop, which is what makes the
  // "will not run again after this returns" guarantee hold.
  std::lock_guard<std::mutex> lock(cbMutex_);

  // nextCallbackId_ is shared, so the id is unique across all four lists.
  auto matches = [id](const auto &entry) { return entry.id == id; };
  std::erase_if(onBusConnect_, matches);
  std::erase_if(onBusDisconnect_, matches);
  std::erase_if(onBusRetry_, matches);
  std::erase_if(onBusError_, matches);
}

std::future<std::expected<void, ModbusError>>
FroniusBus::submit(Transaction t) {
  // Take the future before the move — it consumes the promise.
  auto future = t.promise.get_future();

  {
    std::lock_guard<std::mutex> lock(mtx_);

    if (!running_.load()) {
      // Shutting down: fail now rather than queue something that never runs.
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
        {
          std::lock_guard<std::mutex> lock(mtx_);
          connected_.store(true);
          cv_.notify_all();
        }

        // Held across the fire loop so removeBusCallback() synchronises
        // against an in-flight invocation.
        {
          std::lock_guard<std::mutex> lock(cbMutex_);
          for (auto &entry : onBusConnect_)
            entry.fn();
        }

        if (cfg_.exponential)
          reconnectDelay = cfg_.reconnectDelay;

        // onBusConnected() submits transactions and blocks on future.get(),
        // so it cannot run on the bus thread. Tracked, not detached, so the
        // destructor can join it; a reconnect joins the previous one first.
        if (notifyConnectThread_.joinable())
          notifyConnectThread_.join();
        notifyConnectThread_ =
            std::thread([this] { notifyDevicesConnected(); });

      } else {
        connected_.store(false);

        // The delay goes to onBusRetry_, not onBusDisconnect_: this is one
        // failed attempt, not the loss of the connection, which was already
        // reported when it happened.
        {
          std::lock_guard<std::mutex> lock(cbMutex_);
          for (auto &entry : onBusError_)
            entry.fn(res.error());
          for (auto &entry : onBusRetry_)
            entry.fn(reconnectDelay);
        }

        // Back off, or wake early on shutdown.
        {
          std::unique_lock<std::mutex> lock(mtx_);
          cv_.wait_for(lock, std::chrono::seconds(reconnectDelay),
                       [this] { return !running_.load(); });
        }

        if (cfg_.exponential)
          reconnectDelay = std::min(reconnectDelay * 2, cfg_.reconnectDelayMax);

        continue;
      }
    }

    // -----------------------------------------------------------------
    // Phase 2: drain the transaction queue while connected
    // -----------------------------------------------------------------
    drainQueue();

    // drainQueue() returns on disconnect or on shutdown. Every path that
    // clears connected_ while the loop runs passes through here, so this is
    // where onBusDisconnect_ fires — once per outage, carrying no delay.
    // See addBusDisconnectCallback().
    if (!connected_.load() && running_.load()) {
      cancelPendingTransactions();
      notifyDevicesDisconnected();

      {
        std::lock_guard<std::mutex> lock(cbMutex_);
        for (auto &entry : onBusDisconnect_)
          entry.fn();
      }
    }
  }

  // Shutdown: notify devices a final time. onBusDisconnect_ deliberately
  // does not fire — it would race with the removeBusCallback() calls the
  // consumers' destructors are making.
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
  // Discard any context left over from a previous attempt.
  if (ctx_) {
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }

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

  if (cfg_.debug) {
    if (modbus_set_debug(ctx_, true) == -1) {
      modbus_free(ctx_);
      ctx_ = nullptr;
      return std::unexpected(ModbusError::fromErrno(
          "tryConnect(): Unable to set the libmodbus debug flag"));
    }
  }

  if (modbus_connect(ctx_) == -1) {
    const std::string target =
        cfg_.isTcp() ? cfg_.tcp().host : cfg_.rtu().device;
    modbus_free(ctx_);
    ctx_ = nullptr;
    return std::unexpected(ModbusError::fromErrno(
        "tryConnect(): Connection to '{}' failed", target));
  }

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

    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] {
      return !txQueue_.empty() || !connected_.load() || !running_.load();
    });

    if (!connected_.load() || !running_.load())
      return;

    // Pop under the lock, execute without it, so submit() can keep
    // enqueueing while the bus is busy.
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

  // Stale RTU bytes from a glitched or timed-out read desync the bus by one
  // frame, and a transient error never reconnects to clear them. Flushing
  // before the send — not after the failed read, which only shifts the lag —
  // is what restores alignment. On a healthy bus it is a no-op.
  if (cfg_.isRtu())
    modbus_flush(ctx_);

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

  if (rc == -1) {
    auto err = ModbusError::fromErrno(
        "executeTransaction(): modbus_read_registers() failed "
        "[slave={}, addr={}, count={}]",
        t.slaveId, t.startAddr, t.count);

    // ETIMEDOUT is the one code whose meaning depends on the transport, and
    // this is the only site that produces it on an established one.
    //
    // On TCP it is the sole signal of a link-down: the kernel holds the
    // connection open, the timed-out request stays outstanding, and its late
    // reply answers the next one — leaving every read a frame behind until a
    // fresh connect(). On RTU it is the ordinary silent-slave case, and
    // reconnecting would drop a bus other devices share; the pre-send flush
    // clears the stale frame instead.
    //
    // Handled here rather than in deduceSeverity() because tryConnect() also
    // sees ETIMEDOUT on TCP, from an unanswered SYN, where it is a genuine
    // transient that Phase 1 already retries.
    if (cfg_.isTcp() && err.code == ETIMEDOUT) {
      busLog("[esc] slave={} addr={} -> TCP read timeout, escalating to "
             "RECONNECT",
             t.slaveId, t.startAddr);
      err.severity = ModbusError::Severity::RECONNECT;
    }

    // RECONNECT joins FATAL/SHUTDOWN: Phase 1 is the only place that reopens
    // the transport, and a hung-up tty needs that as much as a reset socket.
    if (err.severity == ModbusError::Severity::FATAL ||
        err.severity == ModbusError::Severity::SHUTDOWN ||
        err.severity == ModbusError::Severity::RECONNECT) {
      connected_.store(false);
      cv_.notify_all();
    }

    {
      std::lock_guard<std::mutex> lock(cbMutex_);
      for (auto &entry : onBusError_)
        entry.fn(err);
    }

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
  // Snapshot under the lock, call outside it — onBusConnected() reaches
  // submit(), which takes mtx_ too.
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
  RetryEntry *entryPtr = nullptr;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!running_.load() || !connected_.load())
      return;

    // Reap finished entries so retries_ does not grow across reconnect
    // cycles. Their jthreads have already returned, so the join inside
    // unique_ptr destruction is non-blocking.
    std::erase_if(retries_, [](const std::unique_ptr<RetryEntry> &e) {
      return e->finished.load();
    });

    // A retry already in flight will pick up the still-present error state
    // on its next iteration.
    for (auto &existing : retries_) {
      if (existing->device == device.get() && !existing->finished.load())
        return;
    }

    // Create the entry under the lock so unregisterDevice / ~FroniusBus see
    // a fully-constructed `cancelled` flag; the jthread is spawned after the
    // lock is released so the loop can take mtx_ immediately.
    auto entry = std::make_unique<RetryEntry>();
    entry->device = device.get();
    entryPtr = entry.get();
    retries_.push_back(std::move(entry));
  }

  // The loop holds `device` by shared_ptr and `*entryPtr` by reference. The
  // entry outlives it: only ~FroniusBus and unregisterDevice remove entries,
  // and both set `cancelled` first.
  std::jthread t(
      [this, device, entryPtr] { deviceConnectLoop(device, *entryPtr); });

  std::lock_guard<std::mutex> lock(mtx_);
  for (auto &existing : retries_) {
    if (existing.get() == entryPtr) {
      existing->thread = std::move(t);
      return;
    }
  }
  // Entry vanished: `t` joins on scope exit, promptly, since a removal
  // would have set `cancelled`.
}

void FroniusBus::deviceConnectLoop(std::shared_ptr<FroniusDevice> device,
                                   RetryEntry &entry) {
  const auto &cfg = device->getDeviceConfig();
  int delay = cfg.reconnectDelay;

  while (running_.load() && connected_.load() && !entry.cancelled.load()) {
    if (!device->isReady())
      device->onBusConnected();
    if (device->isReady())
      break;
    if (entry.cancelled.load())
      break;

    device->fireDeviceRetry(delay);

    {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait_for(lock, std::chrono::seconds(delay), [this, &entry] {
        return !running_.load() || !connected_.load() || entry.cancelled.load();
      });
    }

    if (cfg.exponential)
      delay = std::min(delay * 2, cfg.reconnectDelayMax);
  }

  // Mark reapable rather than self-erasing: removing the entry would mean
  // joining this jthread from itself.
  entry.finished.store(true);

  // Wake any waiting reaper so it observes the finish promptly.
  cv_.notify_all();
}

void FroniusBus::unregisterDevice(FroniusDevice *device) {
  std::lock_guard<std::mutex> lock(mtx_);

  // Drop the registry entry so later notify walks skip it. Matched by
  // lock()-and-compare, since only the raw pointer is available here.
  std::erase_if(devices_, [device](const std::weak_ptr<FroniusDevice> &wp) {
    auto sp = wp.lock();
    return !sp || sp.get() == device;
  });

  // Signal the retry loop rather than joining it: joining under mtx_ would
  // deadlock against the loop's own acquisition in cv_.wait_for. If it is
  // still running, the next scheduleDeviceRetry or ~FroniusBus reaps it.
  for (auto &entry : retries_) {
    if (entry->device == device)
      entry->cancelled.store(true);
  }

  // Reap entries whose loop has already exited.
  std::erase_if(retries_, [](const std::unique_ptr<RetryEntry> &e) {
    return e->finished.load();
  });

  cv_.notify_all();
}
