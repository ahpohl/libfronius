/**
 * @file meter.h
 * @brief Fronius Modbus-compatible power meter device.
 */

#ifndef METER_H_
#define METER_H_

#include "fronius_bus.h"
#include "fronius_device.h"
#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <expected>
#include <memory>
#include <string>

/**
 * @class Meter
 * @brief Represents a Fronius Modbus-compatible power meter.
 *
 * Meter communicates over a shared `FroniusBus` instance and supports
 * automatic detection of float- and integer-based SunSpec register maps as
 * well as the proprietary Fronius RTU register map (Smart Meter TS 65A-3).
 *
 * @note Must always be heap-allocated via `std::shared_ptr`. The constructor
 *       registers the instance with the bus via `weak_from_this()`.
 */
class Meter : public FroniusDevice {
public:
  /**
   * @brief Construct a Meter and register it with the given bus.
   *
   * @param bus  Shared bus this meter communicates over.
   * @param cfg  Per-device configuration (slave ID, response timeout).
   */
  Meter(std::shared_ptr<FroniusBus> bus, const ModbusDeviceConfig &cfg);

  /**
   * @brief Destructor.
   */
  ~Meter() override = default;

  // -------------------------------------------------------------------------
  // FroniusDevice bus lifecycle interface
  // -------------------------------------------------------------------------

  /**
   * @brief Probe and validate the meter when the bus comes up.
   *
   * Detects whether the device speaks the proprietary RTU map or the
   * SunSpec map (and within SunSpec, integer vs. float). Calls
   * `setReady()` on success or `setUnavailable()` on failure.
   */
  void onBusConnected() override;

  /**
   * @brief Reset all detected state and mark the device unavailable.
   */
  void onBusDisconnected() override;

  // -------------------------------------------------------------------------
  // Data fetch
  // -------------------------------------------------------------------------

  /**
   * @brief Fetch the complete meter register map from the device.
   *
   * Submits the necessary register-read transactions and blocks until they
   * complete. After a successful call, all `getXxx()` accessors reflect
   * fresh values.
   */
  std::expected<void, ModbusError> fetchMeterRegisters();

  // -------------------------------------------------------------------------
  // Device identity
  // -------------------------------------------------------------------------

  /**
   * @brief Returns true if the meter uses float-based register encoding.
   *
   * Float model IDs: 211 (single-phase), 212 (split-phase), 213 (three-phase).
   * Integer model IDs: 201, 202, 203.
   */
  bool getUseFloatRegisters() const { return useFloatRegisters_; }

  /**
   * @brief Get the number of phases supported by the meter.
   *
   * For the proprietary RTU map the TS 65A-3 is always three-phase.
   * For SunSpec maps, the phase count is derived from the meter model ID
   * (last digit: 1 = single, 2 = split, 3 = three).
   */
  int getPhases() const {
    if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
      return 3;
    return id_ % 10;
  }

  /**
   * @brief Get the detected meter model ID (e.g. 201, 213).
   *
   * Returns 0 if the device has not been validated yet.
   */
  int getId() const { return id_; }

  /**
   * @brief Get the Modbus slave address reported by the device.
   *
   * For the SunSpec map, reads from the common block DA register.
   * For the proprietary map, returns the configured slave ID directly.
   */
  std::expected<uint16_t, ModbusError> getModbusDeviceAddress();

  /**
   * @brief Get the serial number of the connected meter.
   *
   * For the SunSpec map, reads from the common block SN register.
   * For the proprietary map, reads the 32-bit serial number directly and
   * returns it as a decimal string.
   */
  std::expected<std::string, ModbusError> getSerialNumber();

  /**
   * @brief Get the meter model name.
   *
   * For the SunSpec map, reads from the common block MD register.
   * For the proprietary map, returns the hardcoded string
   * `"Smart Meter TS 65A-3"`.
   */
  std::expected<std::string, ModbusError> getDeviceModel();

  /**
   * @brief Get the manufacturer name.
   *
   * For the SunSpec map, reads from the common block MN register.
   * For the proprietary map, returns the hardcoded string `"Fronius"`.
   */
  std::expected<std::string, ModbusError> getManufacturer();

  /**
   * @brief Get the meter firmware version.
   *
   * For the SunSpec map, reads from the common block VR register.
   * For the proprietary map, reads the version registers directly and
   * returns a string in `"major.minor"` format (e.g. `"1.5"`).
   */
  std::expected<std::string, ModbusError> getFwVersion();

  // -------------------------------------------------------------------------
  // Electrical measurements — all read from regs_ populated by
  // fetchMeterRegisters()
  // -------------------------------------------------------------------------

