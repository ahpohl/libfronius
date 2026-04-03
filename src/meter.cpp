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

  // --- proprietary path ---
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    int rc = modbus_read_registers(ctx_, REG::PHV.ADDR, 16,
                                   regs_.data() + REG::PHV.ADDR);
    if (rc == -1) {
      return reportError<void>(std::unexpected(ModbusError::fromErrno(
          "fetchMeterRegisters(): reading proprietary summary block {} failed",
          REG::PHV.ADDR)));
    }

    rc = modbus_read_registers(ctx_, REG::PPVPHAB.ADDR, 42,
                               regs_.data() + REG::PPVPHAB.ADDR);
    if (rc == -1) {
      return reportError<void>(std::unexpected(ModbusError::fromErrno(
          "fetchMeterRegisters(): reading proprietary phase block {} failed",
          REG::PPVPHAB.ADDR)));
    }

    // Energy block: import/export kWh + Wh pairs
    rc = modbus_read_registers(ctx_, REG::TOT_KWH_IMP.ADDR, 16,
                               regs_.data() + REG::TOT_KWH_IMP.ADDR);
    if (rc == -1) {
      return reportError<void>(std::unexpected(ModbusError::fromErrno(
          "fetchMeterRegisters(): reading proprietary energy block {} failed",
          REG::TOT_KWH_IMP.ADDR)));
    }

    return {};
  }

  // --- SunSpec path ---

  // Validate the end block
  const auto endBlockBaseReg = useFloatRegisters_
                                   ? M_END::ID.withOffset(M_END::FLOAT_OFFSET)
                                   : M_END::ID;

  int rc = modbus_read_registers(ctx_, endBlockBaseReg.ADDR, 2,
                                 regs_.data() + endBlockBaseReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchMeterRegisters(): Receive end block register failed {}",
        endBlockBaseReg.describe())));
  }

  const auto &endBlockLengthReg = (useFloatRegisters_)
                                      ? M_END::L.withOffset(M_END::FLOAT_OFFSET)
                                      : M_END::L;

  if (!(regs_[endBlockBaseReg.ADDR] == 0xFFFF &&
        regs_[endBlockLengthReg.ADDR] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "fetchMeterRegisters(): Invalid end block register: received "
        "[0x{}, {}], expected [0xFFFF, 0]",
        ModbusUtils::toHex(regs_[endBlockBaseReg.ADDR]),
        endBlockLengthReg.ADDR)));
  }

  // Get the meter registers
  const auto &meterBaseReg = useFloatRegisters_ ? M21X::A : M20X::A;
  const uint16_t meterBlockSize =
      (useFloatRegisters_) ? M21X::SIZE : M20X::SIZE;

  rc = modbus_read_registers(ctx_, meterBaseReg.ADDR, meterBlockSize,
                             regs_.data() + meterBaseReg.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "fetchMeterRegisters(): Receive registers failed {}",
        meterBaseReg.describe())));
  }

  return {};
}

std::expected<FroniusTypes::RegisterMap, ModbusError> Meter::validateDevice() {
  registerMap_ = FroniusTypes::RegisterMap::UNAVAILABLE;

  // --- Step 1: Probe proprietary register map ---
  uint16_t buf[REG::ID.NB]{};
  int rc = modbus_read_registers(ctx_, REG::ID.ADDR, REG::ID.NB, buf);
  if (rc == -1) {
    if (errno != EMBXILADD) {
      return std::unexpected(ModbusError::fromErrno(
          "validateDevice(): reading proprietary device type register failed"));
    }
  } else if (buf[0] == 731) {
    registerMap_ = FroniusTypes::RegisterMap::PROPRIETARY;
    return registerMap_;
  }

  // --- Step 2: Check SunSpec signature ---
  auto sunspec = validateSunSpecRegisters();
  if (!sunspec)
    return std::unexpected(sunspec.error());

  // --- Step 3: Fetch common registers ---
  auto common = fetchCommonRegisters();
  if (!common)
    return std::unexpected(common.error());

  // --- Step 4: Detect float vs. integer register model ---
  auto init = detectFloatOrIntRegisters();
  if (!init)
    return std::unexpected(init.error());

  registerMap_ = FroniusTypes::RegisterMap::SUNSPEC;
  return registerMap_;
}

