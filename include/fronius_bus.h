/**
 * @file fronius_bus.h
 * @brief Shared Modbus bus for Fronius devices.
 *
 * @details
 * `FroniusBus` represents one physical Modbus bus — either a single RS485
 * serial port or one TCP connection. It owns the `modbus_t` context, runs
 * the connection/reconnection loop in a background thread, and serialises
 * register reads from every device on the bus through a transaction queue.
 *
 * All `FroniusDevice` instances that share a serial port share one
 * `FroniusBus`; devices on different ports or on TCP each get their own.
 *
 * Reads are submitted via `submit()`, which always returns immediately
 * with a `std::future`. The bus thread executes one transaction at a time
 * — `modbus_set_slave()` followed by `modbus_read_registers()` — and
 * fulfils the promise on completion or timeout.
 *
 * Devices register themselves via `registerDevice()` during construction.
 * `FroniusBus` holds only `weak_ptr`s; on connect/disconnect it walks the
 * registry and invokes `onBusConnected()` / `onBusDisconnected()` on each
 * live device.
 *
 * @note `FroniusBus` must always be heap-allocated via `std::shared_ptr`.
 */

#ifndef FRONIUS_BUS_H_
#define FRONIUS_BUS_H_

#include "fronius_device.h"
#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <atomic>
#include <condition_variable>
#include <expected>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <modbus.h>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

/**
 * @class FroniusBus
 * @brief Owns and manages one shared Modbus bus (RTU or TCP).
 *
 * Construct one instance per physical serial port or TCP endpoint and
 * share it among every `FroniusDevice` that communicates over it. Must
 * be heap-allocated via `std::shared_ptr`. Non-copyable, non-movable.
 */
class FroniusBus {
public:
  // -------------------------------------------------------------------------
  // Transaction — one Modbus register read submitted to the queue
  // -------------------------------------------------------------------------

  /**
   * @struct Transaction
   * @brief A single Modbus register-read request submitted to the bus queue.
   *
   * Created by device fetch methods and queued via `FroniusBus::submit()`.
   * The bus thread fulfils `promise` after the read completes or times out.
   *
   * @note `promise` is move-only; `Transaction` is therefore move-only too.
   */
  struct Transaction {
    /** @brief Modbus slave ID to address for this transaction. */
    int slaveId{0};

    /** @brief Starting Modbus register address to read from. */
    int startAddr{0};

    /** @brief Number of consecutive 16-bit registers to read. */
    int count{0};

    /**
     * @brief Destination buffer; the bus thread writes `count` words
     *        starting at `dest[startAddr]`.
     *
     * Must remain valid until the future is fulfilled. Typically points
     * into the device's own `regs_` buffer.
     */
    uint16_t *dest{nullptr};

    /**
     * @brief Per-transaction response timeout (whole seconds component).
     *
     * Copied from the device's `ModbusDeviceConfig::secTimeout` and applied
     * via `modbus_set_response_timeout()` before each read, so different
     * slaves on the same bus can run with different timeouts.
     */
    int secTimeout{0};

    /** @brief Per-transaction response timeout (microseconds, 0–999999). */
    int usecTimeout{200000};

    /**
     * @brief Fulfilled by the bus thread with the outcome of the read.
     *
     * On success: empty value. On failure: `ModbusError` describing the
     * error (transport, timeout, or bus shutdown).
     */
    std::promise<std::expected<void, ModbusError>> promise;
  };

  // -------------------------------------------------------------------------
  // Construction / destruction
  // -------------------------------------------------------------------------

  /**
   * @brief Construct a FroniusBus with the given bus-level configuration.
   *
   * Validates the configuration immediately.
   *
   * @param cfg  Bus-level configuration (transport, reconnect policy, debug).
   * @throws std::invalid_argument if `cfg.validate()` fails.
   */
  explicit FroniusBus(const ModbusBusConfig &cfg);

  /**
   * @brief Stop the bus thread, cancel pending transactions, close the
   *        Modbus context.
   */
  ~FroniusBus();

  // Non-copyable, non-movable.
  FroniusBus(const FroniusBus &) = delete;
  FroniusBus &operator=(const FroniusBus &) = delete;
  FroniusBus(FroniusBus &&) = delete;
  FroniusBus &operator=(FroniusBus &&) = delete;

