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
#include <modbus_error.h>
#include <sstream>
#include <stdexcept>

Meter::Meter() : isInitialized_(false), useFloatRegisters_(false), phases_(1) {}

Meter::~Meter() {}

void Meter::checkInitialized() const {
  if (!isInitialized_) {
    throw std::logic_error(
        "Meter not initialized: call detectAndInitializeMeter() first");
  }
}

std::expected<int, ModbusError> Meter::detectAndInitializeMeter() {
  std::fill(regs_.begin(), regs_.end(), 0);

  int rc = modbus_read_registers(ctx_, M20X_ID::ADDR, 2,
                                 regs_.data() + M20X_ID::ADDR);
  if (rc == -1) {
    return std::unexpected(ModbusError::fromErrno(
        "Receive register " + std::to_string(M20X_ID::ADDR) + " failed"));
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
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter ID: received " + std::to_string(meterID) +
                    ", expected [" + oss.str() + "]"));
  }

  if (meterID / 10 % 10)
    useFloatRegisters_ = true;
  else
    useFloatRegisters_ = false;

  // Validate the register length
  uint16_t regMapSize = regs_[M20X_L::ADDR];
  if (regMapSize != M20X_SIZE && regMapSize != M21X_SIZE) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register map size: received " +
                    std::to_string(regMapSize) + ", expected [" +
                    std::to_string(M20X_SIZE) + ", " +
                    std::to_string(M21X_SIZE) + "]"));
  }

  // Store number of phases
  phases_ = meterID % 10;

  isInitialized_ = true;
  return meterID;
}

bool Meter::getUseFloatRegisters(void) const {
  checkInitialized();
  return useFloatRegisters_;
}

int Meter::getPhases(void) const {
  checkInitialized();
  return phases_;
};

std::expected<void, ModbusError> Meter::fetchMeterRegisters(void) {
  checkInitialized();

  // Get the meter registers
  uint16_t meterBlockAddr = (useFloatRegisters_) ? M21X_A::ADDR : M20X_A::ADDR;
  uint16_t meterBlockSize = (useFloatRegisters_) ? M21X_SIZE : M20X_SIZE;

  int rc = modbus_read_registers(ctx_, meterBlockAddr, meterBlockSize,
                                 regs_.data() + meterBlockAddr);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(meterBlockAddr) + " failed"));
  }

  // Validate the end block
  uint16_t endBlockAddr = (useFloatRegisters_) ? E21X_ID::ADDR : E20X_ID::ADDR;
  rc =
      modbus_read_registers(ctx_, endBlockAddr, 2, regs_.data() + endBlockAddr);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(endBlockAddr) + " failed"));
  }

  uint16_t endBlockLength = (useFloatRegisters_) ? E21X_L::ADDR : E20X_L::ADDR;
  if (!(regs_[endBlockAddr] == 0xFFFF && regs_[endBlockLength] == 0)) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register end block: received [0x" +
                    modbus_utils::to_hex(regs_[endBlockAddr]) + ", " +
                    std::to_string(regs_[endBlockLength]) +
                    "], expected [0xFFFF, 0]"));
  }

  return {};
}

double Meter::getAcPower(void) {
  double res;
  if (useFloatRegisters_)
    res = modbus_get_float_abcd(regs_.data() + M21X_W::ADDR);
  else
    res = static_cast<double>(regs_[M20X_W::ADDR]) *
          std::pow(10.0, static_cast<int16_t>(regs_[M20X_W_SF::ADDR]));

  return res;
}

double Meter::getAcEnergyImport(void) {
  double res;
  if (useFloatRegisters_)
    res = modbus_get_float_abcd(regs_.data() + M21X_TOTWH_IMP::ADDR);
  else
    res = static_cast<double>(modbus_utils::modbus_get_uint32(
              regs_.data() + M20X_TOTWH_IMP::ADDR)) *
          std::pow(10.0, static_cast<int16_t>(regs_[M20X_TOTWH_SF::ADDR]));

  return res * 1e-3; // Wh to kWh
}