std::expected<uint16_t, ModbusError> Meter::getModbusDeviceAddress(void) {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    return static_cast<uint16_t>(cfg_.slaveId);
  }
  return Fronius::getModbusDeviceAddress();
}

std::expected<std::string, ModbusError> Meter::getSerialNumber(void) {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    uint16_t buf[REG::SN.NB]{};
    int rc = modbus_read_registers(ctx_, REG::SN.ADDR, REG::SN.NB, buf);
    if (rc == -1) {
      return std::unexpected(ModbusError::fromErrno(
          "getSerialNumber(): reading serial number register failed"));
    }
    uint32_t serial = ModbusUtils::modbus_get_uint32(buf);
    return std::to_string(serial);
  }
  return Fronius::getSerialNumber();
}

std::expected<std::string, ModbusError> Meter::getManufacturer(void) {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return std::string("Fronius");
  return Fronius::getManufacturer();
}

std::expected<std::string, ModbusError> Meter::getDeviceModel(void) {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return std::string("Smart Meter TS 65A-3");
  return Fronius::getDeviceModel();
}

std::expected<std::string, ModbusError> Meter::getFwVersion(void) {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    uint16_t buf[REG::VR.NB]{};
    int rc = modbus_read_registers(ctx_, REG::VR.ADDR, REG::VR.NB, buf);
    if (rc == -1) {
      return std::unexpected(ModbusError::fromErrno(
          "getFwVersion(): reading firmware version registers failed"));
    }
    return std::format("{}.{}", buf[0], buf[1]);
  }
  return Fronius::getFwVersion();
}

/* --------------------- get values -------------------------- */

