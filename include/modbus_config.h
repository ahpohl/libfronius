/**
 * @file modbus_config.h
 * @brief Configuration types for Modbus bus and device connections.
 *
 * @details
 * Provides two distinct configuration structures matching the two levels
 * of the shared-bus architecture:
 *
 * - `ModbusBusConfig`    — physical bus parameters (transport, reconnect
 *                          policy, debug). One per serial port or TCP
 *                          endpoint. Passed to `FroniusBus`.
 * - `ModbusDeviceConfig` — per-slave parameters (slave ID, response
 *                          timeout, reconnect policy). One per device.
 *                          Passed to `Meter`, `Inverter`, etc.
 *
 * The active transport is encoded as a `std::variant` so RTU and TCP
 * parameters cannot be mixed up. Both configs expose `validate()` to
 * catch out-of-range parameters at construction time.
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
 */
struct ModbusRtuTransport {
  /** @brief Serial device path (e.g. "/dev/ttyUSB0"). */
  std::string device;

  /** @brief Baud rate (e.g. 9600, 19200). */
  int baud{9600};

  /** @brief Number of data bits (5-8). */
  int dataBits{8};

  /** @brief Number of stop bits (1 or 2). */
  int stopBits{1};

  /** @brief Parity: 'N' (none), 'E' (even), or 'O' (odd). */
  char parity{'N'};
};

// ---------------------------------------------------------------------------
// Bus-level config  (one per physical bus / TCP connection -> FroniusBus)
// ---------------------------------------------------------------------------

/**
 * @struct ModbusBusConfig
 * @brief Configuration for a shared Modbus bus.
 *
 * Holds parameters that belong to the physical transport and are shared by
 * every device on the bus: the transport descriptor, the reconnect policy,
 * and the debug flag. Per-device parameters belong in `ModbusDeviceConfig`.
 *
 * On RTU one instance covers all slaves on the same serial port; on TCP
 * each remote host gets its own instance. Pass to `FroniusBus`.
 */
struct ModbusBusConfig {
  /** @brief Transport: either a TCP connection or an RTU serial bus. */
  std::variant<ModbusTcpTransport, ModbusRtuTransport> transport;

  /** @brief Enable libmodbus debug logging for this bus. */
  bool debug{false};

  // --- Reconnect policy ---

  /** @brief Initial reconnect delay in seconds. */
  int reconnectDelay{5};

  /** @brief Maximum reconnect delay in seconds. */
  int reconnectDelayMax{320};

  /** @brief Use exponential backoff for reconnect if true. */
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
   * @brief Validate bus configuration parameters.
   * @throws std::invalid_argument if any parameter is out of allowed range.
   */
  void validate() const {
    if (isRtu()) {
      const auto &r = rtu();

      if (r.baud <= 0)
        throw std::invalid_argument("RTU baud rate must be positive");
      if (r.dataBits < 5 || r.dataBits > 8)
        throw std::invalid_argument("RTU dataBits must be in range 5-8");
      if (r.stopBits != 1 && r.stopBits != 2)
        throw std::invalid_argument("RTU stopBits must be 1 or 2");
      if (r.parity != 'N' && r.parity != 'E' && r.parity != 'O')
        throw std::invalid_argument("RTU parity must be 'N', 'E', or 'O'");
    }

    if (isTcp()) {
      const auto &t = tcp();

      if (t.port <= 0 || t.port > 65535)
        throw std::invalid_argument("TCP port must be in range 1-65535");
    }

    if (reconnectDelay <= 0 || reconnectDelayMax <= 0)
      throw std::invalid_argument(
          "reconnectDelay and reconnectDelayMax must be positive");
    if (reconnectDelay >= reconnectDelayMax)
      throw std::invalid_argument(
          "reconnectDelay must be less than reconnectDelayMax");
  }
};

// ---------------------------------------------------------------------------
// Device-level config  (one per slave device -> Meter, Inverter, ...)
// ---------------------------------------------------------------------------

/**
 * @struct ModbusDeviceConfig
 * @brief Configuration for a single Modbus slave device.
 *
 * Holds the slave ID, the response timeout the bus thread applies for this
 * device, and the per-device reconnect policy used by `scheduleDeviceRetry`.
 * Multiple devices on the same RTU bus each have their own instance with
 * distinct `slaveId` values but share a single `ModbusBusConfig`.
 *
 * Pass to `Meter`, `Inverter`, etc. alongside a `shared_ptr<FroniusBus>`.
 */
struct ModbusDeviceConfig {
  /** @brief Modbus slave ID (1-247). */
  int slaveId{1};

  /** @brief Response timeout - whole seconds component. */
  int secTimeout{0};

  /**
   * @brief Response timeout - microseconds component (0-999999).
   *
   * The total timeout is secTimeout + usecTimeout / 1e6.
   * Default of 200 ms is suitable for most RS485 installations.
   */
  int usecTimeout{200000};

  /** @brief Initial reconnect delay in seconds. */
  int reconnectDelay{5};

  /** @brief Maximum reconnect delay in seconds. */
  int reconnectDelayMax{320};

  /** @brief Use exponential backoff for reconnect if true. */
  bool exponential{true};

  /**
   * @brief Validate device configuration parameters.
   * @throws std::invalid_argument if any parameter is out of allowed range.
   */
  void validate() const {
    if (slaveId < 1 || slaveId > 247)
      throw std::invalid_argument("slaveId must be in range 1-247");
    if (secTimeout == 0 && usecTimeout == 0)
      throw std::invalid_argument(
          "secTimeout and usecTimeout cannot both be 0");
    if (usecTimeout < 0 || usecTimeout > 999999)
      throw std::invalid_argument(
          "usecTimeout must be in range 0-999999");
    if (reconnectDelay <= 0 || reconnectDelayMax <= 0)
      throw std::invalid_argument(
          "reconnectDelay and reconnectDelayMax must be positive");
    if (reconnectDelay >= reconnectDelayMax)
      throw std::invalid_argument(
          "reconnectDelay must be less than reconnectDelayMax");
  }
};

#endif /* MODBUS_CONFIG_H_ */
