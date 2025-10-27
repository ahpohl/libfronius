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
    return reportError<void>(std::unexpected(ModbusError::custom(
        ENOTCONN, "fetchInverterRegisters(): Modbus context is null")));
  }

  // Validate the end block
  auto endBlockBaseReg = I_END::ID;
  if (useFloatRegisters_)
    endBlockBaseReg = endBlockBaseReg.withOffset(I_END::FLOAT_OFFSET);
  if (hybrid_)
    endBlockBaseReg = endBlockBaseReg.withOffset(I_END::STORAGE_OFFSET);

  int rc = modbus_read_registers(ctx_, endBlockBaseReg.ADDR, 2,
                                 regs_.data() + endBlockBaseReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchInverterRegisters(): Receive end block register failed {}",
        endBlockBaseReg.describe())));
  }

  auto endBlockLengthReg = I_END::L;
  if (useFloatRegisters_)
    endBlockLengthReg = endBlockLengthReg.withOffset(I_END::FLOAT_OFFSET);
  if (hybrid_)
    endBlockLengthReg = endBlockLengthReg.withOffset(I_END::STORAGE_OFFSET);

  if (!(regs_[endBlockBaseReg.ADDR] == 0xFFFF &&
        regs_[endBlockLengthReg.ADDR] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "fetchInverterRegisters(): Invalid end block register: "
        "received [0x{}, {}], expected "
        "[0xFFFF, 0]",
        ModbusUtils::toHex(regs_[endBlockBaseReg.ADDR]),
        endBlockLengthReg.ADDR)));
  }

  // Get the inverter registers
  const auto &inverterBaseReg = useFloatRegisters_ ? I11X::A : I10X::A;
  const uint16_t inverterBlockSize =
      (useFloatRegisters_) ? I11X::SIZE : I10X::SIZE;

  rc = modbus_read_registers(ctx_, inverterBaseReg.ADDR, inverterBlockSize,
                             regs_.data() + inverterBaseReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchInverterRegisters(): Receive inverter registers failed {}",
        inverterBaseReg.describe())));
  }

  // Get the Multi MPPT inverter extension registers
  const auto &multiMpptBaseReg =
      useFloatRegisters_ ? I160::DCA_SF.withOffset(I160::FLOAT_OFFSET)
                         : I160::DCA_SF;

  rc = modbus_read_registers(ctx_, multiMpptBaseReg.ADDR, I160::SIZE,
                             regs_.data() + multiMpptBaseReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchInverterRegisters(): Receive MPPT registers failed {}",
        multiMpptBaseReg.describe())));
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

  // --- Step 4: Validate multi MPPT map and detect number of inputs
  auto mppt = validateMultiMpptRegisters();
  if (!mppt)
    return std::unexpected(mppt.error());

  // --- Step 5: Detect if hybrid and validate storage control registers
  auto storage = validateStorageRegisters();
  if (!storage)
    return std::unexpected(storage.error());

  // If we got here, device is fully valid
  connectedAndValid_ = true;
  return {};
}

/* --------------------- get values -------------------------- */

std::expected<double, ModbusError>
Inverter::getAcCurrent(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::A)
                              : getModbusDouble(regs_, I10X::A, I10X::A_SF);
  case FroniusTypes::Phase::A:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::APHA)
                              : getModbusDouble(regs_, I10X::APHA, I10X::A_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::APHB)
                              : getModbusDouble(regs_, I10X::APHB, I10X::A_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::APHC)
                              : getModbusDouble(regs_, I10X::APHC, I10X::A_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcCurrent(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Inverter::getAcVoltage(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_, I11X::PHVPHA)
               : getModbusDouble(regs_, I10X::PHVPHA, I10X::V_SF);
  case FroniusTypes::Phase::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_, I11X::PHVPHB)
               : getModbusDouble(regs_, I10X::PHVPHB, I10X::V_SF);
  case FroniusTypes::Phase::C:
    return useFloatRegisters_
               ? getModbusDouble(regs_, I11X::PHVPHC)
               : getModbusDouble(regs_, I10X::PHVPHC, I10X::V_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcVoltage(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError> Inverter::getAcFrequency(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::FREQ)
                            : getModbusDouble(regs_, I10X::FREQ, I10X::FREQ_SF);
}

std::expected<double, ModbusError> Inverter::getAcPowerActive(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::W)
                            : getModbusDouble(regs_, I10X::W, I10X::W_SF);
}

std::expected<double, ModbusError> Inverter::getAcPowerApparent(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::VA)
                            : getModbusDouble(regs_, I10X::VA, I10X::VA_SF);
}

std::expected<double, ModbusError> Inverter::getAcPowerReactive(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::VAR)
                            : getModbusDouble(regs_, I10X::VAR, I10X::VAR_SF);
}

std::expected<double, ModbusError> Inverter::getAcPowerFactor(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::PF)
                            : getModbusDouble(regs_, I10X::PF, I10X::PF_SF);
}

