#include "inverter.h"
#include "common_registers.h"
#include "fronius_bus.h"
#include "fronius_types.h"
#include "inverter_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <array>
#include <cerrno>
#include <expected>
#include <optional>
#include <sstream>
#include <vector>

/* -------------------------------------------------------------------------
   Construction
   ------------------------------------------------------------------------- */

Inverter::Inverter(std::shared_ptr<FroniusBus> bus,
                   const ModbusDeviceConfig &cfg)
    : FroniusDevice(cfg), bus_(std::move(bus)) {}

/* -------------------------------------------------------------------------
   FroniusDevice bus lifecycle
   ------------------------------------------------------------------------- */

void Inverter::onBusConnected() {
  // Reset all state from any previous connection cycle
  useFloatRegisters_ = false;
  id_ = 0;
  inputs_ = 0;
  hybrid_ = false;

  if (auto res = validateDevice(); !res) {
    setUnavailable();
    return;
  }

  setReady(FroniusTypes::RegisterMap::SUNSPEC);
}

void Inverter::onBusDisconnected() {
  useFloatRegisters_ = false;
  id_ = 0;
  inputs_ = 0;
  hybrid_ = false;
  setUnavailable();
}

/* -------------------------------------------------------------------------
   Transaction helper
   ------------------------------------------------------------------------- */

FroniusBus::Transaction Inverter::makeTransaction(int startAddr, int count,
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

/* -------------------------------------------------------------------------
   Data fetch
   ------------------------------------------------------------------------- */

std::expected<void, ModbusError> Inverter::fetchInverterRegisters() {
  // Active state code
  auto fState = bus_->submit(makeTransaction(
      F::ACTIVE_STATE_CODE.ADDR, F::ACTIVE_STATE_CODE.NB, regs_.data()));

  // Main inverter register block
  const auto &inverterBaseReg = useFloatRegisters_ ? I11X::A : I10X::A;
  const uint16_t inverterBlockSize =
      useFloatRegisters_ ? I11X::SIZE : I10X::SIZE;

  auto fInv = bus_->submit(
      makeTransaction(inverterBaseReg.ADDR, inverterBlockSize, regs_.data()));

  // Multi MPPT extension block
  const auto &multiMpptBaseReg =
      useFloatRegisters_ ? I160::DCA_SF.withOffset(I160::FLOAT_OFFSET)
                         : I160::DCA_SF;

  auto fMppt = bus_->submit(
      makeTransaction(multiMpptBaseReg.ADDR, I160::SIZE, regs_.data()));

  // Wait for all submitted transactions in order
  if (auto res = fState.get(); !res) {
    setUnavailable();
    return reportError<void>(std::unexpected(res.error()));
  }

  if (auto res = fInv.get(); !res) {
    setUnavailable();
    return reportError<void>(std::unexpected(res.error()));
  }

  if (auto res = fMppt.get(); !res) {
    setUnavailable();
    return reportError<void>(std::unexpected(res.error()));
  }

  return {};
}

/* -------------------------------------------------------------------------
   Electrical measurements
   ------------------------------------------------------------------------- */

std::expected<double, ModbusError>
Inverter::getAcPowerRating(FroniusTypes::Output output) const {
  switch (output) {
  case FroniusTypes::Output::ACTIVE:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I120::WRTG.withOffset(I120::FLOAT_OFFSET),
                                 I120::WRTG_SF.withOffset(I120::FLOAT_OFFSET))
               : getModbusDouble(regs_, I120::WRTG, I120::WRTG_SF);
  case FroniusTypes::Output::APPARENT:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I120::VARTG.withOffset(I120::FLOAT_OFFSET),
                                 I120::VARTG_SF.withOffset(I120::FLOAT_OFFSET))
               : getModbusDouble(regs_, I120::VARTG, I120::VARTG_SF);
  case FroniusTypes::Output::Q1_REACTIVE:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I120::VARRTGQ1.withOffset(I120::FLOAT_OFFSET),
                                 I120::VARRTG_SF.withOffset(I120::FLOAT_OFFSET))
               : getModbusDouble(regs_, I120::VARRTGQ1, I120::VARRTG_SF);
  case FroniusTypes::Output::Q4_REACTIVE:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I120::VARRTGQ4.withOffset(I120::FLOAT_OFFSET),
                                 I120::VARRTG_SF.withOffset(I120::FLOAT_OFFSET))
               : getModbusDouble(regs_, I120::VARRTGQ4, I120::VARRTG_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPowerRating(): Invalid output {}",
                            FroniusTypes::toString(output))));
  }
}