  // -------------------------------------------------------------------------
  // Bus lifecycle
  // -------------------------------------------------------------------------

  /**
   * @brief Start the asynchronous connection loop in a background thread.
   *
   * Returns immediately after launching the thread. The bus attempts to
   * connect, notifies registered devices on success, and falls back to
   * the configured backoff on failure.
   *
   * Calling `connect()` more than once is a no-op (guarded internally).
   * Call after constructing all devices and registering all callbacks.
   */
  void connect();

  /**
   * @brief Returns true if the physical bus is currently connected.
   *
   * Per-device readiness is independent — use `FroniusDevice::isReady()`
   * to query whether a specific slave has completed its validation.
   */
  bool isConnected() const { return connected_.load(); }

  /**
   * @brief Force a reconnection attempt.
   *
   * Marks the bus as disconnected and wakes the connection loop. All
   * registered devices receive `onBusDisconnected()` followed by
   * `onBusConnected()` once the reconnection succeeds. No-op if already
   * disconnected. Safe to call from any thread.
   */
  void triggerReconnect();

  /**
   * @brief Schedule a per-device reconnect without touching the shared bus.
   *
   * Spawns a background thread that retries `device->onBusConnected()`
   * with exponential backoff (governed by the device's own
   * `ModbusDeviceConfig::reconnectDelay` parameters) until the device
   * becomes ready or the physical bus drops.
   *
   * Unlike `triggerReconnect()`, this does *not* close the serial port or
   * disturb other devices on the bus. No-op if a retry is already in
   * flight for the same device. Safe to call from any thread.
   *
   * @param device  The device to re-validate.
   */
  void scheduleDeviceRetry(std::shared_ptr<FroniusDevice> device);

  /**
   * @brief Returns the remote TCP endpoint of the active connection.
   *
   * Returns `{"", 0}` for RTU connections or when not connected.
   */
  FroniusTypes::RemoteEndpoint getRemoteEndpoint() const {
    return remoteEndpoint_;
  }

  // -------------------------------------------------------------------------
  // Device registry
  // -------------------------------------------------------------------------

  /**
   * @brief Register a device so it receives bus lifecycle notifications.
   *
   * Called by `FroniusDevice`-derived constructors via `weak_from_this()`.
   * `FroniusBus` holds only a `weak_ptr` — expired entries are pruned the
   * next time the bus walks the registry.
   *
   * @param device  Weak pointer to the device to register.
   */
  void registerDevice(std::weak_ptr<FroniusDevice> device);

  /**
   * @brief Remove a device from the bus registry.
   *
   * Removes the weak pointer for `device` from the registry and signals any
   * in-flight per-device retry loop for that device to exit at its next
   * wake. Safe to call even if the device is not currently registered or
   * has no retry in flight. Does not block on the retry loop's actual exit;
   * the retry thread, which holds its own `shared_ptr<FroniusDevice>`, will
   * unwind on its own and is joined by `~FroniusBus`.
   *
   * Intended use is in derived-master destructors so a soon-to-die device
   * stops generating retry work and log noise.
   *
   * @param device  Raw pointer used as the registry key (matches the
   *                identity captured in `registerDevice`'s weak_ptr).
   */
  void unregisterDevice(FroniusDevice *device);

  // -------------------------------------------------------------------------
  // Transaction queue
  // -------------------------------------------------------------------------

  /**
   * @brief Submit a register-read transaction to the bus queue.
   *
   * Non-blocking. The transaction is queued and a future is returned
   * immediately. The bus thread executes queued transactions in submission
   * order. Call `.get()` on the future when the result is needed.
   *
   * If the bus is disconnected at submission time the transaction is still
   * queued and will run when the bus reconnects. If the bus is shutting
   * down the promise is fulfilled immediately with a shutdown error.
   *
   * @param t  The transaction to submit. Moved into the queue.
   * @return   A future that becomes ready when the transaction has executed
   *           (or been cancelled).
   */
  std::future<std::expected<void, ModbusError>> submit(Transaction t);

  // -------------------------------------------------------------------------
  // Bus-level callback setters
  // -------------------------------------------------------------------------

  // -------------------------------------------------------------------------
  // Bus-level callback identity
  // -------------------------------------------------------------------------

