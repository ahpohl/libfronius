#include "inverter.h"
#include "inverter_registers.h"
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

  int rc = modbus_read_registers(ctx_, I10X::ID.ADDR, 2,
                                 regs_.data() + I10X::ID.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "Receive register " + std::to_string(I10X::ID.ADDR) + " failed")));
  }

  // Validate inverter ID
  uint16_t inverterID = regs_[I10X::ID.ADDR];
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
  uint16_t regMapSize = regs_[I10X::L.ADDR];
  if (regMapSize != I10X::SIZE && regMapSize != I11X::SIZE) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid inverter register map size: received " +
                    std::to_string(regMapSize) + ", expected [" +
                    std::to_string(I10X::SIZE) + ", " +
                    std::to_string(I11X::SIZE) + "]")));
  }

  return {};
}

std::expected<void, ModbusError> Inverter::fetchInverterRegisters(void) {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  int rc;

  uint16_t endBlockAddr = (useFloatRegisters_)
                              ? I_END::ID.ADDR + I_END::FLOAT_OFFSET
                              : I_END::ID.ADDR;
  rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(endBlockAddr) + " failed")));
  }

  // Validate the end block
  uint16_t endBlockLength = (useFloatRegisters_)
                                ? I_END::L.ADDR + I_END::FLOAT_OFFSET
                                : I_END::L.ADDR;
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid inverter register end block: received [0x" +
                    modbus_utils::to_hex(regs_[endBlockAddr]) + ", " +
                    std::to_string(regs_[endBlockLength]) +
                    "], expected [0xFFFF, 0]")));
  }

  // Get the inverter registers
  uint16_t inverterBlockAddr =
      (useFloatRegisters_) ? I11X::A.ADDR : I10X::A.ADDR;
  uint16_t inverterBlockSize = (useFloatRegisters_) ? I11X::SIZE : I10X::SIZE;

  rc = modbus_read_registers(ctx_, inverterBlockAddr, inverterBlockSize,
                             regs_.data() + inverterBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(inverterBlockAddr) + " failed")));
  }

  // Get the Multi MPPT inverter extension registers
  uint16_t multiMpptBlockAddr = (useFloatRegisters_)
                                    ? I160::DCA_SF.ADDR + +I160::FLOAT_OFFSET
                                    : I160::DCA_SF.ADDR;

  rc = modbus_read_registers(ctx_, multiMpptBlockAddr, I160::SIZE,
                             regs_.data() + multiMpptBlockAddr);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        std::string("Receive register ") + std::to_string(multiMpptBlockAddr) +
        " failed")));
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
      return modbus_get_float_abcd(regs_.data() + I11X::A.ADDR);
    case Phase::A:
      return modbus_get_float_abcd(regs_.data() + I11X::APHA.ADDR);
    case Phase::B:
      return modbus_get_float_abcd(regs_.data() + I11X::APHB.ADDR);
    case Phase::C:
      return modbus_get_float_abcd(regs_.data() + I11X::APHC.ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::TOTAL:
      return static_cast<double>(regs_[I10X::A.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::A_SF.ADDR]));
    case Phase::A:
      return static_cast<double>(regs_[I10X::APHA.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::A_SF.ADDR]));
    case Phase::B:
      return static_cast<double>(regs_[I10X::APHB.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::A_SF.ADDR]));
    case Phase::C:
      return static_cast<double>(regs_[I10X::APHC.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::A_SF.ADDR]));
    default:
      return 0.0;
    }
  }
}

double Inverter::getAcVoltage(const Phase ph) const {
  if (useFloatRegisters_) {
    switch (ph) {
    case Phase::A:
      return modbus_get_float_abcd(regs_.data() + I11X::PHVPHA.ADDR);
    case Phase::B:
      return modbus_get_float_abcd(regs_.data() + I11X::PHVPHB.ADDR);
    case Phase::C:
      return modbus_get_float_abcd(regs_.data() + I11X::PHVPHC.ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (ph) {
    case Phase::A:
      return static_cast<double>(regs_[I10X::PHVPHA.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::V_SF.ADDR]));
    case Phase::B:
      return static_cast<double>(regs_[I10X::PHVPHB.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::V_SF.ADDR]));
    case Phase::C:
      return static_cast<double>(regs_[I10X::PHVPHC.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::V_SF.ADDR]));
    default:
      return 0.0;
    }
  }
}

double Inverter::getAcFrequency(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X::FREQ.ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X::FREQ.ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X::FREQ_SF.ADDR]));
  }
}

double Inverter::getAcPowerActive(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X::W.ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X::W.ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X::W_SF.ADDR]));
  }
}

double Inverter::getAcPowerApparent(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X::VA.ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X::VA.ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X::VA_SF.ADDR]));
  }
}

double Inverter::getAcPowerReactive(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X::VAR.ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X::VAR.ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X::VAR_SF.ADDR]));
  }
}

double Inverter::getAcPowerFactor(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X::PF.ADDR);
  } else {
    return static_cast<double>(static_cast<int16_t>(regs_[I10X::PF.ADDR])) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X::PF_SF.ADDR]));
  }
}

double Inverter::getAcEnergy(void) const {
  if (useFloatRegisters_) {
    return modbus_get_float_abcd(regs_.data() + I11X::WH.ADDR);
  } else {
    return static_cast<double>(
               modbus_utils::modbus_get_uint32(regs_.data() + I10X::WH.ADDR)) *
           std::pow(10.0, static_cast<int16_t>(regs_[I10X::WH_SF.ADDR]));
  }
}

double Inverter::getDcPower(const Input input) const {
  if (useFloatRegisters_) {
    switch (input) {
    case Input::TOTAL:
      return modbus_get_float_abcd(regs_.data() + I11X::DCW.ADDR);
    case Input::A:
      return modbus_get_float_abcd(regs_.data() + I160::DCW_1.ADDR);
    case Input::B:
      return modbus_get_float_abcd(regs_.data() + I160::DCW_2.ADDR);
    default:
      return 0.0;
    }
  } else {
    switch (input) {
    case Input::TOTAL:
      return static_cast<double>(regs_[I10X::DCW.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I10X::DCW_SF.ADDR]));
    case Input::A:
      return static_cast<double>(regs_[I160::DCW_1.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I160::DCW_SF.ADDR]));
    case Input::B:
      return static_cast<double>(regs_[I160::DCW_2.ADDR]) *
             std::pow(10.0, static_cast<int16_t>(regs_[I160::DCW_SF.ADDR]));
    default:
      return 0.0;
    }
  }
}
