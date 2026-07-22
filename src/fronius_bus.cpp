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
  // Signal the bus thread to stop and wake it up in case it is waiting
  // on the condition variable. The same cv_ is used by per-device retry
  // loops, so this also wakes them.
  {
    std::lock_guard<std::mutex> lock(mtx_);
    running_.store(false);
    // Set cancelled on every retry entry so any loop currently inside
    // device->onBusConnected() will short-circuit at its next predicate
    // check rather than retry again.
    for (auto &entry : retries_)
      entry->cancelled.store(true);
    cv_.notify_all();
  }

  if (busThread_.joinable())
    busThread_.join();

  // Join the connect-notify thread before touching retries_ or freeing this
  // object. With running_ already false, any scheduleDeviceRetry() it runs
  // returns without queuing new work, so the join is quick; without it the
  // detached thread could lock mtx_ after it is destroyed (use-after-free at
  // shutdown). Joined before the retries_ move below so any retries it did
  // schedule are captured and joined too.
  if (notifyConnectThread_.joinable())
    notifyConnectThread_.join();

  // Move the retry list out so jthread destructors join while no member
  // lock is held. The retry loops never modify retries_ themselves, so
  // there is no race between this move and an in-flight loop. The loops
  // do, however, acquire mtx_ briefly in cv_.wait_for predicate checks —
  // their condition is already satisfied (running_=false, cancelled=true),
  // so they exit promptly when they next get the lock.
  decltype(retries_) toJoin;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    toJoin = std::move(retries_);
  }
  // jthreads in toJoin join in their destructors as the list unwinds.
  toJoin.clear();

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

FroniusBus::CallbackId
FroniusBus::addBusConnectCallback(std::function<void()> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusConnect_.push_back({id, std::move(cb)});
  return id;
}

FroniusBus::CallbackId
FroniusBus::addBusDisconnectCallback(std::function<void(int)> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusDisconnect_.push_back({id, std::move(cb)});
  return id;
}

FroniusBus::CallbackId FroniusBus::addBusErrorCallback(
    std::function<void(const ModbusError &)> cb) {
  const CallbackId id = nextCallbackId_.fetch_add(1);
  std::lock_guard<std::mutex> lock(cbMutex_);
  onBusError_.push_back({id, std::move(cb)});
  return id;
}