std::expected<double, ModbusError> Inverter::getAcEnergy(void) const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::WH)
                            : getModbusDouble(regs_, I10X::WH, I10X::WH_SF);
}

std::expected<double, ModbusError>
Inverter::getDcPower(const FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::DCW)
                              : getModbusDouble(regs_, I10X::DCW, I10X::DCW_SF);
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCW_1.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCW_1, I160::DCW_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCW_2.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCW_2, I160::DCW_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcPower(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

std::expected<double, ModbusError>
Inverter::getDcCurrent(const FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::DCA)
                              : getModbusDouble(regs_, I10X::DCA, I10X::DCA_SF);
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCA_1.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCA_1, I160::DCA_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCA_2.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCA_2, I160::DCA_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcCurrent(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

std::expected<double, ModbusError>
Inverter::getDcVoltage(const FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::DCV)
                              : getModbusDouble(regs_, I10X::DCV, I10X::DCV_SF);
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCV_1.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCV_1, I160::DCV_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCV_2.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCV_2, I160::DCV_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcVoltage(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

std::expected<double, ModbusError>
Inverter::getDcEnergy(const FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCWH_1.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCWH_1, I160::DCWH_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCWH_2.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCWH_2, I160::DCWH_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcEnergy(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

/* -------------------------- private methods ------------------------------*/

std::expected<void, ModbusError> Inverter::detectFloatOrIntRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        ENOTCONN, "detectFloatOrIntRegisters(): Modbus context is null")));
  }

  int rc = modbus_read_registers(ctx_, I10X::ID.ADDR, 2,
                                 regs_.data() + I10X::ID.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "detectFloatOrIntRegisters(): Receive register failed {}",
        I10X::ID.describe())));
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
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "detectFloatOrIntRegisters(): Invalid meter ID: "
                            "received {}, expected [{}]",
                            inverterID, oss.str())));
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
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "detectFloatOrIntRegisters(): Invalid meter "
                            "register map size: received {}, expected [{}, {}]",
                            regMapSize, I10X::SIZE, I11X::SIZE)));
  }

  return {};
}

std::expected<void, ModbusError> Inverter::validateMultiMpptRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        ENOTCONN, "validateMultiMpptRegisters(): Modbus context is null")));
  }

  // Get registers
  const auto &idReg =
      useFloatRegisters_ ? I160::ID.withOffset(I160::FLOAT_OFFSET) : I160::ID;
  int rc =
      modbus_read_registers(ctx_, idReg.ADDR, 39, regs_.data() + idReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "validateSunSpecRegisters(): Receive register failed {}",
        idReg.describe())));
  }

  // --- Test for MPPT Register ID
  if (regs_[idReg.ADDR] != 160) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "validateMultiMpptRegisters(): Invalid multi MPPT "
                            "register map ID: received {}, expected 160",
                            regs_[idReg.ADDR])));
  }

  // --- Test for MPPT Register Map Length
  if (regs_[idReg.ADDR + idReg.NB] != I160::SIZE) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "validateMultiMpptRegisters(): Invalid multi MPPT "
                            "register map size: received {}, expected {}",
                            regs_[idReg.ADDR + idReg.NB], I160::SIZE)));
  }

  /* ---- Get number of inputs ---- */
  const auto &inputReg = useFloatRegisters_
                             ? I160::IDSTR_2.withOffset(I160::FLOAT_OFFSET)
                             : I160::IDSTR_2;

  auto inputStr = getModbusString(regs_, inputReg);
  if (!inputStr)
    return reportError<void>(std::unexpected(inputStr.error()));
  if (*inputStr == "String 2")
    inputs_ = 2;
  else
    inputs_ = 1;

  return {};
}

std::expected<void, ModbusError> Inverter::validateStorageRegisters() {
  if (!ctx_) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        ENOTCONN, "validateStorageRegisters(): Modbus context is null")));
  }

  hybrid_ = false;

  // Get registers
  const auto &idReg =
      useFloatRegisters_ ? I124::ID.withOffset(I124::FLOAT_OFFSET) : I124::ID;
  int rc =
      modbus_read_registers(ctx_, idReg.ADDR, 2, regs_.data() + idReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "validateStorageRegisters(): Receive register failed {}",
        idReg.describe())));
  }

  // --- Test for Basic Storage Control Register ID
  if (regs_[idReg.ADDR] == 0xFFFF)
    return {}; // not a hybrid inverter

  if (regs_[idReg.ADDR] != 124) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateStorageRegisters()(): Invalid basic storage control "
        "register map ID: received {}, expected 124",
        regs_[idReg.ADDR])));
  }

  // --- Test for Basic Storage Control Register Map Length
  if (regs_[idReg.ADDR + idReg.NB] != I124::SIZE) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateStorageRegisters()(): Invalid basic storage control "
        "register map size: received {}, expected {}",
        regs_[idReg.ADDR + idReg.NB], I124::SIZE)));
  }

  hybrid_ = true;

  return {};
}