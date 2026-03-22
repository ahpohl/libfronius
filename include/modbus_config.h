/**
 * @file modbus_config.h
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

  /** @brief Serial data bits (used if `useTcp` is false, typically 7 or 8) */
  int dataBits{8};

  /** @brief Serial stop bits (used if `useTcp` is false, 1 or 2) */
  int stopBits{1};

  /** @brief Serial parity character (used if `useTcp` is false: 'N', 'E', or
   * 'O') */
  char parity{'N'};

  /** @brief Timeout for response in seconds */
  int secTimeout{0};

  /** @brief Timeout for response in micro seconds */
  int usecTimeout{200000};

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
   *  - dataBits must be 5–8
   *  - stopBits must be 1 or 2
   *  - parity must be 'N', 'E', or 'O'
   *  - TCP port must be 1–65535
   *  - reconnectDelay and reconnectDelayMax must be positive
   *  - reconnectDelay must be smaller than reconnectDelayMax
   */
  void validate() const {
    if (slaveId < 1 || slaveId > 247) {
      throw std::invalid_argument(
          "Slave ID must be in range 1–247 for unicast");
    }
    if (!useTcp) {
      if (baud <= 0) {
        throw std::invalid_argument("Baud rate must be positive");
      }
      if (dataBits < 5 || dataBits > 8) {
        throw std::invalid_argument("dataBits must be in range 5–8");
      }
      if (stopBits != 1 && stopBits != 2) {
        throw std::invalid_argument("stopBits must be 1 or 2");
      }
      if (parity != 'N' && parity != 'E' && parity != 'O') {
        throw std::invalid_argument("parity must be 'N', 'E', or 'O'");
      }
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
    if (secTimeout == 0 && usecTimeout == 0) {
      throw std::invalid_argument(
          "Both secTimeout and usecTimeout cannot be 0");
    }
    if (usecTimeout < 0 || usecTimeout > 999999) {
      throw std::invalid_argument("usecTimeout must be in range 0-999999");
    }
  }
};

#endif /* MB_CONFIG_H_ */
