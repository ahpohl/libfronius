/**
 * @file fronius_device.h
 * @brief Abstract base class for all Fronius Modbus slave devices.
 *
 * @details
 * `FroniusDevice` is the base for every concrete device on a `FroniusBus`
 * (`Meter`, `Inverter`, …). It owns the per-device configuration and
 * register buffer, receives bus lifecycle notifications, decodes registers
 * into typed values, and routes errors through a device-level callback.
 *
 * @note Devices must always be heap-allocated via `std::shared_ptr` — they
 *       inherit `std::enable_shared_from_this` so they can register
 *       themselves with `FroniusBus` via `weak_from_this()`.
 */

#ifndef FRONIUS_DEVICE_H_
#define FRONIUS_DEVICE_H_

#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include "register_base.h"
#include <atomic>
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @class FroniusDevice
 * @brief Abstract base class for all Modbus slave devices on a Fronius bus.
 *
 * Subclasses (`Meter`, `Inverter`, …) must implement `onBusConnected()` —
 * to validate themselves and call `setReady()` or `setUnavailable()` — and
 * `onBusDisconnected()`, which should reset internal state and call
 * `setUnavailable()`.
 */
class FroniusDevice : public std::enable_shared_from_this<FroniusDevice> {
public:
  /**
   * @brief Construct a FroniusDevice with the given per-device configuration.
   *
   * @param cfg Per-device Modbus configuration (slave ID, response timeout).
   *
   * @note Derived class constructors must call
   *       `bus->registerDevice(weak_from_this())` after constructing the
   *       base, once `shared_from_this()` is safe to call.
   */
  explicit FroniusDevice(const ModbusDeviceConfig &cfg);

  /**
   * @brief Virtual destructor.
   */
  virtual ~FroniusDevice() = default;

  // Non-copyable, non-movable — shared ownership via shared_ptr only.
  FroniusDevice(const FroniusDevice &) = delete;
  FroniusDevice &operator=(const FroniusDevice &) = delete;
  FroniusDevice(FroniusDevice &&) = delete;
  FroniusDevice &operator=(FroniusDevice &&) = delete;

  // -------------------------------------------------------------------------
  // Bus lifecycle interface — called by FroniusBus
  // -------------------------------------------------------------------------

  /**
   * @brief Called by `FroniusBus` when the physical bus connection is
   *        established.
   *
   * Implementations should submit the transactions needed to validate and
   * identify the device (register map detection, common block fetch, etc.)
   * through the bus, then fire `onDeviceReady_` on success or
   * `onDeviceUnavailable_` on failure.
   *
   * This method is called from the `FroniusBus` connection thread. It must
   * not block the bus thread for long; submit transactions and let the queue
   * drain normally.
   */
  virtual void onBusConnected() = 0;

  /**
   * @brief Called by `FroniusBus` when the physical bus connection is lost.
   *
   * Implementations must reset their validated state so that the next
   * `onBusConnected()` triggers a full re-validation. They should also
   * fire `onDeviceUnavailable_` to notify the application.
   *
   * This method is called from the `FroniusBus` connection thread.
   */
  virtual void onBusDisconnected() = 0;

  // -------------------------------------------------------------------------
  // Device-level callback setters — called by the application
  // -------------------------------------------------------------------------

  /**
   * @brief Set a callback invoked when the device has been validated.
   *
   * Fired after a successful `onBusConnected()`. Receives the detected
   * register map.
   *
   * @param cb Callback receiving the detected `FroniusTypes::RegisterMap`.
   * @note Invoked from a `FroniusBus` thread; keep the handler lightweight.
   */
  void
  setDeviceReadyCallback(std::function<void(FroniusTypes::RegisterMap)> cb) {
    onDeviceReady_ = std::move(cb);
  }

  /**
   * @brief Set a callback invoked when the device becomes unavailable.
   *
   * Fired when validation fails or the bus drops.
   *
   * @param cb Callback to invoke on device unavailability.
   * @note Invoked from a `FroniusBus` thread; keep the handler lightweight.
   */
  void setDeviceUnavailableCallback(std::function<void()> cb) {
    onDeviceUnavailable_ = std::move(cb);
  }

