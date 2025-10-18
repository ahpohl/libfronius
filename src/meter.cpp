#include "meter.h"
#include "fronius_types.h"
#include "meter_registers.h"
#include "modbus_utils.h"
#include <array>
#include <cerrno>
#include <expected>
#include <iostream>
#include <modbus/modbus.h>
#include <modbus_config.h>
#include <modbus_error.h>
#include <sstream>
#include <vector>

/* ------------------------------ public API -------------------------------*/

Meter::Meter(const ModbusConfig &cfg) : Fronius(cfg) {}

Meter::~Meter() {};

std::expected<void, ModbusError> Meter::fetchMeterRegisters(void) {
  if (!ctx_) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        ENOTCONN, "fetchMeterRegisters(): Modbus context is null")));
  }

  // Validate the end block
  uint16_t endBlockAddr = (useFloatRegisters_)
                              ? M_END::ID.ADDR + M_END::FLOAT_OFFSET
                              : M_END::ID.ADDR;
  uint16_t endBlockLength = (useFloatRegisters_)
                                ? M_END::L.ADDR + M_END::FLOAT_OFFSET
                                : M_END::L.ADDR;

  int rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchMeterRegisters(): Receive end block register failed {}",
        endBlockAddr)));
  }
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "fetchMeterRegisters(): Invalid register end block: received "
        "[0x{}, {}], expected "
        "[0xFFFF, 0]",
        ModbusUtils::toHex(regs_[endBlockAddr]), regs_[endBlockLength])));
  }

  // Get the meter registers
  uint16_t meterBlockAddr = (useFloatRegisters_) ? M21X::A.ADDR : M20X::A.ADDR;
  uint16_t meterBlockSize = (useFloatRegisters_) ? M21X::SIZE : M20X::SIZE;

  rc = modbus_read_registers(ctx_, meterBlockAddr, meterBlockSize,
                             regs_.data() + meterBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchMeterRegisters(): Receive meter registers failed {}",
        meterBlockAddr)));
  }

  return {};
}

std::expected<void, ModbusError> Meter::validateDevice() {
  // Assume not valid until proven otherwise
  connectedAndValid_ = false;

  // --- Step 1: Check SunSpec signature ---
  auto sunspec = validateSunSpecRegisters();
  if (!sunspec)
    return std::unexpected(sunspec.error());

  // --- Step 2: Fetch common registers ---
  auto common = fetchCommonRegisters();
  if (!common)
    return common;

  // --- Step 3: Detect register map type & initialize meter ---
  auto init = detectFloatOrIntRegisters();
  if (!init)
    return std::unexpected(init.error());

  // If we got here, device is fully valid
  connectedAndValid_ = true;
  return {};
}

/* --------------------- get values -------------------------- */

std::expected<double, ModbusError>
Meter::getAcCurrent(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::A)
                              : getModbusDouble(regs_, M20X::A, M20X::A_SF);
  case FroniusTypes::Phase::A:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::APHA)
                              : getModbusDouble(regs_, M20X::APHA, M20X::A_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::APHB)
                              : getModbusDouble(regs_, M20X::APHB, M20X::A_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::APHC)
                              : getModbusDouble(regs_, M20X::APHC, M20X::A_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcCurrent(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcVoltage(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::PHV)
                              : getModbusDouble(regs_, M20X::PHV, M20X::V_SF);
  case FroniusTypes::Phase::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::PHVPHA)
               : getModbusDouble(regs_, M20X::PHVPHA, M20X::V_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::PHVPHB)
               : getModbusDouble(regs_, M20X::PHVPHB, M20X::V_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::PHVPHC)
               : getModbusDouble(regs_, M20X::PHVPHC, M20X::V_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcVoltage(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError> Meter::getAcFrequency(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, M21X::FREQ)
                            : getModbusDouble(regs_, M20X::FREQ, M20X::FREQ_SF);
}

std::expected<double, ModbusError>
Meter::getAcPowerActive(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::W)
                              : getModbusDouble(regs_, M20X::W, M20X::W_SF);
  case FroniusTypes::Phase::A:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::WPHA)
                              : getModbusDouble(regs_, M20X::WPHA, M20X::W_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::WPHB)
                              : getModbusDouble(regs_, M20X::WPHB, M20X::W_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_ ? getModbusDouble(regs_, M21X::WPHC)
                              : getModbusDouble(regs_, M20X::WPHC, M20X::W_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPowerActive(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcEnergyActiveExport(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_EXP)
               : getModbusDouble(regs_, M20X::TOTWH_EXP, M20X::TOTWH_SF);
  case FroniusTypes::Phase::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_EXPPHA)
               : getModbusDouble(regs_, M20X::TOTWH_EXPPHA, M20X::TOTWH_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_EXPPHB)
               : getModbusDouble(regs_, M20X::TOTWH_EXPPHB, M20X::TOTWH_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_EXPPHC)
               : getModbusDouble(regs_, M20X::TOTWH_EXPPHC, M20X::TOTWH_SF);
  default:
    return reportError<double>(std::unexpected(ModbusError::custom(
        EINVAL, "getAcEnergyActiveExport(): Invalid phase {}",
        FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcEnergyActiveImport(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_IMP)
               : getModbusDouble(regs_, M20X::TOTWH_IMP, M20X::TOTWH_SF);
  case FroniusTypes::Phase::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_IMPPHA)
               : getModbusDouble(regs_, M20X::TOTWH_IMPPHA, M20X::TOTWH_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_IMPPHB)
               : getModbusDouble(regs_, M20X::TOTWH_IMPPHB, M20X::TOTWH_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_
               ? getModbusDouble(regs_, M21X::TOTWH_IMPPHC)
               : getModbusDouble(regs_, M20X::TOTWH_IMPPHC, M20X::TOTWH_SF);
  default:
    return reportError<double>(std::unexpected(ModbusError::custom(
        EINVAL, "getAcEnergyActiveImport(): Invalid phase {}",
        FroniusTypes::toString(ph))));
  }
}

/* -------------------------- private methods ------------------------------*/

std::expected<void, ModbusError> Meter::detectFloatOrIntRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        ENOTCONN, "detectFloatOrIntRegisters(): Modbus context is null")));
  }

  int rc = modbus_read_registers(ctx_, M20X::ID.ADDR, 2,
                                 regs_.data() + M20X::ID.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "detectFloatOrIntRegisters(): Receive register failed {}",
        M20X::ID.describe())));
  }

  // Validate meter ID
  uint16_t meterID = regs_[M20X::ID.ADDR];
  static constexpr std::array<uint16_t, 6> validMeterIDs = {201, 202, 203,
                                                            211, 212, 213};

  if (std::find(validMeterIDs.begin(), validMeterIDs.end(), meterID) ==
      validMeterIDs.end()) {
    std::ostringstream oss;
    bool first = true;
    for (auto id : validMeterIDs) {
      if (!first)
        oss << ", ";
      oss << id;
      first = false;
    }
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "detectFloatOrIntRegisters(): Invalid meter ID: "
                            "received {}, expected [{}]",
                            meterID, oss.str())));
  }

  // Store meter ID
  id_ = meterID;

  // Set register model
  if (meterID / 10 % 10)
    useFloatRegisters_ = true;
  else
    useFloatRegisters_ = false;

  // Validate the register length
  uint16_t regMapSize = regs_[M20X::L.ADDR];
  if (regMapSize != M20X::SIZE && regMapSize != M21X::SIZE) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "detectFloatOrIntRegisters(): Invalid meter "
                            "register map size: received {}, expected [{}, {}]",
                            regMapSize, M20X::SIZE, M21X::SIZE)));
  }

  return {};
}