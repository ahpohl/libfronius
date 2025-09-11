#ifndef MODBUS_ERROR_H_
#define MODBUS_ERROR_H_

#include <cerrno>
#include <modbus/modbus.h>
#include <string>

struct ModbusError {
  int code;
  std::string message;

  static ModbusError fromErrno(const std::string &context = "") {
    return {errno, context + ": " + std::string(modbus_strerror(errno))};
  }

  static ModbusError custom(int errCode, const std::string &msg) {
    return {errCode, msg};
  }
};

#endif /* MODBUS_ERROR_H_ */