std::expected<double, ModbusError>
Inverter::getAcCurrent(FroniusTypes::Phase ph) const {
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
Inverter::getAcVoltage(FroniusTypes::Phase ph) const {
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
  case FroniusTypes::Phase::AB:
    return useFloatRegisters_
               ? getModbusDouble(regs_, I11X::PPVPHAB)
               : getModbusDouble(regs_, I10X::PPVPHAB, I10X::V_SF);
  case FroniusTypes::Phase::BC:
    return useFloatRegisters_
               ? getModbusDouble(regs_, I11X::PPVPHBC)
               : getModbusDouble(regs_, I10X::PPVPHBC, I10X::V_SF);
  case FroniusTypes::Phase::CA:
    return useFloatRegisters_
               ? getModbusDouble(regs_, I11X::PPVPHCA)
               : getModbusDouble(regs_, I10X::PPVPHCA, I10X::V_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcVoltage(): Invalid phase {}",
                            FroniusTypes::toString(ph))));
  }
}

std::expected<double, ModbusError>
Inverter::getAcPower(FroniusTypes::Output output) const {
  switch (output) {
  case FroniusTypes::Output::ACTIVE:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::W)
                              : getModbusDouble(regs_, I10X::W, I10X::W_SF);
  case FroniusTypes::Output::APPARENT:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::VA)
                              : getModbusDouble(regs_, I10X::VA, I10X::VA_SF);
  case FroniusTypes::Output::REACTIVE:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::VAR)
                              : getModbusDouble(regs_, I10X::VAR, I10X::VAR_SF);
  case FroniusTypes::Output::FACTOR:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::PF)
                              : getModbusDouble(regs_, I10X::PF, I10X::PF_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getAcPower(): Invalid output {}",
                            FroniusTypes::toString(output))));
  }
}

std::expected<double, ModbusError> Inverter::getAcFrequency() const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::FREQ)
                            : getModbusDouble(regs_, I10X::FREQ, I10X::FREQ_SF);
}

std::expected<double, ModbusError> Inverter::getAcEnergy() const {
  return useFloatRegisters_ ? getModbusDouble(regs_, I11X::WH)
                            : getModbusDouble(regs_, I10X::WH, I10X::WH_SF);
}

