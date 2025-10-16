#include "meter.h"
#include "fronius_types.h"
#include "meter_registers.h"
#include "modbus_utils.h"
#include <array>
#include <cerrno>
#include <cmath>
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
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  uint16_t endBlockAddr =
      (useFloatRegisters_) ? M_END::ID.ADDR : M_END::ID.ADDR;
  int rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(endBlockAddr) + " failed")));
  }

  // Validate the end block
  uint16_t endBlockLength =
      (useFloatRegisters_) ? M_END::L.ADDR : M_END::L.ADDR;
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register end block: received [0x" +
                    ModbusUtils::toHex(regs_[endBlockAddr]) + ", " +
                    std::to_string(regs_[endBlockLength]) +
                    "], expected [0xFFFF, 0]")));
  }

  // Get the meter registers
  uint16_t meterBlockAddr = (useFloatRegisters_) ? M21X::A.ADDR : M20X::A.ADDR;
  uint16_t meterBlockSize = (useFloatRegisters_) ? M21X::SIZE : M20X::SIZE;

  rc = modbus_read_registers(ctx_, meterBlockAddr, meterBlockSize,
                             regs_.data() + meterBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(meterBlockAddr) + " failed")));
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
  auto invalidPhase = std::unexpected(
      ModbusError::custom(EINVAL, "Invalid phase in getAcCurrent(): " +
                                      std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M21X::A);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M21X::APHA);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M21X::APHB);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M21X::APHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M20X::A, M20X::A_SF);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M20X::APHA, M20X::A_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M20X::APHB, M20X::A_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M20X::APHC, M20X::A_SF);
    default:
      return invalidPhase;
    }
  }
}

std::expected<double, ModbusError>
Meter::getAcVoltage(const FroniusTypes::Phase ph) const {
  auto invalidPhase = std::unexpected(
      ModbusError::custom(EINVAL, "Invalid phase in getAcVoltage(): " +
                                      std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::Phase::AVERAGE:
      return getModbusDouble(regs_, M21X::PHV);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M21X::PHVPHA);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M21X::PHVPHB);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M21X::PHVPHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M20X::PHV, M20X::V_SF);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M20X::PHVPHA, M20X::V_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M20X::PHVPHB, M20X::V_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M20X::PHVPHC, M20X::V_SF);
    default:
      return invalidPhase;
    }
  }
}

std::expected<double, ModbusError> Meter::getAcFrequency(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, M21X::FREQ);
  } else {
    return getModbusDouble(regs_, M20X::FREQ, M20X::FREQ_SF);
  }
}

std::expected<double, ModbusError>
Meter::getAcPowerActive(const FroniusTypes::Phase ph) const {
  auto invalidPhase = std::unexpected(
      ModbusError::custom(EINVAL, "Invalid phase in getAcPowerActive(): " +
                                      std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M21X::W);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M21X::WPHA);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M21X::WPHB);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M21X::WPHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M20X::W, M20X::W_SF);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M20X::WPHA, M20X::W_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M20X::WPHB, M20X::W_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M20X::WPHC, M20X::W_SF);
    default:
      return invalidPhase;
    }
  }
}

std::expected<double, ModbusError>
Meter::getAcEnergyActiveExport(const FroniusTypes::Phase ph) const {
  auto invalidPhase = std::unexpected(ModbusError::custom(
      EINVAL, "Invalid phase in getAcEnergyActiveExport(): " +
                  std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M21X::TOTWH_EXP);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M21X::TOTWH_EXPPHA);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M21X::TOTWH_EXPPHB);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M21X::TOTWH_EXPPHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M20X::TOTWH_EXP, M20X::TOTWH_SF);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M20X::TOTWH_EXPPHA, M20X::TOTWH_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M20X::TOTWH_EXPPHB, M20X::TOTWH_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M20X::TOTWH_EXPPHC, M20X::TOTWH_SF);
    default:
      return invalidPhase;
    }
  }
}

std::expected<double, ModbusError>
Meter::getAcEnergyActiveImport(const FroniusTypes::Phase ph) const {
  auto invalidPhase = std::unexpected(ModbusError::custom(
      EINVAL, "Invalid phase in getAcEnergyActiveImport(): " +
                  std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M21X::TOTWH_IMP);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M21X::TOTWH_IMPPHA);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M21X::TOTWH_IMPPHB);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M21X::TOTWH_IMPPHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, M20X::TOTWH_IMP, M20X::TOTWH_SF);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, M20X::TOTWH_IMPPHA, M20X::TOTWH_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, M20X::TOTWH_IMPPHB, M20X::TOTWH_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, M20X::TOTWH_IMPPHC, M20X::TOTWH_SF);
    default:
      return invalidPhase;
    }
  }
}

/* -------------------------- private methods ------------------------------*/

std::expected<void, ModbusError> Meter::detectFloatOrIntRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  int rc = modbus_read_registers(ctx_, M20X::ID.ADDR, 2,
                                 regs_.data() + M20X::ID.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "Receive register " + std::to_string(M20X::ID.ADDR) + " failed")));
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
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter ID: received " + std::to_string(meterID) +
                    ", expected [" + oss.str() + "]")));
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
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register map size: received " +
                    std::to_string(regMapSize) + ", expected [" +
                    std::to_string(M20X::SIZE) + ", " +
                    std::to_string(M21X::SIZE) + "]")));
  }

  return {};
}