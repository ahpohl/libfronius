#include "fronius.h"
#include "common_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <cerrno>
#include <expected>
#include <modbus/modbus.h>
#include <string>
#include <vector>

Fronius::Fronius() : ctx_(nullptr) { regs_.resize(0xFFFF, 0); }

Fronius::~Fronius() { modbus_free(ctx_); }

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
        std::string("Connection to \"") + host + "\" failed"));
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
        std::string("Connection to \"") + device + "\" failed"));
  }

  return {};
}

std::expected<bool, ModbusError> Fronius::isSunSpecCompatible(void) {

  int rc = 0;

  // Get registers
  rc = modbus_read_registers(ctx_, C001_SID::addr, 4, regs_.data());
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_SID::addr) + " failed: "));
  }

  // Test for "SunS" string
  if (!(regs_[0] == 0x5375 && regs_[1] == 0x6e53)) {
    return std::unexpected(ModbusError::custom(
        EADDRNOTAVAIL, "SunSpec signature mismatch: expected [0x5375, 0x6e53], "
                       "received [0x" +
                           modbus_utils::to_hex(regs_[0]) + ", 0x" +
                           modbus_utils::to_hex(regs_[1]) + "]"));
  }

  // Test for Common Register ID
  if (!(regs_[2] = 0x1)) {
    return std::unexpected(ModbusError::custom(
        EADDRNOTAVAIL, "Not a SunSpec common register map: received " +
                           std::to_string(regs_[2]) + ", expected 1"));
  }

  // Test for Common Register Map Length
  if (regs_[3] != COMMON_MAP_SIZE) {
    return std::unexpected(ModbusError::custom(
        EADDRNOTAVAIL, "Invalid common register map size: received " +
                           std::to_string(regs_[3]) + ", expected " +
                           std::to_string(COMMON_MAP_SIZE)));
  }

  return {};
}

std::expected<void, ModbusError> Fronius::readCommonRegisters(void) {

  int rc = modbus_read_registers(ctx_, C001_MN::addr, COMMON_MAP_SIZE,
                                 regs_.data() + C001_MN::addr);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_MN::addr) + " failed: "));
  }
  return {};
}

std::expected<std::string, ModbusError> Fronius::getManufacturer() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_MN::addr,
                                         C001_MN::nb);
}

std::expected<std::string, ModbusError> Fronius::getDeviceModel() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_MD::addr,
                                         C001_MD::nb);
}

std::expected<std::string, ModbusError> Fronius::getOptions() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_OPT::addr,
                                         C001_OPT::nb);
}

std::expected<std::string, ModbusError> Fronius::getFwVersion() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_VR::addr,
                                         C001_VR::nb);
}

std::expected<std::string, ModbusError> Fronius::getSerialNumber() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_SN::addr,
                                         C001_SN::nb);
}

std::expected<uint16_t, ModbusError> Fronius::getModbusDeviceAddress() {
  uint16_t val = regs_[C001_DA::addr];

  if ((val < 1) || (val > 247))
    return std::unexpected(ModbusError::custom(
        EADDRNOTAVAIL, "Invalid Modbus slave address: received " +
                           std::to_string(val) + ", expected 1-247"));
  return val;
}
