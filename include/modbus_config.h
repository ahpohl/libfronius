/**
 * @file modbus_config.h
 * @brief Type-safe configuration for Modbus connections (TCP/RTU)
 *
 * @details
 * Provides convenient structures for storing all parameters needed to
 * configure a Modbus client, including TCP/RTU connection settings,
 * slave ID, debugging, and automatic reconnect parameters. The active
 * transport type is encoded in the type system: holding a
 * `ModbusTcpTransport` is unambiguous. Includes a `validate()` method
 * to ensure all parameters are within allowed ranges before use.
 */

#ifndef MODBUS_CONFIG_H_
#define MODBUS_CONFIG_H_

#include <stdexcept>
#include <string>
#include <variant>

// ---------------------------------------------------------------------------
// Transport descriptors
// ---------------------------------------------------------------------------

/**
 * @struct ModbusTcpTransport
 * @brief Parameters for a Modbus TCP connection.
 */
struct ModbusTcpTransport {
  /** @brief Hostname or IPv4/IPv6 address of the Modbus TCP server. */
  std::string host;

  /** @brief TCP port (default 502). */
  int port{502};
};

/**
 * @struct ModbusRtuTransport
 * @brief Parameters for a Modbus RTU serial connection.
 *
 */
struct ModbusRtuTransport {
  /** @brief Serial device path (e.g. "/dev/ttyUSB0"). */
  std::string device;

  /** @brief Baud rate (e.g. 9600, 19200). */
  int baud{9600};

  /** @brief Number of data bits (5–8). */
  int dataBits{8};

  /** @brief Number of stop bits (1 or 2). */
  int stopBits{1};

  /** @brief Parity: 'N' (none), 'E' (even), or 'O' (odd). */
  char parity{'N'};
};

// ---------------------------------------------------------------------------
// Unified per-device config
// ---------------------------------------------------------------------------

/**
 * @struct ModbusConfig
 * @brief Complete configuration for one Modbus device
 *
 * @details
 * Combines a type-discriminated transport (TCP or RTU) with the per-device
 * parameters that are independent of the physical bus: slave ID, response
 * timeout, reconnect policy, and debug flag.
 *
 * On RTU, the per-device slave ID allows multiple `ModbusConnectionConfig`
 * values to share a single `ModbusRtuBus` (same `device` path) while each
 * addressing a different slave.
 */
struct ModbusConfig {
  /** @brief Transport: either a TCP connection or an RTU serial bus. */
  std::variant<ModbusTcpTransport, ModbusRtuTransport> transport;

  /** @brief Enable debug logging */
  bool debug{false};

  // --- Per-device parameters ---

  /** @brief Modbus slave ID (1–247) */
  int slaveId{1};

  /** @brief Timeout for response in seconds */
  int secTimeout{0};

  /** @brief Timeout for response in micro seconds */
  int usecTimeout{200000};

  // --- Reconnect policy ---

  /** @brief Initial reconnect delay in seconds */
  int reconnectDelay{5};

  /** @brief Maximum reconnect delay in seconds */
  int reconnectDelayMax{320};

  /** @brief Use exponential backoff for reconnect if true */
  bool exponential{true};

  // --- Convenience accessors ---

  /** @brief Returns true when the active transport is TCP. */
  [[nodiscard]] bool isTcp() const noexcept {
    return std::holds_alternative<ModbusTcpTransport>(transport);
  }

  /** @brief Returns true when the active transport is RTU. */
  [[nodiscard]] bool isRtu() const noexcept {
    return std::holds_alternative<ModbusRtuTransport>(transport);
  }

  /**
   * @brief Access the TCP transport parameters.
   * @throws std::bad_variant_access if the active transport is not TCP.
   */
  [[nodiscard]] const ModbusTcpTransport &tcp() const {
    return std::get<ModbusTcpTransport>(transport);
  }

  /**
   * @brief Access the RTU transport parameters.
   * @throws std::bad_variant_access if the active transport is not RTU.
   */
  [[nodiscard]] const ModbusRtuTransport &rtu() const {
    return std::get<ModbusRtuTransport>(transport);
  }

  /**
   * @brief Validate configuration parameters.
   * @throws std::invalid_argument if any parameter is out of allowed range.
   */
  void validate() const {
    if (slaveId < 1 || slaveId > 247)
      throw std::invalid_argument("slaveId must be in range 1–247");

    if (isRtu()) {
      const auto &r = rtu();

      if (r.baud <= 0)
        throw std::invalid_argument("RTU naud rate must be positive");
      if (r.dataBits < 5 || r.dataBits > 8)
        throw std::invalid_argument("RTU dataBits must be in range 5–8");
      if (r.stopBits != 1 && r.stopBits != 2)
        throw std::invalid_argument("RTU stopBits must be 1 or 2");
      if (r.parity != 'N' && r.parity != 'E' && r.parity != 'O')
        throw std::invalid_argument("RTU parity must be 'N', 'E', or 'O'");
    }

    if (isTcp()) {
      const auto &t = tcp();

      if (t.port <= 0 || t.port > 65535)
        throw std::invalid_argument("TCP port must be in range 1–65535");
    }

    if (reconnectDelay <= 0 || reconnectDelayMax <= 0)
      throw std::invalid_argument(
          "reconnectDelay and reconnectDelayMax must be positive");
    if (reconnectDelay >= reconnectDelayMax)
      throw std::invalid_argument(
          "reconnectDelay must be less than reconnectDelayMax");
    if (secTimeout == 0 && usecTimeout == 0)
      throw std::invalid_argument(
          "secTimeout and usecTimeout cannot both be 0");
    if (usecTimeout < 0 || usecTimeout > 999999)
      throw std::invalid_argument("usecTimeout must be in range 0-999999");
  }
};

#endif /* MODBUS_CONFIG_H_ */
