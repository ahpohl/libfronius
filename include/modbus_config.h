#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_

#include <string>

struct ModbusConfig {
  bool useTcp{true};
  std::string host;
  int port{502};
  std::string device;
  int baud{9600};

  // Exponential backoff parameters (seconds)
  int minRetryDelay{5};
  int maxRetryDelay{300};
};

#endif /* MB_CONFIG_H_ */
