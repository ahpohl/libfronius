#ifndef METER_H_
#define METER_H_

#include "fronius.h"
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
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> validateDevice();

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
  bool getUseFloatRegisters(void) const;

  /**
   * @brief Get the number of active phases.
   *
   * @return Number of phases (1, 2, or 3).
   */
  int getPhases(void) const;

  /**
   * @brief Get the meter's device ID.
   *
   * @return Integer representing the detected meter ID.
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
   * @param ph Phase to read (`AVERAGE`, `PHA`, `PHB`, or `PHC`).
   * @return Voltage value in volts (V).
   */
  double getAcVoltage(const Phase ph = Phase::AVERAGE) const;

  /**
   * @brief Get AC active power in watts.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Active power value in watts (W).
   */
  double getAcPowerActive(const Phase ph = Phase::TOTAL) const;

  /**
   * @brief Get AC frequency in hertz.
   *
   * @return Frequency value in hertz (Hz).
   */
  double getAcFrequency(void) const;

  /**
   * @brief Get AC apparent power in volt-amperes.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Apparent power value in volt-amperes (VA).
   */
  double getAcPowerApparent(const Phase ph = Phase::TOTAL) const;

  /**
   * @brief Get AC reactive power in volt-ampere reactive.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Reactive power value in volt-ampere reactive (VAr).
   */
  double getAcPowerReactive(const Phase ph = Phase::TOTAL) const;

  /**
   * @brief Get AC power factor (dimensionless).
   *
   * @param ph Phase to read (`AVERAGE`, `PHA`, `PHB`, or `PHC`).
   * @return Power factor as a unitless ratio (typically between -1 and 1).
   */
  double getAcPowerFactor(const Phase ph = Phase::AVERAGE) const;

  /**
   * @brief Get total exported active energy.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Exported energy in watt-hours (Wh).
   */
  double getAcEnergyActiveExport(const Phase ph = Phase::TOTAL) const;

  /**
   * @brief Get total imported active energy.
   *
   * @param ph Phase to read (`TOTAL`, `PHA`, `PHB`, or `PHC`).
   * @return Imported energy in watt-hours (Wh).
   */
  double getAcEnergyActiveImport(const Phase ph = Phase::TOTAL) const;

private:
  /**
   * @brief Indicates whether the meter connection and register data are valid.
   */
  bool connectedAndValid_{false};

  /**
   * @brief Flag indicating if the meter uses float-based register encoding.
   */
  bool useFloatRegisters_{false};

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
};

#endif /* METER_H_ */