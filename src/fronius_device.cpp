#include "fronius_device.h"
#include "common_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include "register_base.h"
#include <cmath>
#include <expected>
#include <modbus.h>
#include <optional>
#include <string>
#include <vector>

// --------------------------------------------------------------------------
// Construction
// --------------------------------------------------------------------------

FroniusDevice::FroniusDevice(const ModbusDeviceConfig &cfg) : cfg_(cfg) {
  cfg.validate();

  // Pre-zeroed to cover the full 16-bit Modbus address space; see `regs_`.
  regs_.resize(0xFFFF, 0);
}

// -------------------------------------------------------------------------
// Ready / unavailable state helpers
// -------------------------------------------------------------------------

void FroniusDevice::setReady(FroniusTypes::RegisterMap map) {
  registerMap_ = map;
  ready_.store(true);

  if (onDeviceReady_)
    onDeviceReady_(map);
}

void FroniusDevice::setUnavailable() {
  ready_.store(false);
  registerMap_ = FroniusTypes::RegisterMap::UNAVAILABLE;

  if (onDeviceUnavailable_)
    onDeviceUnavailable_();
}

// -------------------------------------------------------------------------
// Register decoding helpers
// -------------------------------------------------------------------------

std::expected<std::string, ModbusError>
FroniusDevice::getModbusString(const std::vector<uint16_t> &regs,
                               const Register &reg) const {
  std::string str;

  try {
    if (reg.TYPE != Register::Type::STRING) {
      throw ModbusError::custom(
          EINVAL, "getModbusString(): Unsupported register {}", reg.describe());
    }

    if (static_cast<size_t>(reg.ADDR + reg.NB) > regs.size()) {
      throw ModbusError::custom(
          EINVAL, "getModbusString(): Register range out of bounds {}",
          reg.describe());
    }

    str.reserve(reg.NB * 2); // two characters per 16-bit register

    for (size_t i = 0; i < reg.NB; ++i) {
      uint16_t word = regs[reg.ADDR + i];

      char hi = static_cast<char>((word >> 8) & 0xFF);
      char lo = static_cast<char>(word & 0xFF);

      // Null bytes are padding — skip rather than embed them.
      if (hi != '\0')
        str.push_back(hi);
      if (lo != '\0')
        str.push_back(lo);
    }

    // Unprintable characters mean a register map mismatch or an
    // unvalidated device.
    for (unsigned char c : str) {
      if (!std::isprint(c)) {
        throw ModbusError::custom(
            EINVAL,
            "getModbusString(): String contains unprintable characters {}",
            reg.describe());
      }
    }

  } catch (const ModbusError &e) {
    if (onDeviceError_)
      onDeviceError_(e);
    return std::unexpected(e);
  }

  return str;
}

std::expected<double, ModbusError>
FroniusDevice::getModbusDouble(const std::vector<uint16_t> &regs,
                               const Register &reg,
                               std::optional<Register> sf) const {
  double value = 0.0;

  try {
    // A scale-factor register holds a signed 16-bit exponent: scale = 10^SF.
    double scale = 1.0;
    if (sf.has_value())
      scale = std::pow(
          10.0, static_cast<double>(static_cast<int16_t>(regs[sf->ADDR])));

    switch (reg.TYPE) {
    case Register::Type::INT16:
      value = static_cast<double>(static_cast<int16_t>(regs[reg.ADDR])) * scale;
      break;

    case Register::Type::UINT16:
      value = static_cast<double>(regs[reg.ADDR]) * scale;
      break;

    case Register::Type::UINT32:
      value = static_cast<double>(
                  ModbusUtils::modbus_get_uint32(regs.data() + reg.ADDR)) *
              scale;
      break;

    case Register::Type::FLOAT:
      // 32-bit IEEE 754 float stored in ABCD byte order (big-endian words,
      // big-endian bytes within each word) — standard SunSpec encoding.
      value =
          static_cast<double>(modbus_get_float_abcd(regs.data() + reg.ADDR));
      break;

    default:
      throw ModbusError::custom(
          EINVAL, "getModbusDouble(): Unsupported register {}", reg.describe());
    }

  } catch (const ModbusError &e) {
    if (onDeviceError_)
      onDeviceError_(e);
    return std::unexpected(e);
  }

  return value;
}

std::expected<double, ModbusError>
FroniusDevice::getModbusDouble(const std::vector<uint16_t> &regs,
                               const Register &reg, double sf) const {
  // The proprietary Fronius RTU map fixes its scale factors at compile time
  // instead of carrying scale-factor registers, and stores values as INT32
  // in little-endian word order.
  if (reg.TYPE != Register::Type::INT32) {
    return reportError<double>(std::unexpected(ModbusError::custom(
        EINVAL, "getModbusDouble(): Unsupported register {}", reg.describe())));
  }

  return static_cast<double>(
             ModbusUtils::modbus_get_int32(regs.data() + reg.ADDR,
                                           /*word_swap=*/true)) *
         sf;
}

// -------------------------------------------------------------------------
// Public API
// -------------------------------------------------------------------------

std::expected<std::string, ModbusError> FroniusDevice::getManufacturer() {
  return getModbusString(regs_, C001::MN);
}

std::expected<std::string, ModbusError> FroniusDevice::getDeviceModel() {
  return getModbusString(regs_, C001::MD);
}

std::expected<std::string, ModbusError> FroniusDevice::getOptions() {
  return getModbusString(regs_, C001::OPT);
}

std::expected<std::string, ModbusError> FroniusDevice::getFwVersion() {
  return getModbusString(regs_, C001::VR);
}

std::expected<std::string, ModbusError> FroniusDevice::getSerialNumber() {
  return getModbusString(regs_, C001::SN);
}

std::expected<uint16_t, ModbusError> FroniusDevice::getModbusDeviceAddress() {
  uint16_t val = regs_[C001::DA.ADDR];

  // EPROTO, not EINVAL: an out-of-range slave ID off the wire is a protocol
  // fault, not a programmer error. EINVAL deduces to FATAL and would tear
  // down the process; EPROTO deduces to TRANSIENT and revalidates the device
  // instead, which is what a booting or briefly silent slave needs.
  if ((val < 1) || (val > 247))
    return reportError<uint16_t>(std::unexpected(
        ModbusError::custom(EPROTO,
                            "getModbusDeviceAddress(): Invalid Modbus slave "
                            "address: received {}, expected 1-247",
                            val)));
  return val;
}
