#include "fronius.h"
#include "common_registers.h"
#include "modbus_error.h"
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

std::expected<void, ModbusError> Fronius::readCommonRegisters(void) {

  int rc = modbus_read_registers(ctx_, C001_Mn.reg, CommonMapSize,
                                 regs_.data() + C001_Mn.reg);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_Mn.reg) + " failed: "));
  }
  return {};
}
