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

/**
 * @class Fronius
 * @brief Provides a Modbus communication interface for Fronius devices.
 *
 * The Fronius class handles establishing and maintaining a Modbus
 * TCP or RTU connection to Fronius devices, reading common device
 * information registers, and reporting connection or protocol errors
 * via callbacks.
 */
class Fronius {
public:
  /**
   * @brief Construct a new Fronius instance.
   *
   * @param cfg Modbus configuration parameters.
   */
  explicit Fronius(const ModbusConfig &cfg);

  /**
   * @brief Destroy the Fronius instance and stop any running connection
   * threads.
   */
  virtual ~Fronius();

  /**
   * @brief Set a callback to be invoked when a connection is established.
   *
   * @param cb Function to call upon successful connection.
   * @note Thread-safe.
   */
  void setConnectCallback(std::function<void()> cb);

  /**
   * @brief Set a callback to be invoked when the connection is lost.
   *
   * @param cb Function to call when disconnected.
   * @note Thread-safe.
   */
  void setDisconnectCallback(std::function<void()> cb);

  /**
   * @brief Set a callback to report Modbus communication errors.
   *
   * @param cb Function to call when an error occurs.
   * @note Thread-safe.
   */
  void setErrorCallback(std::function<void(const ModbusError &)> cb);

  /**
   * @brief Start the asynchronous connection loop in a separate thread.
   *
   * This function attempts to establish a connection based on the
   * configuration (TCP or RTU). It returns immediately after launching
   * the thread.
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> connect();

  /**
   * @brief Block until a connection to the device has been established.
   *
   * This function waits indefinitely until the connection thread
   * reports a successful connection.
   */
  void waitForConnection(void);

  /**
   * @brief Returns true if the device is currently connected.
   *
   * This reads the internal atomic `connected_` state.
   */
  bool isConnected() const { return connected_.load(); }

  /**
   * @brief Marks the Modbus device as disconnected and triggers a reconnection
   * attempt.
   *
   * This function updates the internal connection state to "disconnected" and
   * wakes the connection loop to attempt a new connection. It does not forcibly
   * close the physical Modbus session; it is typically used after communication
   * errors or to recover from transient connection issues.
   */
  void triggerReconnect(void);

  /**
   * @brief Get the manufacturer name from the device.
   *
   * @return `std::expected<std::string, ModbusError>`
   *         On success, returns `"Fronius"`.
   */
  std::expected<std::string, ModbusError> getManufacturer(void);

  /**
   * @brief Get the device model name.
   *
   * @return `std::expected<std::string, ModbusError>`
   *         On success, returns a string like `"IG+150V [3p]"`.
   */
  std::expected<std::string, ModbusError> getDeviceModel(void);

  /**
   * @brief Get the software version of any installed option (e.g.,
   * Datamanager).
   *
   * @return `std::expected<std::string, ModbusError>`
   *         On success, returns the firmware version string.
   */
  std::expected<std::string, ModbusError> getOptions(void);

  /**
   * @brief Get the main device firmware version.
   *
   * @return `std::expected<std::string, ModbusError>`
   *         On success, returns the firmware version string.
   */
  std::expected<std::string, ModbusError> getFwVersion(void);

  /**
   * @brief Get the serial number of the connected device.
   *
   * @return `std::expected<std::string, ModbusError>`
   *         On success, returns the serial number string.
   */
  std::expected<std::string, ModbusError> getSerialNumber(void);

  /**
   * @brief Get the Modbus slave address reported by the remote device.
   *
   * @return `std::expected<uint16_t, ModbusError>`
   *         On success, returns the slave ID.
   */
  std::expected<uint16_t, ModbusError> getModbusDeviceAddress(void);

  /**
   * @enum Phase
   * @brief Identifiers for phase and input related values.
   */
  enum class Phase { TOTAL, AVERAGE, A, B, C };

  /**
   * @enum Input
   * @brief Identifiers for MPPT inputs or DC strings of the inverter.
   *
   * Represents the multiple independent DC input channels (or strings) of the
   * inverter, typically used in multiple MPPT configurations.
   */
  enum class Input { TOTAL, A, B };

protected:
  /**
   * @brief Handle for the libmodbus connection context.
   */
  modbus_t *ctx_{nullptr};

  /**
   * @brief Buffer storing the complete device register map.
   */
  std::vector<uint16_t> regs_;

  /**
   * @brief Report a Modbus error via the registered error callback.
   *
   * @tparam T Expected return type.
   * @param res The result to report. If it contains an error, the
   *        registered error callback will be invoked.
   * @return The same expected result, unmodified.
   */
  template <typename T>
  std::expected<T, ModbusError>
  reportError(std::expected<T, ModbusError> &&res) {
    if (!res && onError_) {
      onError_(res.error());
    }
    return std::move(res);
  }

  /**
   * @brief Validate that the connected device is SunSpec-compliant.
   *
   * @return `std::expected<bool, ModbusError>`
   *         Returns `true` if the device supports SunSpec.
   */
  std::expected<bool, ModbusError> validateSunSpecRegisters(void);

  /**
   * @brief Fetch the complete Fronius Common Register Map from the device.
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> fetchCommonRegisters(void);

private:
  /** @brief Configuration object defining Modbus parameters. */
  const ModbusConfig cfg_;

  /** @brief Background thread managing connection and reconnection logic. */
  std::thread connectionThread_;

  /** @brief Mutex protecting condition variable access. */
  mutable std::mutex mtx_;

  /** @brief Condition variable used to signal connection state changes. */
  std::condition_variable cv_;

  /** @brief Flag indicating if the connection loop thread is running. */
  std::atomic<bool> running_{false};

  /** @brief Flag indicating if the device is currently connected. */
  std::atomic<bool> connected_{false};

  /** @brief Optional callback invoked on successful connection. */
  std::function<void()> onConnect_;

  /** @brief Optional callback invoked when disconnected. */
  std::function<void()> onDisconnect_;

  /** @brief Optional callback invoked on Modbus communication error. */
  std::function<void(const ModbusError &)> onError_;

  /**
   * @brief Connection management loop.
   *
   * Continuously attempts to connect to the device until successful
   * or until the running flag is cleared.
   */
  void connectionLoop();

  /**
   * @brief Attempt a single connection to the Modbus device.
   *
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   */
  std::expected<void, ModbusError> tryConnect();

  /**
   * @brief Establish a Modbus TCP/IPv4 connection.
   *
   * Allocates and initializes a Modbus context to communicate with
   * a TCP server.
   *
   * @param host Hostname or IP address of the Modbus server.
   * @param port Port number of the Modbus server (default: 502).
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   *
   * @see connectModbusRtu()
   */
  std::expected<void, ModbusError> connectModbusTcp(const std::string &host,
                                                    const int port = 502);

  /**
   * @brief Establish a Modbus RTU serial connection.
   *
   * Allocates and initializes a Modbus context to communicate
   * over a serial line in RTU mode.
   *
   * @param device Serial device name (e.g., `/dev/ttyUSB0`).
   * @param baud Baud rate (typically 9600 or 19200).
   * @return `std::expected<void, ModbusError>` indicating success or failure.
   *
   * @see connectModbusTcp()
   */
  std::expected<void, ModbusError> connectModbusRtu(const std::string &device,
                                                    const int baud = 9600);
};

#endif /* FRONIUS_H_ */