  /**
   * @brief Set a callback to report device-level Modbus errors.
   *
   * Bus-level wire errors are reported separately via the `FroniusBus`
   * error callback.
   *
   * @param cb Callback receiving a `ModbusError` describing the failure.
   */
  void setDeviceErrorCallback(std::function<void(const ModbusError &)> cb) {
    onDeviceError_ = std::move(cb);
  }

  /**
   * @brief Set a callback invoked before each per-device reconnect delay.
   *
   * Receives the number of seconds until the next `onBusConnected()`
   * attempt, so the application can log or surface the backoff.
   *
   * @param cb Callback receiving the retry delay in seconds.
   */
  void setDeviceRetryCallback(std::function<void(int)> cb) {
    onDeviceRetry_ = std::move(cb);
  }

  /**
   * @brief Fire the retry callback before a per-device reconnect delay.
   *
   * Called by `FroniusBus::deviceConnectLoop` — not part of the application
   * API; exposed publicly because `FroniusBus` is not a friend.
   *
   * @param delay Seconds until the next `onBusConnected()` attempt.
   */
  void fireDeviceRetry(int delay) {
    if (onDeviceRetry_)
      onDeviceRetry_(delay);
  }

  // -------------------------------------------------------------------------
  // Device state queries
  // -------------------------------------------------------------------------

  /**
   * @brief Returns true if the device has been successfully validated and
   *        is ready to serve data.
   *
   * Unlike `FroniusBus::isConnected()`, which reflects the physical bus
   * state, `isReady()` reflects whether *this specific slave* has completed
   * its register map validation. The bus can be connected while a device is
   * not yet ready (validation pending or failed).
   */
  bool isReady() const { return ready_.load(); }

  /**
   * @brief Returns the register map detected during the last successful
   *        validation, or `RegisterMap::UNAVAILABLE` if not yet validated.
   */
  FroniusTypes::RegisterMap getRegisterMap() const { return registerMap_; }

  /**
   * @brief Returns the per-device Modbus configuration (slave ID, timeouts).
   */
  const ModbusDeviceConfig &getDeviceConfig() const { return cfg_; }

  // -------------------------------------------------------------------------
  // public API
  // -------------------------------------------------------------------------

  /**
   * @brief Get the manufacturer name from the device (typically "Fronius").
   */
  std::expected<std::string, ModbusError> getManufacturer(void);

  /**
   * @brief Get the device model name (e.g. "IG+150V [3p]").
   */
  std::expected<std::string, ModbusError> getDeviceModel(void);

  /**
   * @brief Get the firmware version of any installed option (e.g. Datamanager).
   */
  std::expected<std::string, ModbusError> getOptions(void);

  /**
   * @brief Get the main device firmware version.
   */
  std::expected<std::string, ModbusError> getFwVersion(void);

  /**
   * @brief Get the serial number of the connected device.
   */
  std::expected<std::string, ModbusError> getSerialNumber(void);

  /**
   * @brief Get the Modbus slave address reported by the device (1-247).
   */
  std::expected<uint16_t, ModbusError> getModbusDeviceAddress(void);

protected:
  /** @brief Per-device Modbus configuration (slave ID, response timeout). */
  const ModbusDeviceConfig cfg_;

  /**
   * @brief Per-device register buffer.
   *
   * Sized to cover the full 16-bit Modbus address space (0x0000–0xFFFF).
   * All fetch functions write into this buffer; accessor functions read
   * from it. The buffer belongs exclusively to this device — it is never
   * shared with other devices on the same bus.
   */
  std::vector<uint16_t> regs_;

  /**
   * @brief Register map type detected during the last successful validation.
   *
   * Defaults to `UNAVAILABLE` until `onBusConnected()` completes
   * successfully.
   */
  FroniusTypes::RegisterMap registerMap_{
      FroniusTypes::RegisterMap::UNAVAILABLE};

  // -------------------------------------------------------------------------
  // Callbacks — fired by concrete device implementations
  // -------------------------------------------------------------------------

