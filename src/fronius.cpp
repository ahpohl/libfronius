#include "fronius.h"
#include "common_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <cerrno>
#include <expected>
#include <modbus/modbus.h>
#include <string>
#include <vector>

Fronius::Fronius() { regs_.resize(0xFFFF, 0); }

Fronius::~Fronius() {
  if (ctx_) {
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }
}

std::expected<void, ModbusError> Fronius::setModbusDebugFlag(const bool &flag) {
  int rc = modbus_set_debug(ctx_, flag);
  if (rc == -1) {
    return std::unexpected(ModbusError::fromErrno(
        std::string("Unable to set the libmodbus debug flag")));
  }

  return {};
}

std::expected<void, ModbusError>
Fronius::connectModbusTcp(const std::string &host, const int port) {
  if (host.empty()) {
    return std::unexpected(ModbusError::custom(EINVAL, "Host argument empty"));
  }
  ctx_ = modbus_new_tcp_pi(host.c_str(), std::to_string(port).c_str());
  if (!ctx_) {
    return std::unexpected(
        ModbusError::custom(ENOMEM, "Unable to create the libmodbus context"));
  }
  if (modbus_connect(ctx_) == -1) {
    return std::unexpected(ModbusError::fromErrno(
        std::string("Connection to '") + host + "' failed"));
  }

  return {};
}

std::expected<void, ModbusError>
Fronius::connectModbusRtu(const std::string &device, const int baud) {
  if (device.empty()) {
    return std::unexpected(
        ModbusError::custom(EINVAL, "Device argument empty"));
  }
  ctx_ = modbus_new_rtu(device.c_str(), baud, 'N', 8, 1);
  if (!ctx_) {
    return std::unexpected(
        ModbusError::custom(ENOMEM, "Unable to create the libmodbus context"));
  }
  if (modbus_connect(ctx_) == -1) {
    return std::unexpected(ModbusError::fromErrno(
        std::string("Connection to '") + device + "' failed"));
  }

  return {};
}

std::expected<bool, ModbusError> Fronius::isSunSpecDevice(void) {

  // Get registers
  int rc = modbus_read_registers(ctx_, C001_SID::ADDR, 4,
                                 regs_.data() + C001_SID::ADDR);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_SID::ADDR) + " failed"));
  }

  // Test for "SunS" string
  if (!(regs_[C001_SID::ADDR] == 0x5375 &&
        regs_[C001_SID::ADDR + 1] == 0x6e53)) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "SunSpec signature mismatch: expected [0x5375, 0x6e53], "
                "received [0x" +
                    modbus_utils::to_hex(regs_[C001_SID::ADDR]) + ", 0x" +
                    modbus_utils::to_hex(regs_[C001_SID::ADDR + 1]) + "]"));
  }

  // Test for Common Register ID
  if (!(regs_[C001_ID::ADDR] = 0x1)) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Not a SunSpec common register map: received " +
                    std::to_string(regs_[C001_ID::ADDR]) + ", expected 1"));
  }

  // Test for Common Register Map Length
  if (regs_[C001_L::ADDR] != C001_SIZE) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid common register map size: received " +
                    std::to_string(regs_[C001_L::ADDR]) + ", expected " +
                    std::to_string(C001_SIZE)));
  }

  return {};
}

std::expected<void, ModbusError> Fronius::fetchCommonRegisters(void) {

  int rc = modbus_read_registers(ctx_, C001_MN::ADDR, C001_SIZE,
                                 regs_.data() + C001_MN::ADDR);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_MN::ADDR) + " failed"));
  }
  return {};
}

std::expected<std::string, ModbusError> Fronius::getManufacturer() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_MN::ADDR,
                                         C001_MN::NB);
}

std::expected<std::string, ModbusError> Fronius::getDeviceModel() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_MD::ADDR,
                                         C001_MD::NB);
}

std::expected<std::string, ModbusError> Fronius::getOptions() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_OPT::ADDR,
                                         C001_OPT::NB);
}

std::expected<std::string, ModbusError> Fronius::getFwVersion() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_VR::ADDR,
                                         C001_VR::NB);
}

std::expected<std::string, ModbusError> Fronius::getSerialNumber() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_SN::ADDR,
                                         C001_SN::NB);
}

std::expected<uint16_t, ModbusError> Fronius::getModbusDeviceAddress() {
  uint16_t val = regs_[C001_DA::ADDR];

  if ((val < 1) || (val > 247))
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid Modbus slave address: received " +
                    std::to_string(val) + ", expected 1-247"));
  return val;
}
