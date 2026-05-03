#include "meter.h"
#include "common_registers.h"
#include "fronius_bus.h"
#include "fronius_types.h"
#include "meter_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <array>
#include <cerrno>
#include <expected>
#include <format>
#include <sstream>

/* -------------------------------------------------------------------------
   Construction
   ------------------------------------------------------------------------- */

Meter::Meter(std::shared_ptr<FroniusBus> bus, const ModbusDeviceConfig &cfg)
    : FroniusDevice(cfg), bus_(std::move(bus)) {}

/* -------------------------------------------------------------------------
   FroniusDevice bus lifecycle
   ------------------------------------------------------------------------- */

void Meter::onBusConnected() {
  // Reset state from any previous connection cycle before probing
  useFloatRegisters_ = false;
  id_ = 0;

  auto result = validateDevice();
  if (!result) {
    setUnavailable();
    return;
  }

  setReady(*result);
}

void Meter::onBusDisconnected() {
  useFloatRegisters_ = false;
  id_ = 0;
  setUnavailable();
}

/* -------------------------------------------------------------------------
   Data fetch
   ------------------------------------------------------------------------- */

FroniusBus::Transaction Meter::makeTransaction(int startAddr, int count,
                                               uint16_t *dest) {
  FroniusBus::Transaction t;
  t.slaveId = cfg_.slaveId;
  t.startAddr = startAddr;
  t.count = count;
  t.dest = dest;
  t.secTimeout = cfg_.secTimeout;
  t.usecTimeout = cfg_.usecTimeout;
  return t;
}

std::expected<void, ModbusError> Meter::fetchMeterRegisters() {

  // --- Proprietary path ---

  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    // Submit all three register blocks concurrently — they will be executed
    // sequentially by the bus thread, but submission is non-blocking so all
    // three are in the queue before we start waiting.
    auto fSum = bus_->submit(makeTransaction(REG::PHV.ADDR, 16, regs_.data()));

    auto fPhase =
        bus_->submit(makeTransaction(REG::PPVPHAB.ADDR, 42, regs_.data()));

    auto fEnergy =
        bus_->submit(makeTransaction(REG::TOT_KWH_IMP.ADDR, 16, regs_.data()));

    // Now wait for all three in submission order
    if (auto res = fSum.get(); !res) {
      setUnavailable();
      return reportError<void>(std::unexpected(res.error()));
    }
    if (auto res = fPhase.get(); !res) {
      setUnavailable();
      return reportError<void>(std::unexpected(res.error()));
    }
    if (auto res = fEnergy.get(); !res) {
      setUnavailable();
      return reportError<void>(std::unexpected(res.error()));
    }

    return {};
  }

  // --- SunSpec path ---

  else if (registerMap_ == FroniusTypes::RegisterMap::SUNSPEC) {
    const auto &meterBaseReg = useFloatRegisters_ ? M21X::A : M20X::A;
    const uint16_t meterBlockSize =
        useFloatRegisters_ ? M21X::SIZE : M20X::SIZE;

    auto fMeter = bus_->submit(
        makeTransaction(meterBaseReg.ADDR, meterBlockSize, regs_.data()));

    if (auto res = fMeter.get(); !res) {
      setUnavailable();
      return reportError<void>(std::unexpected(res.error()));
    }
  }

  return {};
}

/* -------------------------------------------------------------------------
   Device identity accessors
   ------------------------------------------------------------------------- */

std::expected<uint16_t, ModbusError> Meter::getModbusDeviceAddress() {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return static_cast<uint16_t>(cfg_.slaveId);

  // SunSpec: read from common block DA register already in regs_
  uint16_t val = regs_[C001::DA.ADDR];
  if (val < 1 || val > 247)
    return reportError<uint16_t>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "getModbusDeviceAddress(): Invalid slave address: "
                            "received {}, expected 1-247",
                            val)));
  return val;
}

std::expected<std::string, ModbusError> Meter::getSerialNumber() {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    auto f =
        bus_->submit(makeTransaction(REG::SN.ADDR, REG::SN.NB, regs_.data()));
    if (auto res = f.get(); !res)
      return reportError<std::string>(std::unexpected(res.error()));

    uint32_t serial =
        ModbusUtils::modbus_get_uint32(regs_.data() + REG::SN.ADDR);
    return std::to_string(serial);
  }
  return getModbusString(regs_, C001::SN);
}

