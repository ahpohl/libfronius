/**
 * @file modbus_utils.h
 * @brief Utility functions for working with Modbus registers and data.
 *
 * @details
 * Provides helper functions to extract integer, floating-point, and string
 * values from Modbus register arrays, handle byte and word swaps, and
 * convert values to hex strings. Includes validation for printable ASCII
 * strings.
 *
 * These functions are commonly used with Fronius SunSpec Modbus maps.
 */

#ifndef MODBUS_UTILS_H_
#define MODBUS_UTILS_H_

#include <cstdint>
#include <string>

/** @namespace ModbusUtils
 *  @brief Contains helper functions for Modbus register manipulation.
 */
namespace ModbusUtils {

/**
 * @brief Swap the bytes of a 16-bit value.
 * @param v Input 16-bit value
 * @return 16-bit value with swapped bytes
 */
inline uint16_t swapBytes16(uint16_t v) { return (v >> 8) | (v << 8); }

/**
 * @brief Combine two Modbus registers into a 32-bit unsigned integer.
 * @param regs Pointer to two 16-bit Modbus registers
 * @param word_swap True to swap the high and low 16-bit words
 * @param byte_swap True to swap bytes within each 16-bit word
 * @return 32-bit unsigned integer value
 */
inline uint32_t modbus_get_uint32(const uint16_t *regs, bool word_swap = false,
                                  bool byte_swap = false) {
  uint16_t hi = regs[0], lo = regs[1];
  if (byte_swap) {
    hi = swapBytes16(hi);
    lo = swapBytes16(lo);
  }
  uint32_t val = ((uint32_t)hi << 16) | lo;
  if (word_swap)
    val = ((val & 0xFFFF) << 16) | (val >> 16);
  return val;
}

/**
 * @brief Combine two Modbus registers into a 32-bit signed integer.
 * @param regs Pointer to two 16-bit Modbus registers
 * @param word_swap True to swap the high and low 16-bit words
 * @param byte_swap True to swap bytes within each 16-bit word
 * @return 32-bit signed integer value
 */
inline int32_t modbus_get_int32(const uint16_t *regs, bool word_swap = false,
                                bool byte_swap = false) {
  return static_cast<int32_t>(modbus_get_uint32(regs, word_swap, byte_swap));
}

/**
 * @brief Combine four Modbus registers into a 64-bit unsigned integer.
 * @param regs Pointer to four 16-bit Modbus registers
 * @param word_swap True to swap 16-bit words
 * @param byte_swap True to swap bytes within each 16-bit word
 * @return 64-bit unsigned integer value
 */
inline uint64_t modbus_get_uint64(const uint16_t *regs, bool word_swap = false,
                                  bool byte_swap = false) {
  uint64_t val = 0;
  for (int i = 0; i < 4; i++) {
    uint16_t w = regs[i];
    if (byte_swap)
      w = swapBytes16(w);
    val |= (uint64_t)w << ((3 - i) * 16);
  }
  if (word_swap) {
    val = ((val & 0xFFFFULL) << 48) | ((val & 0xFFFF0000ULL) << 16) |
          ((val >> 16) & 0xFFFF0000ULL) | ((val >> 48) & 0xFFFFULL);
  }
  return val;
}

/**
 * @brief Combine four Modbus registers into a 64-bit signed integer.
 * @param regs Pointer to four 16-bit Modbus registers
 * @param word_swap True to swap 16-bit words
 * @param byte_swap True to swap bytes within each 16-bit word
 * @return 64-bit signed integer value
 */
inline int64_t modbus_get_int64(const uint16_t *regs, bool word_swap = false,
                                bool byte_swap = false) {
  return static_cast<int64_t>(modbus_get_uint64(regs, word_swap, byte_swap));
}

/**
 * @brief Convert a 16-bit value to a hexadecimal string.
 * @param val 16-bit value
 * @return Hex string in uppercase (e.g., "1A2B")
 */
inline std::string toHex(uint16_t val) {
  char buf[7];
  snprintf(buf, sizeof(buf), "%04X", val);
  return std::string(buf);
}

} // namespace ModbusUtils

#endif /* MODBUS_UTILS_H_ */
