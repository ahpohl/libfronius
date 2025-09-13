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

Meter::Meter() : useFloatRegisters_(true) {}

Meter::~Meter() {}

std::expected<bool, ModbusError> Meter::isSunSpecMeter() {
  auto result = isSunSpecDevice();
  if (!result) {
    return std::unexpected(result.error());
  }

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

  // Validate register map length
  uint16_t regMapSize = regs_[M20X_L::ADDR];
  if (regMapSize != M20X_SIZE && regMapSize != M21X_SIZE) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid meter register map size: received " +
                    std::to_string(regMapSize) + ", expected [" +
                    std::to_string(M20X_SIZE) + ", " +
                    std::to_string(M21X_SIZE) + "]"));
  }

  if (regMapSize == M20X_SIZE) {
    useFloatRegisters_ = false; // Set register model integer + scale factor

    rc = modbus_read_registers(ctx_, E20X_ID::ADDR, 2,
                               regs_.data() + E20X_ID::ADDR);
    if (rc == -1) {
      return std::unexpected(ModbusError::fromErrno(
          "Receive register " + std::to_string(E20X_ID::ADDR) + " failed"));
    }

    // Validate the end block
    uint16_t endBlockID = regs_[E20X_ID::ADDR];
    uint16_t endBlockLength = regs_[E20X_L::ADDR];
    if (!(endBlockID == 0xFFFF && endBlockLength == 0)) {
      return std::unexpected(ModbusError::custom(
          EINVAL, "Invalid meter register end block: received [0x" +
                      modbus_utils::to_hex(endBlockID) + ", " +
                      std::to_string(endBlockLength) +
                      "], expected [0xFFFF, 0]"));
    }

  } else if (regMapSize == M21X_SIZE) {
    useFloatRegisters_ = true; // Set register model float

    rc = modbus_read_registers(ctx_, E21X_ID::ADDR, 2,
                               regs_.data() + E21X_ID::ADDR);
    if (rc == -1) {
      return std::unexpected(ModbusError::fromErrno(
          "Receive register " + std::to_string(E21X_ID::ADDR) + " failed"));
    }

    // Validate the end block
    uint16_t endBlockID = regs_[E21X_ID::ADDR];
    uint16_t endBlockLength = regs_[E21X_L::ADDR];
    if (!(endBlockID == 0xFFFF && endBlockLength == 0)) {
      return std::unexpected(ModbusError::custom(
          EINVAL, "Invalid meter register end block: received [0x" +
                      modbus_utils::to_hex(endBlockID) + ", " +
                      std::to_string(endBlockLength) +
                      "], expected [0xFFFF, 0]"));
    }
  }

  return {};
}

bool Meter::getFloatRegisterModel(void) const { return useFloatRegisters_; }

std::expected<void, ModbusError> Meter::readMeterRegisters(void) {
  int rc;
  if (useFloatRegisters_) {
    rc = modbus_read_registers(ctx_, M21X_A::ADDR, M21X_SIZE,
                               regs_.data() + M21X_A::ADDR);
    if (rc == -1) {
      return std::unexpected(
          ModbusError::fromErrno(std::string("Receive register ") +
                                 std::to_string(M21X_A::ADDR) + " failed: "));
    }
  } else {
    rc = modbus_read_registers(ctx_, M20X_A::ADDR, M20X_SIZE,
                               regs_.data() + M20X_A::ADDR);
    if (rc == -1) {
      return std::unexpected(
          ModbusError::fromErrno(std::string("Receive register ") +
                                 std::to_string(M20X_A::ADDR) + " failed: "));
    }
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