std::expected<double, ModbusError>
Inverter::getDcCurrent(FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::DCA)
                              : getModbusDouble(regs_, I10X::DCA, I10X::DCA_SF);
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCA_1.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCA_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCA_1, I160::DCA_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCA_2.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCA_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCA_2, I160::DCA_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcCurrent(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

std::expected<double, ModbusError>
Inverter::getDcVoltage(FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::DCV)
                              : getModbusDouble(regs_, I10X::DCV, I10X::DCV_SF);
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCV_1.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCV_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCV_1, I160::DCV_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCV_2.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCV_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCV_2, I160::DCV_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcVoltage(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

std::expected<double, ModbusError>
Inverter::getDcPower(FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::TOTAL:
    return useFloatRegisters_ ? getModbusDouble(regs_, I11X::DCW)
                              : getModbusDouble(regs_, I10X::DCW, I10X::DCW_SF);
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCW_1.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCW_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCW_1, I160::DCW_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCW_2.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCW_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCW_2, I160::DCW_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcPower(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

std::expected<double, ModbusError>
Inverter::getDcEnergy(FroniusTypes::Input input) const {
  switch (input) {
  case FroniusTypes::Input::A:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCWH_1.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCWH_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCWH_1, I160::DCWH_SF);
  case FroniusTypes::Input::B:
    return useFloatRegisters_
               ? getModbusDouble(regs_,
                                 I160::DCWH_2.withOffset(I160::FLOAT_OFFSET),
                                 I160::DCWH_SF.withOffset(I160::FLOAT_OFFSET))
               : getModbusDouble(regs_, I160::DCWH_2, I160::DCWH_SF);
  default:
    return reportError<double>(std::unexpected(
        ModbusError::custom(EINVAL, "getDcEnergy(): Invalid input {}",
                            FroniusTypes::toString(input))));
  }
}

int Inverter::getActiveStateCode() const {
  return static_cast<int>(regs_[F::ACTIVE_STATE_CODE.ADDR]);
}

std::expected<std::string, ModbusError> Inverter::getState() const {
  const auto reg = useFloatRegisters_ ? I11X::STVND : I10X::STVND;
  uint16_t statusRaw = regs_[reg.ADDR];

  auto strOpt =
      FroniusTypes::toString(static_cast<FroniusTypes::State>(statusRaw));

  if (!strOpt)
    return reportError<std::string>(std::unexpected(ModbusError::custom(
        EINVAL, "getState(): Unknown inverter state code {}", statusRaw)));

  return std::string(*strOpt);
}

std::expected<std::vector<std::string>, ModbusError>
Inverter::getEvents() const {
  std::vector<std::string> events;

  const uint16_t evtAddrs[3] = {
      useFloatRegisters_ ? I11X::EVTVND1.ADDR : I10X::EVTVND1.ADDR,
      useFloatRegisters_ ? I11X::EVTVND2.ADDR : I10X::EVTVND2.ADDR,
      useFloatRegisters_ ? I11X::EVTVND3.ADDR : I10X::EVTVND3.ADDR};

  for (int group = 0; group < 3; ++group) {
    uint32_t raw =
        ModbusUtils::modbus_get_uint32(regs_.data() + evtAddrs[group]);
    uint32_t unknownBits = raw;

    for (uint32_t bit = 0; bit < 32; ++bit) {
      uint32_t mask = 1u << bit;
      if (!(raw & mask))
        continue;

      std::optional<const char *> strOpt;
      switch (group) {
      case 0:
        strOpt =
            FroniusTypes::toString(static_cast<FroniusTypes::Event_1>(mask));
        break;
      case 1:
        strOpt =
            FroniusTypes::toString(static_cast<FroniusTypes::Event_2>(mask));
        break;
      case 2:
        strOpt =
            FroniusTypes::toString(static_cast<FroniusTypes::Event_3>(mask));
        break;
      }

      if (strOpt) {
        events.emplace_back(*strOpt);
        unknownBits &= ~mask;
      }
    }

    if (unknownBits != 0)
      return reportError<std::vector<std::string>>(
          std::unexpected(ModbusError::custom(
              EINVAL, "getEvents(): Unknown event group {} bits: 0x{:08X}",
              group + 1, unknownBits)));
  }

  return events;
}

/* -------------------------------------------------------------------------
   Private — device validation
   ------------------------------------------------------------------------- */

std::expected<void, ModbusError> Inverter::validateDevice() {
  // --- Step 1: validate common register block ---
  if (auto res = validateCommonRegisters(); !res)
    return res;

  // --- Step 2: detect float vs. integer model ---
  if (auto res = validateInverterRegisters(); !res)
    return res;

  // --- Step 3: validate multi MPPT block and detect input count ---
  if (auto res = validateMultiMpptRegisters(); !res)
    return res;

  // --- Step 4: validate storage control block (sets hybrid_) ---
  if (auto res = validateStorageRegisters(); !res)
    return res;

  // --- Step 5: validate and cache nameplate block ---
  if (auto res = validateNameplateRegisters(); !res)
    return res;

  // --- Step 6: validate and end register block ---
  if (auto res = validateEndRegisters(); !res)
    return res;

  return {};
}

std::expected<void, ModbusError> Inverter::validateCommonRegisters() {

  // read SID, ID, length and all common registers in one transaction
  auto fSig = bus_->submit(makeTransaction(
      C001::SID.ADDR, C001::SID.NB + C001::ID.NB + C001::L.NB + C001::SIZE,
      regs_.data()));

  if (auto res = fSig.get(); !res)
    return reportError<void>(std::unexpected(res.error()));

  if (!(regs_[C001::SID.ADDR] == 0x5375 && regs_[C001::SID.ADDR + 1] == 0x6e53))
    return reportError<void>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "validateDevice(): SunSpec signature mismatch: "
                            "expected [0x5375, 0x6e53], received [0x{}, 0x{}]",
                            ModbusUtils::toHex(regs_[C001::SID.ADDR]),
                            ModbusUtils::toHex(regs_[C001::SID.ADDR + 1]))));

  if (regs_[C001::ID.ADDR] != 0x1)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateDevice(): Invalid common block ID: received {}, expected 1",
        regs_[C001::ID.ADDR])));

  if (regs_[C001::L.ADDR] != C001::SIZE)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateDevice(): Invalid common block size: received {}, expected {}",
        regs_[C001::L.ADDR], C001::SIZE)));

  return {};
}