  /**
   * @brief Opaque identifier for a registered bus-level callback.
   *
   * Returned by `addBusConnectCallback`, `addBusDisconnectCallback`, and
   * `addBusErrorCallback`. Pass it to `removeBusCallback` to deregister
   * the callback before the captured state is torn down.
   *
   * Zero is reserved as a sentinel meaning "no callback".
   */
  using CallbackId = uint64_t;

  /**
   * @brief Register a callback invoked when the physical bus connects.
   *
   * Fired after a successful `modbus_connect()` and before any device
   * validation begins. Multiple callbacks may be registered and are
   * invoked in registration order, so a `FroniusBus` shared between
   * several application masters can notify each of them.
   *
   * Returns a `CallbackId` that can be passed to `removeBusCallback` to
   * deregister the callback before its captured state is destroyed.
   * Callers that never need to deregister may ignore the returned value.
   *
   * @param cb  Callback to invoke on bus connect.
   * @return    Opaque identifier for use with `removeBusCallback`.
   * @note Invoked from the bus thread; keep the handler lightweight.
   */
  CallbackId addBusConnectCallback(std::function<void()> cb);

  /**
   * @brief Register a callback invoked when the bus connection is lost.
   *
   * Fired when a connection attempt fails or after `triggerReconnect()`.
   * Receives the seconds until the next reconnection attempt (with
   * exponential backoff applied if configured).
   *
   * @param cb  Callback receiving the reconnect delay in seconds.
   * @return    Opaque identifier for use with `removeBusCallback`.
   * @note Invoked from the bus thread; keep the handler lightweight.
   */
  CallbackId
  addBusDisconnectCallback(std::function<void(int reconnectDelay)> cb);

  /**
   * @brief Register a callback invoked on bus-level Modbus errors.
   *
   * Bus-level errors are transport failures (CRC errors, framing errors,
   * connection timeouts) that are not specific to any one slave. Per-device
   * errors are reported separately via `FroniusDevice::setDeviceErrorCallback`.
   *
   * @param cb  Callback receiving a `ModbusError` describing the failure.
   * @return    Opaque identifier for use with `removeBusCallback`.
   */
  CallbackId addBusErrorCallback(std::function<void(const ModbusError &)> cb);

  /**
   * @brief Remove a previously-registered bus-level callback.
   *
   * Synchronously removes the callback with the given `id`. If the bus
   * thread is currently iterating the relevant callback list at the time
   * of the call, this method blocks until the iteration completes — so
   * after `removeBusCallback` returns, the callback is guaranteed not to
   * run again, and state captured by the callback's lambda can safely be
   * destroyed.
   *
   * No-op if the id is zero, was never returned by an `addBusXxxCallback`,
   * or has already been removed.
   *
   * @warning Must not be called from inside a bus callback — that would
   *          attempt to re-acquire the callback mutex held by the bus
   *          thread's fire loop and deadlock.
   *
   * @param id  The identifier returned by an `addBusXxxCallback` call.
   */
  void removeBusCallback(CallbackId id);

  /**
   * @brief Register a single callback for internal bus diagnostic messages.
   *
   * Receives a formatted log line (no trailing newline) for every queue
   * dequeue, transaction send, response, slave switch, etc. Intended for
   * wiring to an application logger such as spdlog.
   *
   * Unlike the other callback setters, only the first registered callback
   * is retained; subsequent calls are no-ops. Bus log output is high-volume
   * — a single sink is usually what you want.
   *
   * @param cb Callback receiving the log message string.
   */
  void addBusLogCallback(std::function<void(const std::string &)> cb) {
    if (!onBusLog_.empty())
      return;
    onBusLog_.push_back(std::move(cb));
  }

private:
  // -------------------------------------------------------------------------
  // Configuration and libmodbus context
  // -------------------------------------------------------------------------

  /** @brief Bus-level configuration (transport, reconnect policy, debug). */
  const ModbusBusConfig cfg_;

  /**
   * @brief libmodbus context.
   *
   * Created and owned exclusively by the bus thread. Must never be accessed
   * from any other thread. All reads go through the transaction queue.
   */
  modbus_t *ctx_{nullptr};

  /** @brief Remote TCP endpoint, captured after a successful TCP connect. */
  FroniusTypes::RemoteEndpoint remoteEndpoint_;