std::expected<std::string, ModbusError> Meter::getManufacturer() {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return std::string("Fronius");
  return getModbusString(regs_, C001::MN);
}

std::expected<std::string, ModbusError> Meter::getDeviceModel() {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return std::string("Smart Meter TS 65A-3");
  return getModbusString(regs_, C001::MD);
}

std::expected<std::string, ModbusError> Meter::getFwVersion() {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY) {
    // VR_MAJOR and VR_MINOR are two consecutive UINT16 registers. Read
    // them in a single transaction starting at VR_MAJOR for efficiency.
    auto f = bus_->submit(makeTransaction(REG::VR_MAJOR.ADDR,
                                          REG::VR_MAJOR.NB + REG::VR_MINOR.NB,
                                          regs_.data()));
    if (auto res = f.get(); !res)
      return reportError<std::string>(std::unexpected(res.error()));

    return std::format("{}.{}", regs_[REG::VR_MAJOR.ADDR],
                       regs_[REG::VR_MINOR.ADDR]);
  }
  return getModbusString(regs_, C001::VR);
}

/* -------------------------------------------------------------------------
   Electrical measurements
   ------------------------------------------------------------------------- */

std::expected<double, ModbusError>
Meter::getAcCurrent(FroniusTypes::Phase ph) const {
  switch (ph) {
  case FroniusTypes::Phase::TOTAL:
    if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
      return reportError<double>(std::unexpected(ModbusError::custom(
          ENOTSUP, "getAcCurrent(): Phase::TOTAL not supported for "
                   "proprietary register map")));
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
Meter::getAcVoltage(FroniusTypes::Phase ph) const {
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

std::expected<double, ModbusError>
Meter::getAcPowerActive(FroniusTypes::Phase ph) const {
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

std::expected<double, ModbusError> Meter::getAcFrequency() const {
  return getRegValue(REG::FREQ, REG::FREQ_SF, M20X::FREQ, M20X::FREQ_SF,
                     M21X::FREQ);
}

std::expected<double, ModbusError>
Meter::getAcPowerApparent(FroniusTypes::Phase ph) const {
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
Meter::getAcPowerReactive(FroniusTypes::Phase ph) const {
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
Meter::getAcPowerFactor(FroniusTypes::Phase ph) const {
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
        ENOTSUP, "getAcEnergyApparent(): Not supported for proprietary "
                 "register map")));

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
        ENOTSUP, "getAcEnergyReactive(): Not supported for SunSpec "
                 "register map")));

  auto kwh = getModbusDouble(regs_, eregs.kwh, REG::TOT_SF);
  if (!kwh)
    return std::unexpected(kwh.error());
  auto wh = getModbusDouble(regs_, eregs.wh, REG::TOT_SF);
  if (!wh)
    return std::unexpected(wh.error());
  return *kwh * 1000.0 + *wh;
}

/* -------------------------------------------------------------------------
   Private — device validation
   ------------------------------------------------------------------------- */

std::expected<FroniusTypes::RegisterMap, ModbusError> Meter::validateDevice() {
  // --- Step 1: probe for the proprietary register map ---
  // Read the device type register. If it responds with device type 731 this
  // is a Smart Meter TS 65A-3 using the proprietary RTU map.
  // If the register address is illegal (EMBXILADD) the device does not have
  // this register and we fall through to SunSpec probing.
  auto fProp =
      bus_->submit(makeTransaction(REG::ID.ADDR, REG::ID.NB, regs_.data()));

  if (auto res = fProp.get(); !res) {
    if (res.error().code != EMBXILADD)
      return reportError<FroniusTypes::RegisterMap>(
          std::unexpected(res.error()));
    // EMBXILADD means address does not exist — not a proprietary device
  } else {
    if (regs_[REG::ID.ADDR] == 731)
      return FroniusTypes::RegisterMap::PROPRIETARY;
  }

  // --- Step 2: validate SunSpec signature ---
  // Read the SunSpec ID and common block header in one transaction.
  auto fSunSpec =
      bus_->submit(makeTransaction(C001::SID.ADDR, 4, regs_.data()));

  if (auto res = fSunSpec.get(); !res)
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(res.error()));

  // Verify "SunS" identifier
  if (!(regs_[C001::SID.ADDR] == 0x5375 && regs_[C001::SID.ADDR + 1] == 0x6e53))
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "validateDevice(): SunSpec signature mismatch: "
                            "expected [0x5375, 0x6e53], received [0x{}, 0x{}]",
                            ModbusUtils::toHex(regs_[C001::SID.ADDR]),
                            ModbusUtils::toHex(regs_[C001::SID.ADDR + 1]))));

  if (regs_[C001::ID.ADDR] != 0x1)
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "validateDevice(): Invalid common block ID: "
                            "received {}, expected 1",
                            regs_[C001::ID.ADDR])));

  if (regs_[C001::L.ADDR] != C001::SIZE)
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "validateDevice(): Invalid common block size: "
                            "received {}, expected {}",
                            regs_[C001::L.ADDR], C001::SIZE)));

  // --- Step 3: fetch the full common register block ---
  auto fCommon =
      bus_->submit(makeTransaction(C001::MN.ADDR, C001::SIZE, regs_.data()));

  if (auto res = fCommon.get(); !res)
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(res.error()));

  // --- Step 4: detect float vs. integer model ---
  if (auto res = detectFloatOrIntRegisters(); !res)
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(res.error()));

  // --- Step 5: validate the end register block ---
  if (auto res = validateEndRegisters(); !res)
    return reportError<FroniusTypes::RegisterMap>(std::unexpected(res.error()));

  return FroniusTypes::RegisterMap::SUNSPEC;
}

