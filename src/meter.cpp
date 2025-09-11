#include "../include/meter.h"

#include <modbus_error.h>

Meter::Meter() {
  // TODO Auto-generated constructor stub
}

Meter::~Meter() {
  // TODO Auto-generated destructor stub
}

/*
std::expected<void, ModbusError> Meter::ReadRegisters(void) {

  int rc =
      modbus_read_registers(Ctx, C001_Mn.reg, 65, Regs.data() + C001_Mn.reg);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_Mn.reg) + " failed: "));
  }
  return {};
}
*/