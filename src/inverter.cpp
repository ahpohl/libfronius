#include "inverter.h"
#include "inverter_registers.h"
#include "modbus_utils.h"
#include <array>
#include <cerrno>
#include <expected>
#include <fronius_types.h>
#include <iostream>
#include <modbus/modbus.h>
#include <modbus_config.h>
#include <modbus_error.h>
#include <sstream>
#include <vector>

/* ------------------------------ public API -------------------------------*/

Inverter::Inverter(const ModbusConfig &cfg) : Fronius(cfg) {}

Inverter::~Inverter() {};

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
                    ModbusUtils::toHex(regs_[endBlockAddr]) + ", " +
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

std::expected<double, ModbusError>
Inverter::getAcCurrent(const FroniusTypes::Phase ph) const {
  auto invalidPhase = std::unexpected(
      ModbusError::custom(EINVAL, "Invalid phase in getAcCurrent(): " +
                                      std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, I11X::A);
    case FroniusTypes::FroniusTypes::Phase::A:
      return getModbusDouble(regs_, I11X::APHA);
    case FroniusTypes::FroniusTypes::Phase::B:
      return getModbusDouble(regs_, I11X::APHB);
    case FroniusTypes::FroniusTypes::Phase::C:
      return getModbusDouble(regs_, I11X::APHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::TOTAL:
      return getModbusDouble(regs_, I10X::A, I10X::A_SF);
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, I10X::APHA, I10X::A_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, I10X::APHB, I10X::A_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, I10X::APHC, I10X::A_SF);
    default:
      return invalidPhase;
    }
  }
}

std::expected<double, ModbusError>
Inverter::getAcVoltage(const FroniusTypes::Phase ph) const {
  auto invalidPhase = std::unexpected(
      ModbusError::custom(EINVAL, "Invalid phase in getAcVoltage(): " +
                                      std::string(FroniusTypes::toString(ph))));

  if (useFloatRegisters_) {
    switch (ph) {
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, I11X::PHVPHA);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, I11X::PHVPHB);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, I11X::PHVPHC);
    default:
      return invalidPhase;
    }
  } else {
    switch (ph) {
    case FroniusTypes::Phase::A:
      return getModbusDouble(regs_, I10X::PHVPHA, I10X::V_SF);
    case FroniusTypes::Phase::B:
      return getModbusDouble(regs_, I10X::PHVPHB, I10X::V_SF);
    case FroniusTypes::Phase::C:
      return getModbusDouble(regs_, I10X::PHVPHC, I10X::V_SF);
    default:
      return invalidPhase;
    }
  }
}

std::expected<double, ModbusError> Inverter::getAcFrequency(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, I11X::FREQ);
  } else {
    return getModbusDouble(regs_, I10X::FREQ, I10X::FREQ_SF);
  }
}

std::expected<double, ModbusError> Inverter::getAcPowerActive(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, I11X::W);
  } else {
    return getModbusDouble(regs_, I10X::W, I10X::W_SF);
  }
}

std::expected<double, ModbusError> Inverter::getAcPowerApparent(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, I11X::VA);
  } else {
    return getModbusDouble(regs_, I10X::VA, I10X::VA_SF);
  }
}

std::expected<double, ModbusError> Inverter::getAcPowerReactive(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, I11X::VAR);
  } else {
    return getModbusDouble(regs_, I10X::VAR, I10X::VAR_SF);
  }
}

std::expected<double, ModbusError> Inverter::getAcPowerFactor(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, I11X::PF);
  } else {
    return getModbusDouble(regs_, I10X::PF, I10X::PF_SF);
  }
}

std::expected<double, ModbusError> Inverter::getAcEnergy(void) const {
  if (useFloatRegisters_) {
    return getModbusDouble(regs_, I11X::WH);
  } else {
    return getModbusDouble(regs_, I10X::WH, I10X::WH_SF);
  }
}

