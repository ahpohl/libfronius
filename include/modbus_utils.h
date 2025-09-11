#ifndef MODBUS_UTILS_H_
#define MODBUS_UTILS_H_

#include <cstdint>
#include <string>

inline uint16_t swap_bytes16(uint16_t v) { return (v >> 8) | (v << 8); }

inline uint32_t modbus_get_uint32(const uint16_t *regs, bool word_swap = false,
                                  bool byte_swap = false) {
  uint16_t hi = regs[0], lo = regs[1];
  if (byte_swap) {
    hi = swap_bytes16(hi);
    lo = swap_bytes16(lo);
  }
  uint32_t val = ((uint32_t)hi << 16) | lo;
  if (word_swap)
    val = ((val & 0xFFFF) << 16) | (val >> 16);
  return val;
}

inline int32_t modbus_get_int32(const uint16_t *regs, bool word_swap = false,
                                bool byte_swap = false) {
  return static_cast<int32_t>(modbus_get_uint32(regs, word_swap, byte_swap));
}

inline uint64_t modbus_get_uint64(const uint16_t *regs, bool word_swap = false,
                                  bool byte_swap = false) {
  uint64_t val = 0;
  for (int i = 0; i < 4; i++) {
    uint16_t w = regs[i];
    if (byte_swap)
      w = swap_bytes16(w);
    val |= (uint64_t)w << ((3 - i) * 16);
  }
  if (word_swap) {
    val = ((val & 0xFFFFULL) << 48) | ((val & 0xFFFF0000ULL) << 16) |
          ((val >> 16) & 0xFFFF0000ULL) | ((val >> 48) & 0xFFFFULL);
  }
  return val;
}

inline int64_t modbus_get_int64(const uint16_t *regs, bool word_swap = false,
                                bool byte_swap = false) {
  return static_cast<int64_t>(modbus_get_uint64(regs, word_swap, byte_swap));
}

inline std::string modbus_get_string(const uint16_t *tab_reg, int size) {
  std::string str;
  for (int i = 0; i < size; i++) {
    str.push_back((tab_reg[i] >> 8) & 0xFF);
    str.push_back(tab_reg[i]);
  }
  return str;
}

#endif /* MODBUS_UTILS_H_ */