std::expected<void, ModbusError> Meter::detectFloatOrIntRegisters() {
  // Read the meter model ID and map length registers
  auto f = bus_->submit(makeTransaction(M20X::ID.ADDR, 2, regs_.data()));

  if (auto res = f.get(); !res)
    return reportError<void>(std::unexpected(res.error()));

  uint16_t meterID = regs_[M20X::ID.ADDR];

  static constexpr std::array<uint16_t, 6> validIDs = {201, 202, 203,
                                                       211, 212, 213};

  if (std::find(validIDs.begin(), validIDs.end(), meterID) == validIDs.end()) {
    std::ostringstream oss;
    bool first = true;
    for (auto id : validIDs) {
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

  id_ = meterID;
  useFloatRegisters_ = (meterID / 10 % 10) != 0;

  uint16_t regMapSize = regs_[M20X::L.ADDR];
  if (regMapSize != M20X::SIZE && regMapSize != M21X::SIZE)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "detectFloatOrIntRegisters(): Invalid register map size: "
        "received {}, expected [{}, {}]",
        regMapSize, M20X::SIZE, M21X::SIZE)));

  return {};
}

std::expected<void, ModbusError> Meter::validateEndRegisters() {

  // End block validation
  const auto endBlockBaseReg = useFloatRegisters_
                                   ? M_END::ID.withOffset(M_END::FLOAT_OFFSET)
                                   : M_END::ID;
  const auto endBlockLengthReg =
      useFloatRegisters_ ? M_END::L.withOffset(M_END::FLOAT_OFFSET) : M_END::L;

  auto fEnd =
      bus_->submit(makeTransaction(endBlockBaseReg.ADDR, 2, regs_.data()));

  if (auto res = fEnd.get(); !res) {
    setUnavailable();
    return reportError<void>(std::unexpected(res.error()));
  }

  if (!(regs_[endBlockBaseReg.ADDR] == 0xFFFF &&
        regs_[endBlockLengthReg.ADDR] == 0)) {
    setUnavailable();
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "fetchMeterRegisters(): Invalid end block register: "
        "received [0x{}, {}], expected [0xFFFF, 0]",
        ModbusUtils::toHex(regs_[endBlockBaseReg.ADDR]),
        regs_[endBlockLengthReg.ADDR])));
  }

  return {};
}

std::expected<double, ModbusError>
Meter::getRegValue(const Register &regProp, double sfProp,
                   const Register &regInt, const Register &sfInt,
                   const Register &regFlt) const {
  if (registerMap_ == FroniusTypes::RegisterMap::PROPRIETARY)
    return getModbusDouble(regs_, regProp, sfProp);

  if (registerMap_ == FroniusTypes::RegisterMap::SUNSPEC)
    return useFloatRegisters_ ? getModbusDouble(regs_, regFlt)
                              : getModbusDouble(regs_, regInt, sfInt);

  return reportError<double>(std::unexpected(ModbusError::custom(
      ENODATA, "getRegValue(): Register map not yet detected")));
}
