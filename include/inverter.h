#ifndef INVERTER_H_
#define INVERTER_H_

#include "fronius.h"
#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <expected>
#include <modbus/modbus.h>

/**
 * @class Inverter
 * @brief Represents a Fronius Modbus-compatible inverter.
 *
 * The Inverter class extends the Fronius Modbus base class to handle
 * Fronius-specific inverter register layouts. It supports automatic
 * detection of float- and integer-based register maps, validation
 * of inverter data, and retrieval of key electrical measurements
 * such as current, voltage, power, frequency, and energy.
 *
 * @see Fronius
 */
class Inverter : public Fronius {
public:
  /**
   * @brief Construct a new Inverter instance.
   * @param cfg Modbus configuration parameters.
   */
  explicit Inverter(const ModbusConfig &cfg);

  /**
   * @brief Destroy the Inverter instance.
   */
  virtual ~Inverter();

  /**
   * @brief Validate the inverter's register data.
   *
   * Performs internal checks to ensure that the connected inverter
   * is responding correctly and that its register layout is supported.
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> validateDevice();

  /**
   * @brief Fetch the complete Fronius Inverter Register Map from the device.
   *
   * Reads all registers relevant to the inverter, including voltage,
   * current, power, frequency, and energy values.
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> fetchInverterRegisters(void);

  /**
   * @brief Get the current register model type.
   *
   * @return `true` if using float-based registers, `false` if using
   * integer/scaled registers.
   */
  bool getUseFloatRegisters(void) const { return useFloatRegisters_; }

  /**
   * @brief Get the number of active phases.
   *
   * @return Number of phases (1, 2, or 3).
   */
  int getPhases(void) const { return id_ % 10; }

  /**
   * @brief Get the inverter's device ID.
   *
   * @return Integer representing the detected inverter ID.
   */
  int getId(void) const { return id_; }

  /**
   * @brief Get AC current in amperes.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Current value in amperes (A).
   */
  std::expected<double, ModbusError>
  getAcCurrent(const FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC voltage in volts.
   *
   * @param ph Phase to read (`PHA`, `PHB`, or `PHC`).
   * @return Voltage value in volts (V).
   */
  std::expected<double, ModbusError>
  getAcVoltage(const FroniusTypes::Phase ph = FroniusTypes::Phase::A) const;

  /**
   * @brief Get AC active power in watts.
   * @return Active power value in watts (W).
   */
  std::expected<double, ModbusError> getAcPowerActive(void) const;

  /**
   * @brief Get AC frequency in hertz.
   * @return Frequency value in hertz (Hz).
   */
  std::expected<double, ModbusError> getAcFrequency(void) const;

  /**
   * @brief Get AC apparent power in volt-amperes.
   * @return Apparent power value in volt-amperes (VA).
   */
  std::expected<double, ModbusError> getAcPowerApparent(void) const;

  /**
   * @brief Get AC reactive power in volt-ampere reactive.
   * @return Reactive power value in volt-ampere reactive (VAr).
   */
  std::expected<double, ModbusError> getAcPowerReactive(void) const;

  /**
   * @brief Get AC power factor (dimensionless).
   * @return Power factor as a unitless ratio (typically between -1 and 1).
   */
  std::expected<double, ModbusError> getAcPowerFactor(void) const;

  /**
   * @brief Get total lifetime energy production.
   * @return Exported energy in watt-hours (Wh).
   */
  std::expected<double, ModbusError> getAcEnergy(void) const;

  /**
   * @brief Get DC power of a specific input in watts.
   *
   * @param input The input for which to retrieve DC power. Valid options are:
   *              - TOTAL : total DC power of all inputs
   *              - A : DC power of input string A
   *              - B : DC power of input string B
   * @return Power value in watts (W).
   */
  std::expected<double, ModbusError>
  getDcPower(const FroniusTypes::Input input) const;

  /**
   * @brief Get DC current of a specific input in amperes.
   *
   * @param input The input for which to retrieve DC current. Valid options are:
   *              - TOTAL : total DC current of all inputs
   *              - A : DC current of input string A
   *              - B : DC current of input string B
   * @return Current value in amperes (A).
   */
  std::expected<double, ModbusError>
  getDcCurrent(const FroniusTypes::Input input) const;

  /**
   * @brief Get DC voltage of a specific input in volts.
   *
   * @param input The input for which to retrieve DC voltage. Valid options are:
   *              - TOTAL : total DC voltage of all inputs
   *              - A : DC voltage of input string A
   *              - B : DC voltage of input string B
   * @return Voltage value in volts (V).
   */
  std::expected<double, ModbusError>
  getDcVoltage(const FroniusTypes::Input input) const;

private:
  /**
   * @brief Indicates whether the inverter connection and register data are
   * valid.
   */
  bool connectedAndValid_{false};

  /**
   * @brief Flag indicating if the inverter uses float-based register encoding.
   */
  bool useFloatRegisters_{false};

  /**
   * @brief Detected Fronius inverter ID (e.g., 111, 112, 113, etc.).
   */
  int id_{0};

  /**
   * @brief Detect the register map type (float vs. integer) and initialize the
   * inverter.
   *
   * Must be called after establishing a connection.
   * Determines the register model based on the inverter ID:
   * - **Float model:**
   *   - 111: single-phase
   *   - 112: split-phase
   *   - 113: three-phase
   *
   * - **Integer model:**
   *   - 101: single-phase
   *   - 102: split-phase
   *   - 103: three-phase
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   * @note Automatically sets `useFloatRegisters_` and `id_` based on the device
   * response.
   */
  std::expected<void, ModbusError> detectFloatOrIntRegisters(void);
};

#endif /* INVERTER_H_ */
