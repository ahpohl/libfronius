#ifndef METER_H_
#define METER_H_

#include "fronius.h"
#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <expected>
#include <modbus/modbus.h>

/**
 * @class Meter
 * @brief Represents a Fronius Modbus-compatible power meter.
 *
 * The Meter class extends the Fronius Modbus base class to handle
 * Fronius-specific meter register layouts. It supports automatic
 * detection of float- and integer-based register maps, validation
 * of meter data, and retrieval of key electrical measurements
 * such as current, voltage, power, and energy.
 *
 * @see Fronius
 */
class Meter : public Fronius {
public:
  /**
   * @brief Construct a new Meter instance.
   * @param cfg Modbus configuration parameters.
   */
  explicit Meter(const ModbusConfig &cfg);

  /**
   * @brief Destroy the Meter instance.
   */
  virtual ~Meter();

  /**
   * @brief Validate the meter's register data.
   *
   * Performs internal checks to ensure that the connected meter
   * is responding correctly and that its register layout is supported.
   *
   */
  std::expected<FroniusTypes::RegisterMap, ModbusError> validateDevice();

  /**
   * @brief Fetch the complete Fronius Meter Register Map from the device.
   *
   * Reads all registers relevant to the meter, including voltage,
   * current, power, and energy values.
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> fetchMeterRegisters(void);

  /**
   * @brief Get the current register model type.
   *
   * @return `true` if using float-based registers, `false` if using
   * integer/scaled registers.
   */
  bool getUseFloatRegisters(void) const { return useFloatRegisters_; };

  /**
   * @brief Get the number of phases supported by the meter.
   *
   * @return Number of phases (1, 2, or 3).
   * @note For the proprietary Fronius RTU register map, the TS 65A-3 is
   *       always a three-phase meter. Individual phase readings may still
   *       be zero if fewer phases are active in the installation.
   *       For the SunSpec register map, the phase count is derived from
   *       the meter model ID (e.g. 201 = single-phase, 203 = three-phase).
   */
  int getPhases(void) const {
    if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
      return 3;
    return id_ % 10;
  };

  /**
   * @brief Get the meter's device ID.
   *
   * @return Integer representing the detected meter ID.
   */
  int getId(void) const { return id_; };

  /**
   * @brief Get the Modbus slave address.
   *
   * @details
   * For the SunSpec register map, reads the device address from the
   * common block register @ref C001::DA. For the proprietary Fronius RTU
   * map, returns the configured slave ID directly since the common block
   * is not present.
   *
   * @return Slave ID in the range 1–247.
   */
  std::expected<uint16_t, ModbusError> getModbusDeviceAddress(void);

  /**
   * @brief Get the serial number of the connected meter.
   *
   * @details
   * For the SunSpec register map, reads from the common block string
   * register @ref C001::SN. For the proprietary Fronius RTU map, reads
   * the 32-bit serial number from registers 0x5007–0x5008 directly and
   * returns it as a decimal string.
   *
   * @return Serial number string (e.g. `"149685681"`).
   */
  std::expected<std::string, ModbusError> getSerialNumber(void);

  /**
   * @brief Get the meter model name.
   *
   * @details
   * For the SunSpec register map, reads from @ref C001::MD. For the
   * proprietary Fronius RTU map, returns the hardcoded string
   * `"Smart Meter TS 65A-3"` since no model string register exists in
   * that map.
   *
   * @return Model name string (e.g. `"Smart Meter TS 65A-3"`).
   */
  std::expected<std::string, ModbusError> getDeviceModel(void);

  /**
   * @brief Get the manufacturer name.
   *
   * @details
   * For the SunSpec register map, reads from @ref C001::MN. For the
   * proprietary Fronius RTU map, returns the hardcoded string `"Fronius"`
   * since no manufacturer string register exists in that map.
   *
   * @return Manufacturer name string (e.g. `"Fronius"`).
   */
  std::expected<std::string, ModbusError> getManufacturer(void);

  /**
   * @brief Get the meter firmware version.
   *
   * @details
   * For the SunSpec register map, reads from @ref C001::VR. For the
   * proprietary Fronius RTU map, returns the version cached from the
   * device type probe performed during @ref validateDevice() (e.g.
   * `"1.5"`), since no separate firmware version register exists in that
   * map.
   *
   * @return Firmware version string (e.g. `"1.5"`).
   */
  std::expected<std::string, ModbusError> getFwVersion(void);

