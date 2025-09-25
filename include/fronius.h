#ifndef FRONIUS_H_
#define FRONIUS_H_

#include "modbus_config.h"
#include "modbus_error.h"
#include <condition_variable>
#include <expected>
#include <functional>
#include <modbus/modbus.h>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Fronius {
public:
  explicit Fronius(const ModbusConfig &cfg);
  virtual ~Fronius();

  bool isConnected(void) const;

  /** Start connection thread */
  std::expected<void, ModbusError> connect();

  /** Wait indefinitely for connection to be established */
  void waitForConnection(void);

  /** Set callbacks (thread-safe) */
  void setOnConnect(std::function<void()> cb);
  void setOnDisconnect(std::function<void()> cb);

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
  std::expected<bool, ModbusError> isSunSpecDevice(void);

  /** Fetch the complete Fronius Common Register Map from device */
  std::expected<void, ModbusError> fetchCommonRegisters(void);

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

  /** Phase names */
  enum class Phase {
    TOTAL,
    AVERAGE,
    PHA,
    PHB,
    PHC,
    PHAB,
    PHBC,
    PHCA,
    Q1,
    Q1PHA,
    Q1PHB,
    Q1PHC,
    Q2,
    Q2PHA,
    Q2PHB,
    Q2PHC,
    Q3,
    Q3PHA,
    Q3PHB,
    Q3PHC,
    Q4,
    Q4PHA,
    Q4PHB,
    Q4PHC
  };

protected:
  /* Connection handle for the libmodbus context */
  modbus_t *ctx_{nullptr};

  /* Vector to hold the complete register map */
  std::vector<uint16_t> regs_;

private:
  const ModbusConfig cfg_;
  std::thread connectionThread_;
  mutable std::mutex mtx_;
  std::condition_variable cv_;
  std::atomic<bool> running_{false};
  std::atomic<bool> connected_{false};

  /** forward declarations */
  void connectionLoop();
  std::expected<void, ModbusError> tryConnect();

  /** Optional callbacks (can also be set directly) */
  std::function<void()> onConnect;
  std::function<void()> onDisconnect;
};

#endif /* FRONIUS_H_ */
