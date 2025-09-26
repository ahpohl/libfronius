#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_

#include <string>

struct ModbusConfig {
  bool debug{false};
  bool useTcp{true};
  std::string host;
  int port{502};
  std::string device;
  int baud{9600};
  int minRetryDelay{5};
  int maxRetryDelay{320};
  int pingInterval{1};
};

#endif /* MB_CONFIG_H_ */
