#ifndef MODBUS_ERROR_H_
#define MODBUS_ERROR_H_

#include <cerrno>
#include <modbus/modbus.h>
#include <string>

struct ModbusError {
  enum class Severity { TRANSIENT, FATAL };

  int code;
  std::string message;
  Severity severity;

public:
  // Factory for errors based on errno
  static ModbusError fromErrno(const std::string &msg) {
    return {errno, msg, deduceSeverity(errno)};
  }

  // Factory for custom errors with specific code
  static ModbusError custom(int c, const std::string &msg) {
    return {c, msg, deduceSeverity(c)};
  }

private:
  // Deduce severity based on errno or custom code
  static Severity deduceSeverity(int c) {
    switch (c) {
    case EINVAL: // Invalid argument
    case ENOMEM: // Out of memory
      return Severity::FATAL;
    default:
      return Severity::TRANSIENT;
    }
  }
};

#endif /* MODBUS_ERROR_H_ */