  /**
   * @brief Get AC current in amperes.
   * @param ph Phase to read (`TOTAL`, `A`, `B`, or `C`).
   *           `TOTAL` is not supported for the proprietary register map.
   */
  std::expected<double, ModbusError>
  getAcCurrent(FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC voltage in volts.
   * @param ph Phase to read (`PHV`, `PPV`, `A`, `B`, `C`, `AB`, `BC`, `CA`).
   */
  std::expected<double, ModbusError>
  getAcVoltage(FroniusTypes::Phase ph = FroniusTypes::Phase::PHV) const;

  /**
   * @brief Get AC active power in watts.
   * @param ph Phase to read (`TOTAL`, `A`, `B`, or `C`).
   */
  std::expected<double, ModbusError>
  getAcPowerActive(FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC frequency in hertz.
   */
  std::expected<double, ModbusError> getAcFrequency() const;

  /**
   * @brief Get AC apparent power in volt-amperes.
   * @param ph Phase to read (`TOTAL`, `A`, `B`, or `C`).
   */
  std::expected<double, ModbusError>
  getAcPowerApparent(FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC reactive power in volt-ampere reactive.
   * @param ph Phase to read (`TOTAL`, `A`, `B`, or `C`).
   */
  std::expected<double, ModbusError>
  getAcPowerReactive(FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC power factor (dimensionless, typically -1 to 1).
   * @param ph Phase to read (`AVERAGE`, `A`, `B`, or `C`).
   */
  std::expected<double, ModbusError>
  getAcPowerFactor(FroniusTypes::Phase ph = FroniusTypes::Phase::AVERAGE) const;

  /**
   * @brief Get total active energy in watt-hours (Wh).
   * @param direction `IMPORT` or `EXPORT`.
   * @note Per-phase energy is not supported.
   */
  std::expected<double, ModbusError>
  getAcEnergyActive(FroniusTypes::EnergyDirection direction) const;

  /**
   * @brief Get total apparent energy in volt-ampere-hours (VAh).
   * @param direction `IMPORT` or `EXPORT`.
   * @note Not supported for the proprietary register map.
   */
  std::expected<double, ModbusError>
  getAcEnergyApparent(FroniusTypes::EnergyDirection direction) const;

  /**
   * @brief Get total reactive energy in volt-ampere-reactive-hours (VArh).
   * @param direction `IMPORT` or `EXPORT`.
   * @note Only supported for the proprietary register map.
   */
  std::expected<double, ModbusError>
  getAcEnergyReactive(FroniusTypes::EnergyDirection direction) const;

private:
  /** @brief Shared bus this device communicates over. */
  std::shared_ptr<FroniusBus> bus_;

  /** @brief True if the meter uses float-based (M21X) register encoding. */
  bool useFloatRegisters_{false};

  /** @brief Detected meter model ID (201–203 integer, 211–213 float, 0 =
   * unknown). */
  int id_{0};

  /**
   * @brief Probe the device to determine its register map.
   *
   * Tries the proprietary RTU map first; falls back to the SunSpec common
   * block. Within SunSpec, dispatches to `detectFloatOrIntRegisters()` to
   * pick between the M20X (integer + scale factor) and M21X (float)
   * variants. Sets `registerMap_`, `id_`, and `useFloatRegisters_`.
   *
   * @return The detected register map on success.
   */
  std::expected<FroniusTypes::RegisterMap, ModbusError> validateDevice();

  /**
   * @brief Detect float vs. integer SunSpec meter model.
   *
   * Reads the meter model ID register and matches it against the known
   * SunSpec IDs (201–203 integer, 211–213 float). Sets `id_` and
   * `useFloatRegisters_`.
   */
  std::expected<void, ModbusError> detectFloatOrIntRegisters();

  /**
   * @brief Validate the SunSpec end-of-map block.
   *
   * Confirms that the end-block ID is `0xFFFF` and length is `0` at the
   * address that corresponds to the detected encoding (integer vs. float).
   */
  std::expected<void, ModbusError> validateEndRegisters();

  /**
   * @brief Read a value from the register that matches the detected map.
   *
   * Dispatches to the proprietary, SunSpec-integer, or SunSpec-float
   * variant based on `registerMap_` and `useFloatRegisters_`.
   *
   * @param regProp  Proprietary register (INT32, swapped word order).
   * @param sfProp   Compile-time scale factor for the proprietary register.
   * @param regInt   SunSpec integer register.
   * @param sfInt    SunSpec scale-factor register for `regInt`.
   * @param regFlt   SunSpec float register.
   */
  std::expected<double, ModbusError>
  getRegValue(const Register &regProp, double sfProp, const Register &regInt,
              const Register &sfInt, const Register &regFlt) const;

  /**
   * @brief Build a Transaction targeting this device's slave ID and timeouts.
   *
   * @param startAddr  Starting Modbus register address.
   * @param count      Number of registers to read.
   * @param dest       Pointer into `regs_` where results are written.
   */
  FroniusBus::Transaction makeTransaction(int startAddr, int count,
                                          uint16_t *dest);
};

#endif /* METER_H_ */
