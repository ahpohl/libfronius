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

#include "modbus_error.h"
#include "register_base.h"
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

/** @namespace modbus_utils
 *  @brief Contains helper functions for Modbus register manipulation.
 */
namespace modbus_utils {

/**
 * @brief Swap the bytes of a 16-bit value.
 * @param v Input 16-bit value
 * @return 16-bit value with swapped bytes
 */
inline uint16_t swap_bytes16(uint16_t v) { return (v >> 8) | (v << 8); }

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
    hi = swap_bytes16(hi);
    lo = swap_bytes16(lo);
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
      w = swap_bytes16(w);
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
 * @brief Convert Modbus registers to a printable ASCII string.
 * @param tab_reg Pointer to 16-bit Modbus registers
 * @param size Number of registers
 * @return std::expected containing the string on success, or a ModbusError
 *         if unprintable characters are found
 */
inline std::expected<std::string, ModbusError>
modbus_get_string(const uint16_t *tab_reg, int size) {
  std::string str;
  str.reserve(size * 2); // avoid reallocations

  for (int i = 0; i < size; i++) {
    char hi = static_cast<char>((tab_reg[i] >> 8) & 0xFF);
    char lo = static_cast<char>(tab_reg[i] & 0xFF);

    if (hi != '\0')
      str.push_back(hi);
    if (lo != '\0')
      str.push_back(lo);
  }

  if (str.empty())
    return std::string{};

  for (unsigned char c : str) {
    if (!std::isprint(c) && c != ' ') { // allow space but not control chars
      return std::unexpected(ModbusError::custom(
          EINVAL, "String contains unprintable characters"));
    }
  }

  return str;
}

/**
 * @brief Convert a 16-bit value to a hexadecimal string.
 * @param val 16-bit value
 * @return Hex string in uppercase (e.g., "1A2B")
 */
inline std::string to_hex(uint16_t val) {
  char buf[7];
  snprintf(buf, sizeof(buf), "%04X", val);
  return std::string(buf);
}

/**
 * @brief Retrieve a scaled double value from Modbus registers.
 *
 * @details
 * This function reads one or more Modbus registers and applies the
 * corresponding scale factor to return a floating-point value. The scale factor
 * register is assumed to contain a signed integer exponent (10^SF). The number
 * of registers and their type (signed 16-bit, unsigned 16-bit, or unsigned
 * 32-bit) are indicated by the `Register` definition.
 *
 * @param regMap A vector containing the raw Modbus register values.
 * @param reg The register representing the value to read.
 * @param sf The register representing the associated scale factor.
 *
 * @return On success, returns the scaled double value.
 *         On failure, returns a ModbusError indicating why the value could not
 *         be retrieved, e.g., unsupported register type or invalid register
 * size.
 */
inline std::expected<double, ModbusError>
modbus_get_double(std::vector<uint16_t> &regMap, const Register &reg,
                  const Register &sf) {
  const auto scale = std::pow(10.0, static_cast<int16_t>(regMap[sf.ADDR]));

  switch (reg.TYPE) {
  case RegType::INT16:
    return static_cast<double>(static_cast<int16_t>(regMap[reg.ADDR])) * scale;
  case RegType::UINT16:
    return static_cast<double>(regMap[reg.ADDR]) * scale;
  case RegType::UINT32:
    return static_cast<double>(modbus_get_uint32(regMap.data() + reg.ADDR)) *
           scale;
  default:
    return std::unexpected(ModbusError::custom(
        EINVAL, "Unsupported register type for double conversion"));
  }
}

} // namespace modbus_utils

#endif /* MODBUS_UTILS_H_ */
