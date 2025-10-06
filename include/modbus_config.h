/**
 * @file mb_config.h
 * @brief Configuration structure for Modbus connections (TCP/RTU) and
 * reconnection behavior.
 *
 * @details
 * Provides a convenient struct for storing all parameters needed to
 * configure a Modbus client, including TCP/RTU connection settings,
 * slave ID, debugging, and automatic reconnect parameters.
 * Includes a `validate()` method to ensure all parameters are within
 * allowed ranges before use.
 */

#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_

#include <stdexcept>
#include <string>

/** @struct ModbusConfig
 *  @brief Holds all configuration options for a Modbus client.
 *
 *  @details
 *  Contains TCP/RTU connection settings, slave ID, debug flag,
 *  and reconnect parameters. Call `validate()` to ensure configuration
 *  values are within valid ranges.
 */
struct ModbusConfig {
  /** @brief Enable debug logging */
  bool debug{false};

  // --- Connection parameters ---

  /** @brief Modbus slave ID (1–247) */
  int slaveId{1};

  /** @brief Use TCP if true, else use RTU (serial) */
  bool useTcp{true};

  /** @brief TCP host name or IP address (used if `useTcp` is true) */
  std::string host;

  /** @brief TCP port (default 502) */
  int port{502};

  /** @brief Serial device path (used if `useTcp` is false, e.g.,
   * "/dev/ttyUSB0") */
  std::string device;

  /** @brief Serial baud rate (used if `useTcp` is false) */
  int baud{9600};

  // --- Reconnect parameters ---

  /** @brief Initial reconnect delay in seconds */
  int reconnectDelay{5};

  /** @brief Maximum reconnect delay in seconds */
  int reconnectDelayMax{320};

  /** @brief Use exponential backoff for reconnect if true */
  bool exponential{true};

  /**
   * @brief Validate configuration parameters.
   * @throws std::invalid_argument if any parameter is out of allowed range.
   *
   * @details
   * Checks:
   *  - slaveId must be in 1–247
   *  - baud rate must be positive
   *  - TCP port must be 1–65535
   *  - reconnectDelay and reconnectDelayMax must be positive
   *  - reconnectDelay must be smaller than reconnectDelayMax
   */
  void validate() const {
    if (slaveId < 1 || slaveId > 247) {
      throw std::invalid_argument(
          "Slave ID must be in range 1–247 for unicast");
    }
    if (baud <= 0) {
      throw std::invalid_argument("Baud rate must be positive");
    }
    if (port <= 0 || port > 65535) {
      throw std::invalid_argument("TCP port must be in range 1–65535");
    }
    if (reconnectDelay < 0 || reconnectDelayMax < 0) {
      throw std::invalid_argument(
          "reconnectDelay and reconnectDelayMax must be > 0");
    }
    if (reconnectDelay >= reconnectDelayMax) {
      throw std::invalid_argument(
          "reconnectDelay must be smaller than reconnectDelayMax");
    }
  }
};

#endif /* MB_CONFIG_H_ */
