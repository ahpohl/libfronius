/**
 * @file inverter.h
 * @brief Fronius Modbus-compatible inverter device.
 */

#ifndef INVERTER_H_
#define INVERTER_H_

#include "fronius_bus.h"
#include "fronius_device.h"
#include "fronius_types.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <expected>
#include <memory>
#include <string>
#include <vector>

/**
 * @class Inverter
 * @brief Represents a Fronius Modbus-compatible inverter.
 *
 * `Inverter` communicates over a shared `FroniusBus` and supports
 * automatic detection of float- and integer-based SunSpec register maps.
 * Validation identifies the inverter model, the number of MPPT inputs,
 * hybrid storage capability, and the nameplate block.
 *
 * Typical construction:
 * ```cpp
 * auto bus      = std::make_shared<FroniusBus>(busConfig);
 * auto inverter = std::make_shared<Inverter>(bus, deviceConfig);
 * inverter->setDeviceReadyCallback([](auto map){ ... });
 * bus->connect();
 * ```
 *
 * @note Must always be heap-allocated via `std::shared_ptr`. The
 *       constructor registers the instance with the bus via
 *       `weak_from_this()`.
 */
class Inverter : public FroniusDevice {
public:
  /**
   * @brief Construct an Inverter and register it with the given bus.
   *
   * @param bus  Shared bus this inverter communicates over.
   * @param cfg  Per-device configuration (slave ID, response timeout).
   */
  Inverter(std::shared_ptr<FroniusBus> bus, const ModbusDeviceConfig &cfg);

  /**
   * @brief Destructor.
   */
  ~Inverter() override = default;

  // -------------------------------------------------------------------------
  // FroniusDevice bus lifecycle interface
  // -------------------------------------------------------------------------

  /**
   * @brief Probe and validate the inverter when the bus comes up.
   *
   * Runs the full validation sequence (common, inverter, multi-MPPT,
   * storage, nameplate, end blocks). Calls `setReady()` on success or
   * `setUnavailable()` on failure.
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
   * @brief Fetch the complete inverter register map from the device.
   *
   * Submits the necessary register-read transactions and blocks until they
   * complete. After a successful call, all `getXxx()` accessors reflect
   * fresh values.
   */
  std::expected<void, ModbusError> fetchInverterRegisters();

  // -------------------------------------------------------------------------
  // Device identity
  // -------------------------------------------------------------------------

  /**
   * @brief Returns true if the inverter uses float-based register encoding.
   *
   * Float model IDs: 111 (single-phase), 112 (split-phase), 113 (three-phase).
   * Integer model IDs: 101, 102, 103.
   */
  bool getUseFloatRegisters() const { return useFloatRegisters_; }

  /**
   * @brief Get the number of active AC phases (derived from model ID).
   */
  int getPhases() const { return id_ % 10; }

  /**
   * @brief Get the number of DC input strings (MPPT inputs).
   *
   * Returns 1 or 2. Determined during validation by
   * `validateMultiMpptRegisters()`.
   */
  int getInputs() const { return inputs_; }

  /**
   * @brief Returns true if the inverter has hybrid storage capability.
   *
   * Determined during validation by `validateStorageRegisters()`.
   */
  bool isHybrid() const { return hybrid_; }

  /**
   * @brief Get the detected inverter model ID (e.g. 101, 113).
   *
   * Returns 0 if the device has not been validated yet.
   */
  int getId() const { return id_; }

  // -------------------------------------------------------------------------
  // Electrical measurements — all read from regs_ populated by
  // fetchInverterRegisters()
  // -------------------------------------------------------------------------

  /**
   * @brief Get the AC power rating for the specified output type.
   *
   * Reads from the nameplate register block (I120). Supports `ACTIVE`,
   * `APPARENT`, `Q1_REACTIVE`, and `Q4_REACTIVE`.
   */
  std::expected<double, ModbusError>
  getAcPowerRating(FroniusTypes::Output output) const;

  /**
   * @brief Get AC current in amperes.
   * @param ph Phase to read (`TOTAL`, `A`, `B`, or `C`).
   */
  std::expected<double, ModbusError>
  getAcCurrent(FroniusTypes::Phase ph = FroniusTypes::Phase::TOTAL) const;

  /**
   * @brief Get AC voltage in volts.
   * @param ph Phase to read (`A`, `B`, `C`, `AB`, `BC`, or `CA`).
   */
  std::expected<double, ModbusError>
  getAcVoltage(FroniusTypes::Phase ph = FroniusTypes::Phase::A) const;

  /**
   * @brief Get an AC power value by output type.
   * @param output `ACTIVE` (W), `APPARENT` (VA), `REACTIVE` (VAr),
   *               or `FACTOR` (dimensionless).
   */
  std::expected<double, ModbusError>
  getAcPower(FroniusTypes::Output output) const;

