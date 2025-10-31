#ifndef INVERTER_H_
#define INVERTER_H_

#include "fronius.h"
#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <expected>
#include <modbus/modbus.h>
#include <vector>

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
   * @brief Get the number of DC input strings (MPPT inputs).
   *
   * This function returns the number of DC inputs detected on the inverter.
   * The value is determined by reading and validating the Multi MPPT register
   * block using validateMultiMpptRegisters().
   *
   * @return Number of DC inputs (1 or 2).
   */
  int getInputs(void) const { return inputs_; }

  /**
   * @brief Check whether the inverter is a hybrid system.
   *
   * @return true if the inverter has valid hybrid storage registers (as
   * determined by `validateStorageRegisters()`), false otherwise.
   */
  bool isHybrid(void) const { return hybrid_; }

  /**
   * @brief Get the inverter's device ID.
   *
   * @return Integer representing the detected inverter ID.
   */
  int getId(void) const { return id_; }

  /**
   * @brief Returns the AC power rating for the specified output type.
   *
   * Reads and scales the corresponding Modbus registers from the inverter’s
   * nameplate register map to compute the requested power value.
   * The function supports both integer and float-based register mappings.
   *
   * @param output The AC power output type to read
   *               (see @ref FroniusTypes::Output).
   *
   * @return std::expected<double, ModbusError>
   *   - The measured AC power value on success.
   *   - A `ModbusError` if register access or scaling fails.
   */
  std::expected<double, ModbusError>
  getAcPowerRating(FroniusTypes::Output output) const;

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
   * @brief Retrieve the AC power value from the inverter.
   *
   * This function reads the AC power of the inverter for a specified output
   * type. It supports both floating-point and scaled integer Modbus registers
   * depending on the `useFloatRegisters_` flag.
   *
   * @param output The type of AC power to retrieve. Possible values are:
   * - `FroniusTypes::Output::ACTIVE` – Active power (W)
   * - `FroniusTypes::Output::APPARENT` – Apparent power (VA)
   * - `FroniusTypes::Output::REACTIVE` – Reactive power (VAR)
   * - `FroniusTypes::Output::FACTOR` – Power factor (unitless, 0..1)
   *
   * @return std::expected<double, ModbusError>
   * - On success: The AC power value corresponding to the requested output
   * type.
   * - On failure: A `ModbusError` describing why the value could not be read,
   *   e.g., invalid output type or communication error.
   *
   * @note If `useFloatRegisters_` is true, the function reads floating-point
   * Modbus registers (I11X). Otherwise, it reads scaled integer registers
   * (I10X) and applies the corresponding scale factor.
   *
   * @warning Passing an invalid `output` value will result in a `ModbusError`.
   */
  std::expected<double, ModbusError>
  getAcPower(FroniusTypes::Output output) const;

  /**
   * @brief Get AC frequency in hertz.
   * @return Frequency value in hertz (Hz).
   */
  std::expected<double, ModbusError> getAcFrequency(void) const;

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

  /**
   * @brief Get the DC energy for a specific input string of the inverter.
   *
   * This function reads the cumulative DC energy (in watt-hours) from the
   * inverter for the selected input string (A or B). The function handles both
   * float-based and scaled integer Modbus registers, depending on the
   * `useFloatRegisters_` flag.
   *
   * @param input The input string for which to read DC energy. Valid values
   * are:
   *              - `FroniusTypes::Input::A`
   *              - `FroniusTypes::Input::B`
   *
   * @return std::expected<double, ModbusError>
   *         - On success: the DC energy in watt-hours (Wh) as a `double`.
   *         - On failure: a `ModbusError` indicating the cause of the failure,
   *           for example if an invalid input is provided or a Modbus read
   * fails.
   *
   * @note If an invalid input is provided, the function returns a `ModbusError`
   *       with code `EINVAL` and a descriptive message.
   */
  std::expected<double, ModbusError>
  getDcEnergy(const FroniusTypes::Input input) const;

  /**
   * @brief Reads the active state code of the inverter.
   *
   * This function returns the current active state code from the inverter's
   * Modbus registers. The active state code represents the inverter's
   * operational state, such as running, standby, or fault conditions. The raw
   * register value is cast to an `int` for convenience.
   *
   * @return Contains the active state code if the register
   *
   * @note The returned integer corresponds directly to the inverter's internal
   * state codes. Users should refer to the Fronius inverter documentation for
   * the mapping of codes to human-readable states.
   */
  int getActiveStateCode(void) const;

  /**
   * @brief Retrieves the current operational state of the inverter.
   *
   * This function reads the inverter status register (`STVND`) and converts
   * the raw status code into a human-readable string representation using
   * `FroniusTypes::toString(FroniusTypes::State)`.
   *
   * The register address depends on whether floating-point register mapping
   * is used (`useFloatRegisters_`).
   *
   * @return
   * A `std::expected<std::string, ModbusError>` containing:
   * - **value**: the string representation of the inverter state, if
   * recognized.
   * - **error**: a `ModbusError` with code `EINVAL` if the state code is
   * unknown or cannot be converted to a string.
   */
  std::expected<std::string, ModbusError> getState(void) const;

  /**
   * @brief Retrieves all active inverter events as human-readable strings.
   *
   * This function reads the raw event registers from the inverter and converts
   * the set bits into descriptive event names. The inverter has three event
   * groups, each represented by a 32-bit Modbus register:
   *   - Event group 1: FroniusTypes::Event_1
   *   - Event group 2: FroniusTypes::Event_2
   *   - Event group 3: FroniusTypes::Event_3
   *
   * Each bit in these registers indicates a specific event. Only bits that map
   * to known events are converted; unknown bits result in a `ModbusError`.
   *
   * @return std::expected<std::vector<std::string>, ModbusError>
   *         - On success: a vector of strings describing the active events.
   *         - On failure: a `ModbusError` describing unknown bits or Modbus
   * read issues.
   *
   * @note The function reads the event registers directly from `regs_`. If
   * `useFloatRegisters_` is true, it reads the corresponding float register
   * addresses.
   * @note Event group numbers in the error message are 1-based.
   */
  std::expected<std::vector<std::string>, ModbusError> getEvents() const;

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

  /**
   * @brief Number of DC input strings (MPPT inputs) supported by the inverter.
   *
   * This member is determined dynamically from the inverter’s Multi MPPT
   * register map. Possible values are:
   * - 1: Single string input or Multi MPPT not supported.
   * - 2: Dual string (Multi MPPT) input supported.
   *
   * Initialized to 0 until validated by validateMultiMpptRegisters().
   */
  int inputs_{0};

  /**
   * @brief Validate and identify the Multi MPPT (Multiple Maximum Power Point
   * Tracker) register map.
   *
   * This function verifies that the inverter’s Modbus register map supports
   * multiple DC input strings (MPPTs). It performs the following checks:
   *  - Validates that the Modbus context is available.
   *  - Reads and verifies the Multi MPPT register map ID and size.
   *  - Reads the second input string identifier (IDSTR_2) to determine whether
   *    a second MPPT input is present.
   *
   * Depending on the Modbus response, the internal member @ref inputs_ is set:
   *  - `inputs_ = 2` if the inverter reports `"String 2"`.
   *  - `inputs_ = 1` otherwise (e.g., `"Not supported"` or missing entry).
   *
   * @return
   *  - `std::expected<void, ModbusError>` containing no value on success.
   *  - `std::unexpected(ModbusError)` if a Modbus communication or validation
   * error occurs.
   *
   * @note This function must be called after establishing a valid Modbus
   *       connection. If the context is null, an ENOTCONN error is returned.
   *
   * @see getInputs()
   */
  std::expected<void, ModbusError> validateMultiMpptRegisters(void);

  /**
   * @brief Indicates whether the inverter has hybrid storage capability.
   *
   * This member is set to `true` by `validateStorageRegisters()` if the
   * inverter contains valid hybrid storage registers, otherwise it remains
   * `false`.
   */
  bool hybrid_{false};

  /**
   * @brief Validate the storage-related Modbus registers on the inverter.
   *
   * This function reads and checks the storage control registers of the
   * inverter to ensure they match the expected register map ID and size for a
   * basic storage system. It sets the `hybrid_` member to `true` if the storage
   * registers are valid and indicate a hybrid system, or `false` otherwise.
   *
   * The function handles both float-based and standard Modbus registers
   * depending on the `useFloatRegisters_` flag. It returns an error if:
   * - The Modbus context (`ctx_`) is null.
   * - Reading the registers fails.
   * - The registers contain an unexpected ID or size.
   *
   * @return std::expected<void, ModbusError>
   *         - On success: an empty `std::expected` indicating the storage
   * registers are valid.
   *         - On failure: a `ModbusError` describing the specific validation
   * failure.
   */
  std::expected<void, ModbusError> validateStorageRegisters(void);

  /**
   * @brief Validates the inverter's nameplate register map.
   *
   * This function performs a Modbus read operation on the inverter’s
   * nameplate registers to verify that the register map ID and map size
   * match the expected I120 register definition.
   *
   * The function checks:
   *  - That the Modbus context (`ctx_`) is valid.
   *  - That the nameplate register map ID matches the expected value (120).
   *  - That the map size corresponds to `I120::SIZE`.
   *
   * If any of these checks fail, a descriptive `ModbusError` is returned
   * via `std::unexpected`.
   *
   * @return std::expected<void, ModbusError>
   *   - An empty expected value on success.
   *   - A `ModbusError` describing the failure if validation fails.
   */
  std::expected<void, ModbusError> validateNameplateRegisters(void);
};

#endif /* INVERTER_H_ */