  /** @brief Fired when device validation succeeds. Carries the register map. */
  std::function<void(FroniusTypes::RegisterMap)> onDeviceReady_;

  /** @brief Fired when the device is unavailable (bus down or validation
   *         failed). */
  std::function<void()> onDeviceUnavailable_;

  /** @brief Fired when a device-level Modbus error occurs. */
  std::function<void(const ModbusError &)> onDeviceError_;

  /** @brief Fired before each per-device reconnect delay. */
  std::function<void(int)> onDeviceRetry_;

  // -------------------------------------------------------------------------
  // Helpers for concrete device implementations
  // -------------------------------------------------------------------------

  /**
   * @brief Mark the device as ready and fire `onDeviceReady_`.
   *
   * Concrete implementations call this at the end of a successful
   * `onBusConnected()` validation sequence.
   *
   * @param map The detected register map to pass to the callback.
   */
  void setReady(FroniusTypes::RegisterMap map);

  /**
   * @brief Mark the device as not ready and fire `onDeviceUnavailable_`.
   *
   * Concrete implementations call this when validation fails or when
   * `onBusDisconnected()` is called.
   */
  void setUnavailable();

  /**
   * @brief Report a Modbus error via the registered device error callback.
   *
   * If the result contains an error and `onDeviceError_` is set, the
   * callback is invoked. The result is returned unchanged so callers can
   * propagate it naturally with `return reportError(...)`.
   *
   * @tparam T Expected value type.
   * @param res The result to inspect and potentially report.
   * @return The same result, unmodified.
   */
  template <typename T>
  std::expected<T, ModbusError>
  reportError(std::expected<T, ModbusError> res) const {
    if (!res && onDeviceError_)
      onDeviceError_(res.error());
    return res;
  }

  /**
   * @brief Decode a Modbus register range into a printable ASCII string.
   *
   * Interprets a sequence of 16-bit registers as pairs of ASCII characters
   * (high byte first, low byte second). Verifies that the register type is
   * `Register::Type::STRING` and that all characters are printable. Null
   * bytes are treated as string terminators.
   *
   * @param regs  Register buffer to read from (typically `regs_`).
   * @param reg   Register descriptor specifying address, length, and type.
   * @return Decoded string on success, or a `ModbusError` on failure.
   */
  std::expected<std::string, ModbusError>
  getModbusString(const std::vector<uint16_t> &regs, const Register &reg) const;

  /**
   * @brief Retrieve a scaled double value from Modbus registers.
   *
   * Reads `reg` and applies the SunSpec scale factor `10^sf` if `sf` is
   * provided. Supports `INT16`, `UINT16`, `UINT32`, and `FLOAT` register
   * types; `FLOAT` registers are read in ABCD byte order and ignore `sf`.
   *
   * @param regs  Register buffer to read from (typically `regs_`).
   * @param reg   Register descriptor for the value.
   * @param sf    Optional scale-factor register; omit for no scaling.
   * @return Scaled double on success, or a `ModbusError` on failure.
   */
  std::expected<double, ModbusError>
  getModbusDouble(const std::vector<uint16_t> &regs, const Register &reg,
                  std::optional<Register> sf = std::nullopt) const;

  /**
   * @brief Retrieve a scaled double from an INT32 register using a
   *        compile-time scale factor.
   *
   * Used with the proprietary Fronius RTU register map, where scale factors
   * are fixed constants and values are stored in INT32 with swapped word
   * order.
   *
   * @param regs  Register buffer to read from (typically `regs_`).
   * @param reg   Register descriptor (must be `Register::Type::INT32`).
   * @param sf    Fixed-point scale factor (e.g. `0.1`, `0.001`).
   * @return Scaled double on success, or a `ModbusError` on failure.
   */
  std::expected<double, ModbusError>
  getModbusDouble(const std::vector<uint16_t> &regs, const Register &reg,
                  double sf) const;

private:
  /**
   * @brief Atomic flag reflecting whether this device has been successfully
   *        validated and is ready to serve data.
   */
  std::atomic<bool> ready_{false};
};

#endif /* FRONIUS_DEVICE_H_ */