  /**
   * @brief Get AC current in amperes.
   *
   * @param ph FroniusTypes::Phase to read (`TOTAL`, `A`, `B`, or `C`).
   * @return Current value in amperes (A).
   */
  std::expected<double, ModbusError>
  getAcCurrent(const FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC voltage in volts.
   *
   * @param ph FroniusTypes::Phase to read (`PHV`, `A`, `B`, or `C`).
   * @return Voltage value in volts (V).
   */
  std::expected<double, ModbusError>
  getAcVoltage(const FroniusTypes::Phase ph = FroniusTypes::Phase::PHV) const;

  /**
   * @brief Get AC active power in watts.
   *
   * @param ph FroniusTypes::Phase to read (`TOTAL`, `A`, `B`, or `C`).
   * @return Active power value in watts (W).
   */
  std::expected<double, ModbusError> getAcPowerActive(
      const FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC frequency in hertz.
   *
   * @return Frequency value in hertz (Hz).
   */
  std::expected<double, ModbusError> getAcFrequency(void) const;

  /**
   * @brief Get AC apparent power in volt-amperes.
   *
   * @param ph FroniusTypes::Phase to read (`TOTAL`, `A`, `B`, or `C`).
   * @return Apparent power value in volt-amperes (VA).
   */
  std::expected<double, ModbusError> getAcPowerApparent(
      const FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC reactive power in volt-ampere reactive.
   *
   * @param ph FroniusTypes::Phase to read (`TOTAL`, `A`, `B`, or `C`).
   * @return Reactive power value in volt-ampere reactive (VAr).
   */
  std::expected<double, ModbusError> getAcPowerReactive(
      const FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC power factor (dimensionless).
   *
   * @param ph FroniusTypes::Phase to read (`AVERAGE`, `A`, `B`, or `C`).
   * @return Power factor as a unitless ratio (typically between -1 and 1).
   */
  std::expected<double, ModbusError> getAcPowerFactor(
      const FroniusTypes::Phase ph = FroniusTypes::Phase::AVERAGE) const;

  /**
   * @brief Get total exported active energy.
   *
   * @return Total exported energy in watt-hours (Wh).
   * @note Per-phase export energy is not supported by any currently
   *       implemented meter (EasyMeter, Fronius TS 65A-3).
   */
  std::expected<double, ModbusError> getAcEnergyActiveExport(void) const;

  /**
   * @brief Get total imported active energy.
   *
   * @return Total imported energy in watt-hours (Wh).
   * @note Per-phase import energy is not supported by any currently
   *       implemented meter (EasyMeter, Fronius TS 65A-3).
   */
  std::expected<double, ModbusError> getAcEnergyActiveImport(void) const;

private:
  /**
   * @brief Flag indicating if the meter uses float-based register encoding.
   */
  bool useFloatRegisters_{false};

  /**
   * @brief Register map type detected during the last successful
   *        validateDevice() call. Defaults to UNAVAILABLE until detection
   *        has completed.
   */
  FroniusTypes::RegisterMap registerMap_{
      FroniusTypes::RegisterMap::UNAVAILABLE};

  /**
   * @brief Detected Fronius meter ID (e.g., 201, 213, etc.).
   */
  int id_{0};

  /**
   * @brief Detect the register map type (float vs. integer) and initialize the
   * meter.
   *
   * Must be called after establishing a connection.
   * Determines the register model based on the meter ID:
   * - **Float model:**
   *   - 211: single-phase
   *   - 212: split-phase
   *   - 213: three-phase
   *
   * - **Integer model:**
   *   - 201: single-phase
   *   - 202: split-phase
   *   - 203: three-phase
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   * @note Automatically sets `useFloatRegisters_` and `id_` based on the device
   * response.
   */
  std::expected<void, ModbusError> detectFloatOrIntRegisters(void);

  /**
   * @brief Retrieve a scaled double value from the appropriate register
   *        depending on the detected register map.
   *
   * @param regProp  Proprietary RTU register descriptor (INT32, little-endian
   *                 word order).
   * @param sfProp   Compile-time scale factor for the proprietary register
   *                 (e.g. @ref REG::V_SF, @ref REG::A_SF).
   * @param regInt   SunSpec integer register descriptor (int+sf model).
   * @param sfInt    SunSpec scale-factor register for @p regInt.
   * @param regFlt   SunSpec float register descriptor (float model).
   *
   * @return Scaled physical value on success, or a @ref ModbusError if
   *         the register map is unavailable or the register type is
   *         unsupported.
   */
  std::expected<double, ModbusError>
  getRegValue(const Register &regProp, double sfProp, const Register &regInt,
              const Register &sfInt, const Register &regFlt) const;
};

#endif /* METER_H_ */