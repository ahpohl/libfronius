#ifndef INVERTER_H_
#define INVERTER_H_

#include "fronius.h"
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
  bool getUseFloatRegisters(void) const;

  /**
   * @brief Get the number of active phases.
   *
   * @return Number of phases (1, 2, or 3).
   */
  int getPhases(void) const;

  /**
   * @brief Get the inverter's device ID.
   *
   * @return Integer representing the detected inverter ID.
   */
  int getId(void) const;

  /**
   * @brief Get AC current in amperes.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Current value in amperes (A).
   */
  double getAcCurrent(const Phase ph = Phase::TOTAL) const;

  /**
   * @brief Get AC voltage in volts.
   *
   * @param ph Phase to read (`PHA`, `PHB`, or `PHC`).
   * @return Voltage value in volts (V).
   */
  double getAcVoltage(const Phase ph = Phase::PHA) const;

  /**
   * @brief Get AC active power in watts.
   * @return Active power value in watts (W).
   */
  double getAcPowerActive(void) const;

  /**
   * @brief Get AC frequency in hertz.
   * @return Frequency value in hertz (Hz).
   */
  double getAcFrequency(void) const;

  /**
   * @brief Get AC apparent power in volt-amperes.
   * @return Apparent power value in volt-amperes (VA).
   */
  double getAcPowerApparent(void) const;

  /**
   * @brief Get AC reactive power in volt-ampere reactive.
   * @return Reactive power value in volt-ampere reactive (VAr).
   */
  double getAcPowerReactive(void) const;

  /**
   * @brief Get AC power factor (dimensionless).
   * @return Power factor as a unitless ratio (typically between -1 and 1).
   */
  double getAcPowerFactor(void) const;

  /**
   * @brief Get total lifetime energy production.
   * @return Exported energy in watt-hours (Wh).
   */
  double getAcEnergy(void) const;

  /**
   * @brief Get DC power of all inputs in watts.
   * @return Power value in watts (W).
   */
  double getDcPower(void) const;

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
