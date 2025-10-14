#ifndef REGISTER_BASE_H_
#define REGISTER_BASE_H_

#include <cstdint>

struct Register {
  uint16_t ADDR{0};
  uint16_t NB{0};
  constexpr Register(uint16_t addr, uint16_t nb) : ADDR(addr), NB(nb) {}
};

#endif /* REGISTER_BASE_H_ */
