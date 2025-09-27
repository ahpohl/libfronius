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

  // Factory for errors based on errno
  static ModbusError fromErrno(const std::string &msg,
                               Severity sev = Severity::TRANSIENT) {
    return {errno, msg, sev};
  }

  // Factory for custom errors with specific code
  static ModbusError custom(int c, const std::string &msg,
                            Severity sev = Severity::TRANSIENT) {
    return {c, msg, sev};
  }
};

#endif /* MODBUS_ERROR_H_ */
