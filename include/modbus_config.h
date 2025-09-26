#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_

#include <string>

struct ModbusConfig {
  bool debug{false};

  // Connection parameters
  int slave_id{1};
  bool useTcp{true};
  std::string host;
  int port{502};
  std::string device;
  int baud{9600};

  // Reconnect parameters
  int minRetryDelay{5};
  int maxRetryDelay{320};
};

#endif /* MB_CONFIG_H_ */
