#include "modbus_error.h"
#include <modbus/modbus.h>

ModbusError ModbusError::make(int c, std::string msg) {
  ModbusError err;
  err.code = c;
  err.message = std::move(msg);
  err.severity = deduceSeverity(c);
  const char *txt = modbus_strerror(c);
  err.libText = txt ? txt : "";
  return err;
}