  /**
   * @brief Get AC frequency in hertz.
   */
  std::expected<double, ModbusError> getAcFrequency() const;

  /**
   * @brief Get total lifetime AC energy production in watt-hours (Wh).
   */
  std::expected<double, ModbusError> getAcEnergy() const;

  /**
   * @brief Get DC power for a specific input string in watts.
   * @param input `TOTAL`, `A`, or `B`.
   */
  std::expected<double, ModbusError>
  getDcPower(FroniusTypes::Input input) const;

  /**
   * @brief Get DC current for a specific input string in amperes.
   * @param input `TOTAL`, `A`, or `B`.
   */
  std::expected<double, ModbusError>
  getDcCurrent(FroniusTypes::Input input) const;

  /**
   * @brief Get DC voltage for a specific input string in volts.
   * @param input `TOTAL`, `A`, or `B`.
   */
  std::expected<double, ModbusError>
  getDcVoltage(FroniusTypes::Input input) const;

  /**
   * @brief Get cumulative DC energy for a specific input string in
   *        watt-hours (Wh).
   * @param input `A` or `B` only — `TOTAL` is not supported.
   */
  std::expected<double, ModbusError>
  getDcEnergy(FroniusTypes::Input input) const;

  /**
   * @brief Get the raw active state code from the inverter status register.
   */
  int getActiveStateCode() const;

  /**
   * @brief Get the current operational state as a human-readable string.
   *
   * Reads the STVND register and maps it through `FroniusTypes::toString()`.
   */
  std::expected<std::string, ModbusError> getState() const;

  /**
   * @brief Get all active inverter events as human-readable strings.
   *
   * Reads the three 32-bit event registers (EVT1, EVT2, EVT3) and converts
   * each set bit to a descriptive string via `FroniusTypes::toString()`.
   */
  std::expected<std::vector<std::string>, ModbusError> getEvents() const;

private:
  /** @brief Shared bus this device communicates over. */
  std::shared_ptr<FroniusBus> bus_;

  /** @brief True if the inverter uses float-based (I11X) register encoding. */
  bool useFloatRegisters_{false};

  /** @brief Detected inverter model ID (101-103 integer, 111-113 float, 0 =
   * unknown). */
  int id_{0};

  /** @brief Number of DC input strings (1 or 2). Set by
   * validateMultiMpptRegisters(). */
  int inputs_{0};

  /** @brief True if the inverter has hybrid storage registers. */
  bool hybrid_{false};

  /**
   * @brief Run all validation steps to identify the inverter.
   *
   * Probes the SunSpec common, inverter, multi-MPPT, storage, nameplate,
   * and end blocks in sequence. On success, sets `id_`,
   * `useFloatRegisters_`, `inputs_`, and `hybrid_`.
   */
  std::expected<void, ModbusError> validateDevice();

  /**
   * @brief Validate the SunSpec common block (C001).
   *
   * Reads the common block, verifies the SunSpec identifier ("SunS"), and
   * checks that the model ID and length match the SunSpec spec.
   */
  std::expected<void, ModbusError> validateCommonRegisters();

  /**
   * @brief Validate the inverter model block (I101–103 / I111–113).
   *
   * Reads the model ID register and rejects anything outside the known
   * SunSpec inverter IDs. Sets `id_` and `useFloatRegisters_`.
   */
  std::expected<void, ModbusError> validateInverterRegisters();

  /**
   * @brief Validate the Multi-MPPT extension block (I160).
   *
   * Verifies block ID and size, then reads the second input string name to
   * decide whether `inputs_` is 1 or 2.
   */
  std::expected<void, ModbusError> validateMultiMpptRegisters();

  /**
   * @brief Validate the basic storage control block (I124).
   *
   * Sets `hybrid_ = true` if the block is present and valid; sets
   * `hybrid_ = false` if the SunSpec end-of-map marker is found at this
   * address instead.
   */
  std::expected<void, ModbusError> validateStorageRegisters();

  /**
   * @brief Validate the nameplate block (I120).
   *
   * Verifies the block ID (120) and size. Nameplate data is available via
   * `getAcPowerRating()` once this succeeds.
   */
  std::expected<void, ModbusError> validateNameplateRegisters();

  /**
   * @brief Validate the SunSpec end-of-map block.
   *
   * Confirms that the end-block ID is `0xFFFF` and length is `0` at the
   * address that corresponds to the detected register encoding (integer
   * vs. float) and storage variant (hybrid vs. non-hybrid).
   */
  std::expected<void, ModbusError> validateEndRegisters();

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

#endif /* INVERTER_H_ */