void FroniusBus::removeBusCallback(CallbackId id) {
  if (id == 0)
    return;

  // Acquire cbMutex_. If the bus thread is currently inside a fire loop,
  // this blocks until that loop finishes, fulfilling the contract that
  // after this method returns, the callback is guaranteed not to run.
  std::lock_guard<std::mutex> lock(cbMutex_);

  // The id is unique across all three lists (nextCallbackId_ is shared),
  // so erase_if at most one will fire.
  auto matches = [id](const auto &entry) { return entry.id == id; };
  std::erase_if(onBusConnect_, matches);
  std::erase_if(onBusDisconnect_, matches);
  std::erase_if(onBusError_, matches);
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

        // Fire all registered bus-level connect callbacks. Lock cbMutex_
        // for the duration so removeBusCallback() callers (typically a
        // master in its destructor) synchronize against any in-flight
        // invocation here.
        {
          std::lock_guard<std::mutex> lock(cbMutex_);
          for (auto &entry : onBusConnect_)
            entry.fn();
        }

        // Reset backoff delay after a successful connection
        if (cfg_.exponential)
          reconnectDelay = cfg_.reconnectDelay;

        // Notify devices on a separate thread so the bus thread proceeds
        // directly to drainQueue(). onBusConnected() submits transactions
        // and blocks on future.get() — it must not run on the bus thread.
        // Tracked (not detached) so ~FroniusBus can join it; a reconnect
        // joins the previous one first.
        if (notifyConnectThread_.joinable())
          notifyConnectThread_.join();
        notifyConnectThread_ =
            std::thread([this] { notifyDevicesConnected(); });

      } else {
        // Connection failed
        connected_.store(false);

        // Report the bus error and notify with current reconnect delay
        // before backing off. Both fires share a single cbMutex_ lock so
        // we synchronize once against in-flight removeBusCallback callers.
        {
          std::lock_guard<std::mutex> lock(cbMutex_);
          for (auto &entry : onBusError_)
            entry.fn(res.error());
          for (auto &entry : onBusDisconnect_)
            entry.fn(reconnectDelay);
        }

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

      // Fire the disconnect callbacks under cbMutex_ so removeBusCallback()
      // synchronizes against the iteration. An empty list is fine — the
      // disconnect-then-reconnect contract is independent of whether any
      // application-level callback is registered.
      std::lock_guard<std::mutex> lock(cbMutex_);
      for (auto &entry : onBusDisconnect_)
        entry.fn(reconnectDelay);
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

  // RTU receive-buffer hygiene: stale bytes from a glitched or timed-out read
  // desync the bus by one frame (every reply reads as the previous request's),
  // and a transient error never reconnects to clear them. Flush before the send
  // -- not after the failed read, which only shifts the lag -- so the read
  // blocks for this request's own reply. On a healthy bus this is a no-op.
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

  busLog("[--] slave={} addr={} guard done, queue free", t.slaveId,
         t.startAddr);

  if (rc == -1) {
    auto err = ModbusError::fromErrno(
        "executeTransaction(): modbus_read_registers() failed "
        "[slave={}, addr={}, count={}]",
        t.slaveId, t.startAddr, t.count);

    // RECONNECT joins FATAL/SHUTDOWN: Phase 1 is the only place that
    // reopens the transport, so without this the bus spins on a dead fd.
    // No transport gate — a hung-up tty needs this as much as a reset
    // socket.
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
  RetryEntry *entryPtr = nullptr;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!running_.load() || !connected_.load())
      return;

    // Opportunistically reap finished entries so retries_ does not grow
    // unboundedly across many disconnect/reconnect cycles. Loops that
    // exited set finished=true; their jthreads have already returned,
    // so the join inside unique_ptr destruction is non-blocking.
    std::erase_if(retries_, [](const std::unique_ptr<RetryEntry> &e) {
      return e->finished.load();
    });

    // If a retry is already in flight for this device (an entry that has
    // not yet finished), nothing to do — the existing loop will observe
    // the (still-present) error state on its next iteration.
    for (auto &existing : retries_) {
      if (existing->device == device.get() && !existing->finished.load())
        return;
    }

    // Create the entry under the lock so unregisterDevice / ~FroniusBus
    // observe a fully-constructed entry whose `cancelled` flag they can
    // set safely. The jthread is constructed *after* the lock is released
    // so the loop can immediately acquire mtx_ at its first cv_.wait_for.
    auto entry = std::make_unique<RetryEntry>();
    entry->device = device.get();
    entryPtr = entry.get();
    retries_.push_back(std::move(entry));
  }

  // Launch outside the lock. Once spawned, the jthread owns its own
  // execution; the loop holds `device` (a shared_ptr) and a reference to
  // `*entryPtr` (alive for as long as the entry remains in retries_, which
  // is guaranteed because only ~FroniusBus or unregisterDevice removes
  // entries, and both either set cancelled first and join, or simply set
  // cancelled and leave the entry in place to be reaped later).
  std::jthread t(
      [this, device, entryPtr] { deviceConnectLoop(device, *entryPtr); });

  std::lock_guard<std::mutex> lock(mtx_);
  // Find the entry we created (it should still be there — only the
  // destructor moves entries out, and the destructor would have set
  // running_=false which would have aborted us above). If it is somehow
  // gone, joining `t` here is safe — it just blocks until the loop exits,
  // which is fast because cancelled would have been set.
  for (auto &existing : retries_) {
    if (existing.get() == entryPtr) {
      existing->thread = std::move(t);
      return;
    }
  }
  // Fallback: entry vanished. Joining here is fine.
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
        return !running_.load() || !connected_.load() ||
               entry.cancelled.load();
      });
    }

    if (cfg.exponential)
      delay = std::min(delay * 2, cfg.reconnectDelayMax);
  }

  // Mark the entry as finished so ~FroniusBus or the next scheduleDeviceRetry
  // can join and remove it. We do *not* erase ourselves: that would require
  // joining the running jthread, which a jthread cannot do to itself.
  entry.finished.store(true);

  // Wake anyone (notably ~FroniusBus on a fast path, or a near-simultaneous
  // scheduleDeviceRetry that is about to reap) so they observe the finish
  // promptly.
  cv_.notify_all();
}

void FroniusBus::unregisterDevice(FroniusDevice *device) {
  std::lock_guard<std::mutex> lock(mtx_);

  // Remove this device's weak_ptr from the registry so future
  // notifyDevicesConnected / notifyDevicesDisconnected walks skip it.
  // We match by lock()-and-compare because we only have the raw pointer.
  std::erase_if(devices_, [device](const std::weak_ptr<FroniusDevice> &wp) {
    auto sp = wp.lock();
    return !sp || sp.get() == device;
  });

  // Signal any in-flight retry loop for this device to exit at its next
  // wake. We do not join the entry here — joining under mtx_ would deadlock
  // with the loop's own mtx_ acquisition in cv_.wait_for. The reap below
  // picks up any entries that have already finished (typically prior
  // reconnect cycles); the one we just cancelled, if still running, will
  // be reaped by the next scheduleDeviceRetry call or by ~FroniusBus.
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