  // -------------------------------------------------------------------------
  // Connection thread state
  // -------------------------------------------------------------------------

  /** @brief Background thread running the connection loop and queue drainer. */
  std::thread busThread_;

  /** @brief Mutex protecting `cv_`, `connected_`, and the transaction queue. */
  mutable std::mutex mtx_;

  /** @brief Condition variable for waking the bus thread. */
  std::condition_variable cv_;

  /** @brief Set to true while the bus thread should keep running. */
  std::atomic<bool> running_{false};

  /** @brief True when the physical transport is connected. */
  std::atomic<bool> connected_{false};

  // -------------------------------------------------------------------------
  // Device registry
  // -------------------------------------------------------------------------

  /**
   * @brief Weak pointers to all registered devices.
   *
   * Protected by `mtx_`. Expired entries are pruned during each walk.
   */
  std::vector<std::weak_ptr<FroniusDevice>> devices_;

  /**
   * @brief Per-device retry tracking entry.
   *
   * One entry is created every time `scheduleDeviceRetry` launches a new
   * `deviceConnectLoop`. `device` is the raw pointer used to match the
   * entry against the device that owns the retry; `thread` is the jthread
   * executing the loop; `cancelled` is set when the device has been
   * unregistered (or the bus is tearing down) so the loop exits at its
   * next wake without calling `onBusConnected()` again.
   *
   * Entries are never removed by the loop itself — self-removal would
   * require joining the running thread. Instead, finished entries are
   * reaped opportunistically by `scheduleDeviceRetry` and
   * `unregisterDevice` (so the list does not grow unboundedly) and the
   * destructor of `FroniusBus` joins any that are still in flight.
   */
  struct RetryEntry {
    FroniusDevice *device{nullptr};
    std::atomic<bool> cancelled{false};
    std::atomic<bool> finished{false};
    std::jthread thread;
  };

  /**
   * @brief All retry entries — both running and finished-but-not-reaped.
   *
   * Protected by `mtx_`. We use `std::list` so iterators and `RetryEntry`
   * references remain valid across insertions and erasures of other
   * elements; this matters because the running jthread holds a reference
   * to its own entry's `cancelled` flag and we must not invalidate it
   * while reaping other finished entries.
   */
  std::list<std::unique_ptr<RetryEntry>> retries_;

  // -------------------------------------------------------------------------
  // Transaction queue
  // -------------------------------------------------------------------------

  /**
   * @brief Queue of pending read transactions.
   *
   * Protected by `mtx_`. The bus thread is the sole consumer; any thread
   * may produce via `submit()`.
   */
  std::queue<Transaction> txQueue_;

  /**
   * @brief Slave ID of the most recently executed transaction.
   *
   * Used on RTU buses to detect slave switches and insert a settle delay
   * before addressing a different slave. Zero until the first transaction.
   */
  int lastSlaveId_{0};

  // -------------------------------------------------------------------------
  // Bus-level callbacks
  // -------------------------------------------------------------------------

  /**
   * @brief A registered callback together with its identifier.
   *
   * `id` is opaque to the bus thread; it exists so external callers can
   * later remove the callback via `removeBusCallback`. `fn` is the actual
   * callable.
   */
  template <typename F> struct CallbackEntry {
    CallbackId id;
    std::function<F> fn;
  };

  /**
   * @brief Source of fresh `CallbackId`s.
   *
   * Starts at 1; zero is reserved as a sentinel. Incremented atomically
   * inside the cbMutex_-protected registration methods, but kept atomic
   * defensively so reading the next id outside the lock is safe.
   */
  std::atomic<CallbackId> nextCallbackId_{1};

  /**
   * @brief Mutex guarding the three callback vectors below.
   *
   * Held by `addBusXxxCallback` and `removeBusCallback` during list
   * mutation, and by the bus thread for the duration of each fire loop.
   * Holding it across the fire loop is what gives `removeBusCallback`
   * its synchronous-with-in-flight-invocation guarantee.
   *
   * Distinct from `mtx_` so the bus's queue, retry, and connection
   * state are not held hostage by a slow callback.
   */
  mutable std::mutex cbMutex_;

  /** @brief Fired when the physical bus connects. */
  std::vector<CallbackEntry<void()>> onBusConnect_;

