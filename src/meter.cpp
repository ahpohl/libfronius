#include "meter.h"
#include "meter_float_registers.h"
#include "meter_intsf_registers.h"
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

Meter::Meter(const ModbusConfig &cfg) : Fronius(cfg) {}

Meter::~Meter() {};

bool Meter::getUseFloatRegisters(void) const { return useFloatRegisters_; }

int Meter::getPhases(void) const { return id_ % 10; };

int Meter::getId(void) const { return id_; };

std::expected<void, ModbusError> Meter::detectFloatOrIntRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  int rc = modbus_read_registers(ctx_, M20X_ID::ADDR, 2,
                                 regs_.data() + M20X_ID::ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "Receive register " + std::to_string(M20X_ID::ADDR) + " failed")));
  }

  // Validate meter ID
  uint16_t meterID = regs_[M20X_ID::ADDR];
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
  uint16_t regMapSize = regs_[M20X_L::ADDR];
  if (regMapSize != M20X_SIZE && regMapSize != M21X_SIZE) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register map size: received " +
                    std::to_string(regMapSize) + ", expected [" +
                    std::to_string(M20X_SIZE) + ", " +
                    std::to_string(M21X_SIZE) + "]")));
  }

  return {};
}

std::expected<void, ModbusError> Meter::fetchMeterRegisters(void) {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  uint16_t endBlockAddr = (useFloatRegisters_) ? E21X_ID::ADDR : E20X_ID::ADDR;
  int rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(endBlockAddr) + " failed")));
  }

  // Validate the end block
  uint16_t endBlockLength = (useFloatRegisters_) ? E21X_L::ADDR : E20X_L::ADDR;
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register end block: received [0x" +
                    modbus_utils::to_hex(regs_[endBlockAddr]) + ", " +
                    std::to_string(regs_[endBlockLength]) +
                    "], expected [0xFFFF, 0]")));
  }

  // Get the meter registers
  uint16_t meterBlockAddr = (useFloatRegisters_) ? M21X_A::ADDR : M20X_A::ADDR;
  uint16_t meterBlockSize = (useFloatRegisters_) ? M21X_SIZE : M20X_SIZE;

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

double Meter::getAcCurrent(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::TOTAL:
      return modbus_get_float_abcd(regs_.data() + M21X_A::ADDR);
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + M21X_APHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + M21X_APHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + M21X_APHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::TOTAL:
      return static_cast<double>(regs_[M20X_A::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_A_SF::ADDR]));
    case Phase::PHA:
      return static_cast<double>(regs_[M20X_APHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_A_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[M20X_APHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_A_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[M20X_APHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_A_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}

double Meter::getAcVoltage(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::AVERAGE:
      return modbus_get_float_abcd(regs_.data() + M21X_PHV::ADDR);
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + M21X_PHVPHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + M21X_PHVPHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + M21X_PHVPHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::AVERAGE:
      return static_cast<double>(regs_[M20X_PHV::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_V_SF::ADDR]));
    case Phase::PHA:
      return static_cast<double>(regs_[M20X_PHVPHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_V_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[M20X_PHVPHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_V_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[M20X_PHVPHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_V_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}

double Meter::getAcPowerActive(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::TOTAL:
      return modbus_get_float_abcd(regs_.data() + M21X_W::ADDR);
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + M21X_WPHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + M21X_WPHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + M21X_WPHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::TOTAL:
      return static_cast<double>(regs_[M20X_W::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_W_SF::ADDR]));
    case Phase::PHA:
      return static_cast<double>(regs_[M20X_WPHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_W_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[M20X_WPHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_W_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[M20X_WPHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_W_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}

double Meter::getAcEnergyActiveExport(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::TOTAL:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_EXP::ADDR);
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_EXPPHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_EXPPHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_EXPPHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::TOTAL:
      return static_cast<double>(regs_[M20X_TOTWH_EXP::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    case Phase::PHA:
      return static_cast<double>(regs_[M20X_TOTWH_EXPPHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[M20X_TOTWH_EXPPHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[M20X_TOTWH_EXPPHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}

double Meter::getAcEnergyActiveImport(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::TOTAL:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_IMP::ADDR);
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_IMPPHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_IMPPHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + M21X_TOTWH_IMPPHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::TOTAL:
      return static_cast<double>(regs_[M20X_TOTWH_IMP::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    case Phase::PHA:
      return static_cast<double>(regs_[M20X_TOTWH_IMPPHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[M20X_TOTWH_IMPPHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[M20X_TOTWH_IMPPHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}