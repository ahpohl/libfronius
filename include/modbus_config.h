#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_

#include <stdexcept>
#include <string>

struct ModbusConfig {
  bool debug{false};

  // Connection parameters
  int slaveId{1};
  bool useTcp{true};
  std::string host;
  int port{502};
  std::string device;
  int baud{9600};

  // Reconnect parameters
  int minRetryDelay{5};
  int maxRetryDelay{320};

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
    if (minRetryDelay < 0 || maxRetryDelay < 0) {
      throw std::invalid_argument(
          "minRetryDelay and maxRetryDelay must be > 0");
    }
    if (minRetryDelay >= maxRetryDelay) {
      throw std::invalid_argument(
          "minRetryDelay must be smaller than maxRetryDelay");
    }
  }
};

#endif /* MB_CONFIG_H_ */