  /** @brief Fired when the physical bus disconnects. Carries reconnect delay
   *         in seconds. */
  std::vector<CallbackEntry<void(int)>> onBusDisconnect_;

  /** @brief Fired on bus-level Modbus errors. */
  std::vector<CallbackEntry<void(const ModbusError &)>> onBusError_;

  /** @brief Fired on bus log messages */
  std::vector<std::function<void(const std::string &)>> onBusLog_;

  // -------------------------------------------------------------------------
  // Private methods — run exclusively on the bus thread
  // -------------------------------------------------------------------------

  /**
   * @brief Bus-thread entry point: runs the connect / drain / reconnect loop.
   */
  void busLoop();

  /**
   * @brief Attempt a single connection to the Modbus bus.
   *
   * Creates the `modbus_t` context, applies the debug flag, opens the
   * transport, and flushes stale RTU bytes.
   *
   * @return Empty expected on success, `ModbusError` on failure.
   */
  std::expected<void, ModbusError> tryConnect();

  /**
   * @brief Establish a Modbus TCP connection.
   * @param host  Hostname or IP address.
   * @param port  TCP port (default 502).
   */
  std::expected<void, ModbusError> connectTcp(const std::string &host,
                                              int port);

  /**
   * @brief Establish a Modbus RTU serial connection.
   * @param device    Serial device path (e.g. "/dev/ttyUSB0").
   * @param baud      Baud rate.
   * @param parity    Parity character ('N', 'E', or 'O').
   * @param dataBits  Number of data bits (5–8).
   * @param stopBits  Number of stop bits (1 or 2).
   */
  std::expected<void, ModbusError> connectRtu(const std::string &device,
                                              int baud, char parity,
                                              int dataBits, int stopBits);

  /**
   * @brief Drain the transaction queue while the bus stays connected.
   *
   * Pops one transaction at a time, executes it, and fulfils its promise
   * before moving on. Returns when the bus disconnects or `running_` is
   * cleared.
   */
  void drainQueue();

  /**
   * @brief Execute a single transaction on the bus.
   *
   * Sets the slave ID, applies the per-transaction response timeout,
   * performs the register read, and fulfils the transaction's promise.
   * On RTU buses inserts a settle delay if the slave ID changed since the
   * previous transaction.
   *
   * @param t  The transaction to execute.
   */
  void executeTransaction(Transaction &t);

  /**
   * @brief Cancel all queued transactions with a shutdown error.
   *
   * Invoked during destruction or after a disconnect so that callers
   * blocked on `future.get()` are unblocked promptly.
   */
  void cancelPendingTransactions();

  /**
   * @brief Notify all registered devices that the bus has connected.
   *
   * Walks `devices_`, prunes expired entries, and schedules a per-device
   * retry for each live device.
   */
  void notifyDevicesConnected();

  /**
   * @brief Notify all registered devices that the bus has disconnected.
   */
  void notifyDevicesDisconnected();

  /**
   * @brief Per-device reconnect loop, executed by a tracked jthread.
   *
   * Calls `device->onBusConnected()` and retries with exponential backoff
   * until the device becomes ready, its `cancelled` flag is set, or the
   * bus drops. On exit sets `entry.finished = true`; it does *not* remove
   * itself from `retries_` (a self-running jthread cannot join itself).
   * Finished entries are reaped opportunistically by `scheduleDeviceRetry`
   * and `unregisterDevice`, and unconditionally by `~FroniusBus`.
   *
   * @param device  The device to re-validate. The shared_ptr is captured by
   *                the jthread to keep the device alive for the loop's
   *                lifetime.
   * @param entry   Reference to the retry entry that owns this jthread.
   */
  void deviceConnectLoop(std::shared_ptr<FroniusDevice> device,
                         RetryEntry &entry);

  /**
   * @brief Format a log message and dispatch it to the registered log sink.
   */
  template <typename... Args>
  void busLog(std::format_string<Args...> fmt, Args &&...args) {
    if (onBusLog_.empty())
      return;
    std::string msg = std::format(fmt, std::forward<Args>(args)...);
    for (auto &cb : onBusLog_)
      cb(msg);
  }
};

#endif /* FRONIUS_BUS_H_ */