std::expected<void, ModbusError> Inverter::validateInverterRegisters() {

  // Read ID + size in one transaction
  auto fInt = bus_->submit(
      makeTransaction(I10X::ID.ADDR, I10X::ID.NB + I10X::L.NB, regs_.data()));

  if (auto res = fInt.get(); !res)
    return reportError<void>(std::unexpected(res.error()));

  uint16_t inverterID = regs_[I10X::ID.ADDR];

  static constexpr std::array<uint16_t, 6> validIDs = {101, 102, 103,
                                                       111, 112, 113};

  if (std::find(validIDs.begin(), validIDs.end(), inverterID) ==
      validIDs.end()) {
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
                            "validateInverterRegisters(): Invalid inverter ID: "
                            "received {}, expected [{}]",
                            inverterID, oss.str())));
  }

  id_ = inverterID;
  useFloatRegisters_ = (inverterID / 10 % 10) != 0;

  uint16_t regMapSize = regs_[I10X::L.ADDR];
  if (regMapSize != I10X::SIZE && regMapSize != I11X::SIZE)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateInverterRegisters(): Invalid register map size: "
        "received {}, expected [{}, {}]",
        regMapSize, I10X::SIZE, I11X::SIZE)));

  return {};
}

std::expected<void, ModbusError> Inverter::validateMultiMpptRegisters() {
  const auto idReg =
      useFloatRegisters_ ? I160::ID.withOffset(I160::FLOAT_OFFSET) : I160::ID;

  // Read ID + size + all MPPT extension registers in one transaction
  auto fMulti = bus_->submit(makeTransaction(
      idReg.ADDR, I160::ID.NB + I160::L.NB + I160::SIZE, regs_.data()));

  if (auto res = fMulti.get(); !res)
    return reportError<void>(std::unexpected(res.error()));

  if (regs_[idReg.ADDR] != 160)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateMultiMpptRegisters(): Invalid multi MPPT map ID: "
        "received {}, expected 160",
        regs_[idReg.ADDR])));

  if (regs_[idReg.ADDR + idReg.NB] != I160::SIZE)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateMultiMpptRegisters(): Invalid multi MPPT map size: "
        "received {}, expected {}",
        regs_[idReg.ADDR + idReg.NB], I160::SIZE)));

  // Determine number of inputs from the second input string name
  const auto inputReg = useFloatRegisters_
                            ? I160::IDSTR_2.withOffset(I160::FLOAT_OFFSET)
                            : I160::IDSTR_2;

  auto inputStr = getModbusString(regs_, inputReg);
  if (!inputStr)
    return reportError<void>(std::unexpected(inputStr.error()));

  inputs_ = (*inputStr == "String 2") ? 2 : 1;

  return {};
}