std::expected<double, ModbusError>
Inverter::getDcPower(const FroniusTypes::Input input) const {
  auto invalidInput = std::unexpected(ModbusError::custom(
      EINVAL, "Invalid input in getDcPower(): " +
                  std::string(FroniusTypes::toString(input))));

  if (useFloatRegisters_) {
    switch (input) {
    case FroniusTypes::Input::TOTAL:
      return getModbusDouble(regs_, I11X::DCW);
    case FroniusTypes::Input::A:
      return getModbusDouble(regs_, I160::DCW_1.withOffset(I160::FLOAT_OFFSET));
    case FroniusTypes::Input::B:
      return getModbusDouble(regs_, I160::DCW_2.withOffset(I160::FLOAT_OFFSET));
    default:
      return invalidInput;
    }
  } else {
    switch (input) {
    case FroniusTypes::Input::TOTAL:
      return getModbusDouble(regs_, I10X::DCW, I10X::DCW_SF);
    case FroniusTypes::Input::A:
      return getModbusDouble(regs_, I160::DCW_1, I160::DCW_SF);
    case FroniusTypes::Input::B:
      return getModbusDouble(regs_, I160::DCW_2, I160::DCW_SF);
    default:
      return invalidInput;
    }
  }
}

std::expected<double, ModbusError>
Inverter::getDcCurrent(const FroniusTypes::Input input) const {
  auto invalidInput = std::unexpected(ModbusError::custom(
      EINVAL, "Invalid input in getDcCurrent(): " +
                  std::string(FroniusTypes::toString(input))));

  if (useFloatRegisters_) {
    switch (input) {
    case FroniusTypes::Input::TOTAL:
      return getModbusDouble(regs_, I11X::DCA);
    case FroniusTypes::Input::A:
      return getModbusDouble(regs_, I160::DCA_1.withOffset(I160::FLOAT_OFFSET));
    case FroniusTypes::Input::B:
      return getModbusDouble(regs_, I160::DCA_2.withOffset(I160::FLOAT_OFFSET));
    default:
      return invalidInput;
    }
  } else {
    switch (input) {
    case FroniusTypes::Input::TOTAL:
      return getModbusDouble(regs_, I10X::DCA, I10X::DCA_SF);
    case FroniusTypes::Input::A:
      return getModbusDouble(regs_, I160::DCA_1, I160::DCA_SF);
    case FroniusTypes::Input::B:
      return getModbusDouble(regs_, I160::DCA_2, I160::DCA_SF);
    default:
      return invalidInput;
    }
  }
}

std::expected<double, ModbusError>
Inverter::getDcVoltage(const FroniusTypes::Input input) const {
  auto invalidInput = std::unexpected(ModbusError::custom(
      EINVAL, "Invalid input in getDcVoltage(): " +
                  std::string(FroniusTypes::toString(input))));

  if (useFloatRegisters_) {
    switch (input) {
    case FroniusTypes::Input::TOTAL:
      return getModbusDouble(regs_, I11X::DCV);
    case FroniusTypes::Input::A:
      return getModbusDouble(regs_, I160::DCV_1.withOffset(I160::FLOAT_OFFSET));
    case FroniusTypes::Input::B:
      return getModbusDouble(regs_, I160::DCV_2.withOffset(I160::FLOAT_OFFSET));
    default:
      return invalidInput;
    }
  } else {
    switch (input) {
    case FroniusTypes::Input::TOTAL:
      return getModbusDouble(regs_, I10X::DCV, I10X::DCV_SF);
    case FroniusTypes::Input::A:
      return getModbusDouble(regs_, I160::DCV_1, I160::DCV_SF);
    case FroniusTypes::Input::B:
      return getModbusDouble(regs_, I160::DCV_2, I160::DCV_SF);
    default:
      return invalidInput;
    }
  }
}

/* -------------------------- private methods ------------------------------*/

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