std::expected<double, ModbusError>
Meter::getAcPowerActive(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return getRegValue(REG::W, REG::W_SF, M20X::W, M20X::W_SF, M21X::W);
  case FroniusTypes::Phase::A:
    return getRegValue(REG::WPHA, REG::W_SF, M20X::WPHA, M20X::W_SF,
                       M21X::WPHA);
  case FroniusTypes::Phase::B:
    return getRegValue(REG::WPHB, REG::W_SF, M20X::WPHB, M20X::W_SF,
                       M21X::WPHB);
  case FroniusTypes::Phase::C:
    return getRegValue(REG::WPHC, REG::W_SF, M20X::WPHC, M20X::W_SF,
                       M21X::WPHC);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPowerActive(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcPowerApparent(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return getRegValue(REG::VA, REG::VA_SF, M20X::VA, M20X::VA_SF, M21X::VA);
  case FroniusTypes::Phase::A:
    return getRegValue(REG::VAPHA, REG::VA_SF, M20X::VAPHA, M20X::VA_SF,
                       M21X::VAPHA);
  case FroniusTypes::Phase::B:
    return getRegValue(REG::VAPHB, REG::VA_SF, M20X::VAPHB, M20X::VA_SF,
                       M21X::VAPHB);
  case FroniusTypes::Phase::C:
    return getRegValue(REG::VAPHC, REG::VA_SF, M20X::VAPHC, M20X::VA_SF,
                       M21X::VAPHC);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPowerApparent(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcPowerReactive(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    return getRegValue(REG::VAR, REG::VAR_SF, M20X::VAR, M20X::VAR_SF,
                       M21X::VAR);
  case FroniusTypes::Phase::A:
    return getRegValue(REG::VARPHA, REG::VAR_SF, M20X::VARPHA, M20X::VAR_SF,
                       M21X::VARPHA);
  case FroniusTypes::Phase::B:
    return getRegValue(REG::VARPHB, REG::VAR_SF, M20X::VARPHB, M20X::VAR_SF,
                       M21X::VARPHB);
  case FroniusTypes::Phase::C:
    return getRegValue(REG::VARPHC, REG::VAR_SF, M20X::VARPHC, M20X::VAR_SF,
                       M21X::VARPHC);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPowerReactive(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcPowerFactor(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::AVERAGE:
    return getRegValue(REG::PF, REG::PF_SF, M20X::PF, M20X::PF_SF, M21X::PF);
  case FroniusTypes::Phase::A:
    return getRegValue(REG::PFPHA, REG::PF_SF, M20X::PFPHA, M20X::PF_SF,
                       M21X::PFPHA);
  case FroniusTypes::Phase::B:
    return getRegValue(REG::PFPHB, REG::PF_SF, M20X::PFPHB, M20X::PF_SF,
                       M21X::PFPHB);
  case FroniusTypes::Phase::C:
    return getRegValue(REG::PFPHC, REG::PF_SF, M20X::PFPHC, M20X::PF_SF,
                       M21X::PFPHC);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPowerFactor(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcCurrent(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
      return reportError<double>(std::unexpected(ModbusError::custom(
          ENOTSUP, "getAcCurrent(): Phase::TOTAL not supported for proprietary "
                   "register map")));
    return getRegValue(REG::A, REG::A_SF, M20X::A, M20X::A_SF, M21X::A);
  case FroniusTypes::Phase::A:
    return getRegValue(REG::APHA, REG::A_SF, M20X::APHA, M20X::A_SF,
                       M21X::APHA);
  case FroniusTypes::Phase::B:
    return getRegValue(REG::APHB, REG::A_SF, M20X::APHB, M20X::A_SF,
                       M21X::APHB);
  case FroniusTypes::Phase::C:
    return getRegValue(REG::APHC, REG::A_SF, M20X::APHC, M20X::A_SF,
                       M21X::APHC);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcCurrent(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Meter::getAcVoltage(const FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::PHV:
    return getRegValue(REG::PHV, REG::V_SF, M20X::PHV, M20X::V_SF, M21X::PHV);
  case FroniusTypes::Phase::A:
    return getRegValue(REG::PHVPHA, REG::V_SF, M20X::PHVPHA, M20X::V_SF,
                       M21X::PHVPHA);
  case FroniusTypes::Phase::B:
    return getRegValue(REG::PHVPHB, REG::V_SF, M20X::PHVPHB, M20X::V_SF,
                       M21X::PHVPHB);
  case FroniusTypes::Phase::C:
    return getRegValue(REG::PHVPHC, REG::V_SF, M20X::PHVPHC, M20X::V_SF,
                       M21X::PHVPHC);
  case FroniusTypes::Phase::PPV:
    return getRegValue(REG::PPV, REG::V_SF, M20X::PPV, M20X::V_SF, M21X::PPV);
  case FroniusTypes::Phase::AB:
    return getRegValue(REG::PPVPHAB, REG::V_SF, M20X::PPVPHAB, M20X::V_SF,
                       M21X::PPVPHAB);
  case FroniusTypes::Phase::BC:
    return getRegValue(REG::PPVPHBC, REG::V_SF, M20X::PPVPHBC, M20X::V_SF,
                       M21X::PPVPHBC);
  case FroniusTypes::Phase::CA:
    return getRegValue(REG::PPVPHCA, REG::V_SF, M20X::PPVPHCA, M20X::V_SF,
                       M21X::PPVPHCA);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcVoltage(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError> Meter::getAcFrequency(void) const {
  return getRegValue(REG::FREQ, REG::FREQ_SF, M20X::FREQ, M20X::FREQ_SF,
                     M21X::FREQ);
}

std::expected<double, ModbusError>
Meter::getAcEnergyActive(FroniusTypes::EnergyDirection direction) const {

  struct EnergyRegs {
    Register kwh, wh;
    Register m20x_wh, m20x_sf;
    Register m21x_wh;
  };

  EnergyRegs eregs;
  switch (direction) {
  case FroniusTypes::EnergyDirection::EXPORT:
    eregs = {REG::TOT_KWH_EXP, REG::TOT_WH_EXP, M20X::TOT_WH_EXP,
             M20X::TOT_WH_SF, M21X::TOT_WH_EXP};
    break;
  case FroniusTypes::EnergyDirection::IMPORT:
    eregs = {REG::TOT_KWH_IMP, REG::TOT_WH_IMP, M20X::TOT_WH_IMP,
             M20X::TOT_WH_SF, M21X::TOT_WH_IMP};
    break;
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcEnergyActive(): Invalid direction {}",
                            FroniusTypes::toString(direction))));
  }

  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    auto kwh = getModbusDouble(regs_, eregs.kwh, REG::TOT_SF);
    if (!kwh)
      return std::unexpected(kwh.error());
    auto wh = getModbusDouble(regs_, eregs.wh, REG::TOT_SF);
    if (!wh)
      return std::unexpected(wh.error());
    return *kwh * 1000.0 + *wh;
  }

  return useFloatRegisters_
             ? getModbusDouble(regs_, eregs.m21x_wh)
             : getModbusDouble(regs_, eregs.m20x_wh, eregs.m20x_sf);
}

std::expected<double, ModbusError>
Meter::getAcEnergyApparent(FroniusTypes::EnergyDirection direction) const {

  struct EnergyRegs {
    Register m20x_vah, m20x_sf;
    Register m21x_vah;
  };

  EnergyRegs eregs;
  switch (direction) {
  case FroniusTypes::EnergyDirection::EXPORT:
    eregs = {M20X::TOT_VAH_EXP, M20X::TOT_VAH_SF, M21X::TOT_VAH_EXP};
    break;
  case FroniusTypes::EnergyDirection::IMPORT:
    eregs = {M20X::TOT_VAH_IMP, M20X::TOT_VAH_SF, M21X::TOT_VAH_IMP};
    break;
  default:
    return reportError<double>(std::unexpected(ModbusError::custom(
        EINVAL, "getAcEnergyApparent(): Invalid direction {}",
        FroniusTypes::toString(direction))));
  }

  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return reportError<double>(std::unexpected(ModbusError::custom(
        ENOTSUP,
        "getAcEnergyApparent(): Not supported for proprietary register map")));

  return useFloatRegisters_
             ? getModbusDouble(regs_, eregs.m21x_vah)
             : getModbusDouble(regs_, eregs.m20x_vah, eregs.m20x_sf);
}

std::expected<double, ModbusError>
Meter::getAcEnergyReactive(FroniusTypes::EnergyDirection direction) const {

  struct EnergyRegs {
    Register kwh, wh;
  };

  EnergyRegs eregs;
  switch (direction) {
  case FroniusTypes::EnergyDirection::EXPORT:
    eregs = {REG::TOT_KVARH_EXP, REG::TOT_VARH_EXP};
    break;
  case FroniusTypes::EnergyDirection::IMPORT:
    eregs = {REG::TOT_KVARH_IMP, REG::TOT_VARH_IMP};
    break;
  default:
    return reportError<double>(std::unexpected(ModbusError::custom(
        EINVAL, "getAcEnergyReactive(): Invalid direction {}",
        FroniusTypes::toString(direction))));
  }

  if (registerMap_ == FroniusTypes::RegisterMap::SUNSPEC)
    return reportError<double>(std::unexpected(ModbusError::custom(
        ENOTSUP,
        "getAcEnergyReactive(): Not supported for sunspec register map")));

  auto kwh = getModbusDouble(regs_, eregs.kwh, REG::TOT_SF);
  if (!kwh)
    return std::unexpected(kwh.error());
  auto wh = getModbusDouble(regs_, eregs.wh, REG::TOT_SF);
  if (!wh)
    return std::unexpected(wh.error());
  return *kwh * 1000.0 + *wh;
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

std::expected<double, ModbusError>
Meter::getRegValue(const Register &regProp, double sfProp,
                   const Register &regInt, const Register &sfInt,
                   const Register &regFlt) const {

  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    return getModbusDouble(regs_, regProp, sfProp);
  }
  if (registerMap_ == FroniusTypes::RegisterMap::SUNSPEC) {
    return useFloatRegisters_ ? getModbusDouble(regs_, regFlt)
                              : getModbusDouble(regs_, regInt, sfInt);
  }

  return reportError<double>(std::unexpected(ModbusError::custom(
      ENODATA, "getRegValue(): register map not yet detected")));
}
