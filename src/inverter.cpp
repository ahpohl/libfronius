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

Inverter::Inverter(const ModbusConfig &cfg) : Fronius(cfg) {}

Inverter::~Inverter() {}

void Inverter::checkInitialized() const {
  if (!isInitialized_) {
    throw std::logic_error(
        "Device not initialized: call detectAndInitialize() first");
  }
}

bool Inverter::getUseFloatRegisters(void) const {
  checkInitialized();
  return useFloatRegisters_;
}

int Inverter::getPhases(void) const {
  checkInitialized();
  return phases_;
};

std::expected<int, ModbusError> Inverter::detectAndInitialize() {
  std::fill(regs_.begin(), regs_.end(), 0);

  int rc = modbus_read_registers(ctx_, I10X_ID::ADDR, 2,
                                 regs_.data() + I10X_ID::ADDR);
  if (rc == -1) {
    return std::unexpected(ModbusError::fromErrno(
        "Receive register " + std::to_string(I10X_ID::ADDR) + " failed",
        ModbusError::Severity::TRANSIENT));
  }

  // Validate meter ID
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
    return std::unexpected(ModbusError::custom(
        EINVAL,
        "Invalid meter ID: received " + std::to_string(inverterID) +
            ", expected [" + oss.str() + "]",
        ModbusError::Severity::FATAL));
  }

  if (inverterID / 10 % 10)
    useFloatRegisters_ = true;
  else
    useFloatRegisters_ = false;

  // Validate the register length
  uint16_t regMapSize = regs_[I10X_L::ADDR];
  if (regMapSize != I10X_SIZE && regMapSize != I11X_SIZE) {
    return std::unexpected(ModbusError::custom(
        EINVAL,
        "Invalid meter register map size: received " +
            std::to_string(regMapSize) + ", expected [" +
            std::to_string(I10X_SIZE) + ", " + std::to_string(I11X_SIZE) + "]",
        ModbusError::Severity::FATAL));
  }

  // Store number of phases
  phases_ = inverterID % 10;

  isInitialized_ = true;
  return inverterID;
}

std::expected<void, ModbusError> Inverter::fetchInverterRegisters(void) {
  checkInitialized();

  uint16_t endBlockAddr = (useFloatRegisters_) ? E11X_ID::ADDR : E10X_ID::ADDR;
  int rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                                   std::to_string(endBlockAddr) + " failed",
                               ModbusError::Severity::TRANSIENT));
  }

  // Validate the end block
  uint16_t endBlockLength = (useFloatRegisters_) ? E11X_L::ADDR : E10X_L::ADDR;
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return std::unexpected(ModbusError::custom(
        EINVAL,
        "Invalid meter register end block: received [0x" +
            modbus_utils::to_hex(regs_[endBlockAddr]) + ", " +
            std::to_string(regs_[endBlockLength]) + "], expected [0xFFFF, 0]",
        ModbusError::Severity::FATAL));
  }

  // Get the inverter registers
  uint16_t inverterBlockAddr =
      (useFloatRegisters_) ? I11X_A::ADDR : I10X_A::ADDR;
  uint16_t inverterBlockSize = (useFloatRegisters_) ? I11X_SIZE : I10X_SIZE;

  rc = modbus_read_registers(ctx_, inverterBlockAddr, inverterBlockSize,
                             regs_.data() + inverterBlockAddr);
  if (rc == -1) {
    return std::unexpected(ModbusError::fromErrno(
        std::string("Receive register ") + std::to_string(inverterBlockAddr) +
            " failed",
        ModbusError::Severity::TRANSIENT));
  }

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
    case Phase::PHAB:
      return modbus_get_float_abcd(regs_.data() + I11X_PPVPHAB::ADDR);
    case Phase::PHBC:
      return modbus_get_float_abcd(regs_.data() + I11X_PPVPHBC::ADDR);
    case Phase::PHCA:
      return modbus_get_float_abcd(regs_.data() + I11X_PPVPHCA::ADDR);
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
    case Phase::PHAB:
      return static_cast<double>(regs_[I10X_PPVPHAB::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_V_SF::ADDR]));
    case Phase::PHBC:
      return static_cast<double>(regs_[I10X_PPVPHBC::ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X_V_SF::ADDR]));
    case Phase::PHCA:
      return static_cast<double>(regs_[I10X_PPVPHCA::ADDR]) *
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
    return static_cast<double>(regs_[I10X_W::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_W_SF::ADDR]));
  }
}

double Inverter::getAcPowerApparent(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_VA::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_VA::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_VA_SF::ADDR]));
  }
}

double Inverter::getAcPowerReactive(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_VAR::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_VAR::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_VAR_SF::ADDR]));
  }
}

double Inverter::getAcFrequency(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_HZ::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_HZ::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_HZ_SF::ADDR]));
  }
}

double Inverter::getAcPowerFactor(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_PF::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_PF::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_PF_SF::ADDR]));
  }
}

double Inverter::getAcEnergy(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_WH::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_WH::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_WH_SF::ADDR]));
  }
}

double Inverter::getDcCurrent(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_DCA::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_DCA::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_DCA_SF::ADDR]));
  }
}

double Inverter::getDcVoltage(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_DCV::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_DCV::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_DCV_SF::ADDR]));
  }
}

double Inverter::getDcPower(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X_DCW::ADDR);
  } else {
    return static_cast<double>(regs_[I10X_DCW::ADDR]) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X_DCW_SF::ADDR]));
  }
}
