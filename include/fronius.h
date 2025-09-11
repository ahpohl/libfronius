#ifndef FRONIUS_H_
#define FRONIUS_H_

#include "modbus_error.h"
#include <expected>
#include <modbus/modbus.h>
#include <string>
#include <vector>

class Fronius {
public:
  Fronius();
  virtual ~Fronius();

  /** Create a Modbus context for TCP/IPv4

     The ConnectTcp() function shall allocate and initialize a modbus_t
     structure to communicate with a Modbus TCP IPv4/IPv6 server.

     @param host hostname or address of the server to which the client wants to
     establish a connection
     @param port service name/port number to connect to
     */
  std::expected<void, ModbusError> connectModbusTcp(const std::string &host,
                                                    const int port = 502);
  /** Create a Modbus context for RTU serial

     The ConnectRtu() function shall allocate and initialize a modbus_t
     structure to communicate in RTU mode on a serial line.

     @param device device specifies the name of the serial port
     @param baud_rate baud rate (9600 or 19200)
   */
  std::expected<void, ModbusError> connectModbusRtu(const std::string &device,
                                                    const int baud = 9600);

  /** Read the complete Fronius Common Register Map */
  std::expected<void, ModbusError> readCommonRegisters(void);

protected:
  modbus_t *ctx_;
  std::vector<uint16_t> regs_;
};

#endif /* FRONIUS_H_ */
