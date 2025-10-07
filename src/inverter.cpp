#include "inverter.h"
#include "inverter_float_registers.h"
#include "inverter_intsf_registers.h"
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

Inverter::Inverter(const ModbusConfig &cfg) : Fronius(cfg) {}

Inverter::~Inverter() {};

bool Inverter::getUseFloatRegisters(void) const { return useFloatRegisters_; }

int Inverter::getPhases(void) const { return id_ % 10; };

int Inverter::getId(void) const { return id_; };

std::expected<void, ModbusError> Inverter::detectFloatOrIntRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  int rc = modbus_read_registers(ctx_, I10X_ID::ADDR, 2,
                                 regs_.data() + I10X_ID::ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "Receive register " + std::to_string(I10X_ID::ADDR) + " failed")));
  }

  // Validate inverter ID
  uint16_t inverterID = regs_[I10X_ID::ADDR];
  static constexpr std::array<uint16_t, 6> validInverterIDs = {101, 102, 103,
                                                               111, 112, 113};

  if (std::find(validInverterIDs.begin(), validInverterIDs.end(), inverterID) ==
      validInverterIDs.end()) {
    std::ostringstream oss;
    bool first = true;
    for (auto id : validInverterIDs) {
      if (!first)
        oss << ", ";
      oss << id;
      first = false;
    }
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid inverter ID: received " + std::to_string(inverterID) +
                    ", expected [" + oss.str() + "]")));
  }

  // Store inverter ID
  id_ = inverterID;

  // Set register model
  if (inverterID / 10 % 10)
    useFloatRegisters_ = true;
  else
    useFloatRegisters_ = false;

  // Validate the register length
  uint16_t regMapSize = regs_[I10X_L::ADDR];
  if (regMapSize != I10X_SIZE && regMapSize != I11X_SIZE) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid inverter register map size: received " +
                    std::to_string(regMapSize) + ", expected [" +
                    std::to_string(I10X_SIZE) + ", " +
                    std::to_string(I11X_SIZE) + "]")));
  }

  return {};
}

std::expected<void, ModbusError> Inverter::fetchInverterRegisters(void) {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  uint16_t endBlockAddr = (useFloatRegisters_) ? E11X_ID::ADDR : E10X_ID::ADDR;
  int rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(endBlockAddr) + " failed")));
  }

  // Validate the end block
  uint16_t endBlockLength = (useFloatRegisters_) ? E11X_L::ADDR : E10X_L::ADDR;
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid inverter register end block: received [0x" +
                    modbus_utils::to_hex(regs_[endBlockAddr]) + ", " +
                    std::to_string(regs_[endBlockLength]) +
                    "], expected [0xFFFF, 0]")));
  }

  // Get the inverter registers
  uint16_t inverterBlockAddr =
      (useFloatRegisters_) ? I11X_A::ADDR : I10X_A::ADDR;
  uint16_t inverterBlockSize = (useFloatRegisters_) ? I11X_SIZE : I10X_SIZE;

  rc = modbus_read_registers(ctx_, inverterBlockAddr, inverterBlockSize,
                             regs_.data() + inverterBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(inverterBlockAddr) + " failed")));
  }

  return {};
}

std::expected<void, ModbusError> Inverter::validateDevice() {
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

  // --- Step 3: Detect register map type & initialize inverter ---
  auto init = detectFloatOrIntRegisters();
  if (!init)
    return std::unexpected(init.error());

  // If we got here, device is fully valid
  connectedAndValid_ = true;
  return {};
}

/* --------------------- get values -------------------------- */

double Inverter::getAcCurrent(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::TOTAL:
      return modbus_get_float_abcd(regs_.data() + I11X_A::ADDR);
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + I11X_APHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + I11X_APHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + I11X_APHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::TOTAL:
      return static_cast<double>(regs_[I10X_A::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_A_SF::ADDR]));
    case Phase::PHA:
      return static_cast<double>(regs_[I10X_APHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_A_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[I10X_APHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_A_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[I10X_APHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_A_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}

double Inverter::getAcVoltage(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::PHA:
      return modbus_get_float_abcd(regs_.data() + I11X_PHVPHA::ADDR);
    case Phase::PHB:
      return modbus_get_float_abcd(regs_.data() + I11X_PHVPHB::ADDR);
    case Phase::PHC:
      return modbus_get_float_abcd(regs_.data() + I11X_PHVPHC::ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::PHA:
      return static_cast<double>(regs_[I10X_PHVPHA::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_V_SF::ADDR]));
    case Phase::PHB:
      return static_cast<double>(regs_[I10X_PHVPHB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_V_SF::ADDR]));
    case Phase::PHC:
      return static_cast<double>(regs_[I10X_PHVPHC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_V_SF::ADDR]));
    default:
      return 0.0;
    }
  }
}

double Inverter::getAcPowerActive(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_W::ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X_W::ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_W_SF::ADDR]));
  }
}

double Inverter::getAcPowerApparent(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_VA::ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X_VA::ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_VA_SF::ADDR]));
  }
}

double Inverter::getAcPowerReactive(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_VAR::ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X_VAR::ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_VAR_SF::ADDR]));
  }
}

double Inverter::getAcPowerFactor(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_PF::ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X_PF::ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_PF_SF::ADDR]));
  }
}

double Inverter::getAcEnergy(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_WH::ADDR);
  } else {
    return static_cast<double>(
               modbus_utils::modbus_get_uint32(regs_.data() + I10X_WH::ADDR)) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_WH_SF::ADDR]));
  }
}

double Inverter::getDcPower(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_DCW::ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X_DCW::ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_DCW_SF::ADDR]));
  }
}
