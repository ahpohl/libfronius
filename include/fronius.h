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

  /** Test if the Fronius device is SunSpec compatible */
  std::expected<bool, ModbusError> isSunSpecCompatible(void);

  /** Read the complete Fronius Common Register Map */
  std::expected<void, ModbusError> readCommonRegisters(void);

  /** The the device manufacturer

    @returns string, i.e. Fronius
   */
  std::expected<std::string, ModbusError> getManufacturer(void);

  /** Device model

   @returns string, i.e. IG+150V [3p]
   */
  std::expected<std::string, ModbusError> getDeviceModel(void);

  /** Get the software version of installed option

   @returns string, i.e. firmware version of Datamanager
   */
  std::expected<std::string, ModbusError> getOptions(void);

  /** Get the software version of main device

   @returns string, i.e. firmware version of inverter, meter, battery etc.
   */
  std::expected<std::string, ModbusError> getFwVersion(void);

  /** Get the serial number of the device

   @returns string, i.e. serial number
   */
  std::expected<std::string, ModbusError> getSerialNumber(void);

  /** Get the serial number of the device

   @returns string, i.e. serial number
   */
  std::expected<uint16_t, ModbusError> getModbusDeviceAddress(void);

protected:
  modbus_t *ctx_;
  std::vector<uint16_t> regs_;
};

#endif /* FRONIUS_H_ */