std::expected<void, ModbusError> Inverter::validateStorageRegisters() {
  hybrid_ = false;

  const auto idReg =
      useFloatRegisters_ ? I124::ID.withOffset(I124::FLOAT_OFFSET) : I124::ID;

  auto fStorage = bus_->submit(
      makeTransaction(idReg.ADDR, I124::ID.NB + I124::L.NB, regs_.data()));

  if (auto res = fStorage.get(); !res)
    return reportError<void>(std::unexpected(res.error()));

  // End-block marker (0xFFFF) means no storage block — not a hybrid inverter
  if (regs_[idReg.ADDR] == 0xFFFF)
    return {};

  if (regs_[idReg.ADDR] != 124)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateStorageRegisters(): Invalid storage block ID: "
        "received {}, expected 124",
        regs_[idReg.ADDR])));

  if (regs_[idReg.ADDR + I124::ID.NB] != I124::SIZE)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateStorageRegisters(): Invalid storage block size: "
        "received {}, expected {}",
        regs_[idReg.ADDR + I124::ID.NB], I124::SIZE)));

  hybrid_ = true;

  return {};
}

std::expected<void, ModbusError> Inverter::validateNameplateRegisters() {
  const auto idReg =
      useFloatRegisters_ ? I120::ID.withOffset(I120::FLOAT_OFFSET) : I120::ID;

  auto fName = bus_->submit(
      makeTransaction(idReg.ADDR, I120::ID.NB + I120::L.NB, regs_.data()));

  if (auto res = fName.get(); !res)
    return reportError<void>(std::unexpected(res.error()));

  if (regs_[idReg.ADDR] != 120)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateNameplateRegisters(): Invalid nameplate block ID: "
        "received {}, expected 120",
        regs_[idReg.ADDR])));

  if (regs_[idReg.ADDR + idReg.NB] != I120::SIZE)
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "validateNameplateRegisters(): Invalid nameplate block size: "
        "received {}, expected {}",
        regs_[idReg.ADDR + idReg.NB], I120::SIZE)));

  return {};
}

std::expected<void, ModbusError> Inverter::validateEndRegisters() {

  // End block — address depends on register model and hybrid flag
  auto endBlockBaseReg = I_END::ID;
  if (useFloatRegisters_)
    endBlockBaseReg = endBlockBaseReg.withOffset(I_END::FLOAT_OFFSET);
  if (hybrid_)
    endBlockBaseReg = endBlockBaseReg.withOffset(I_END::STORAGE_OFFSET);

  auto endBlockLengthReg = I_END::L;
  if (useFloatRegisters_)
    endBlockLengthReg = endBlockLengthReg.withOffset(I_END::FLOAT_OFFSET);
  if (hybrid_)
    endBlockLengthReg = endBlockLengthReg.withOffset(I_END::STORAGE_OFFSET);

  auto fEnd = bus_->submit(makeTransaction(
      endBlockBaseReg.ADDR, I_END::ID.NB + I_END::L.NB, regs_.data()));

  if (auto res = fEnd.get(); !res) {
    setUnavailable();
    return reportError<void>(std::unexpected(res.error()));
  }

  // Validate end block content
  if (!(regs_[endBlockBaseReg.ADDR] == 0xFFFF &&
        regs_[endBlockLengthReg.ADDR] == 0)) {
    return reportError<void>(std::unexpected(ModbusError::custom(
        EINVAL,
        "fetchInverterRegisters(): Invalid end block register: "
        "received [0x{}, {}], expected [0xFFFF, 0]",
        ModbusUtils::toHex(regs_[endBlockBaseReg.ADDR]),
        regs_[endBlockLengthReg.ADDR])));
  }

  return {};
}