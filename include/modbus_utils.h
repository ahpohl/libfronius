/**
 * @file modbus_utils.h
 * @brief Utility functions for working with Modbus registers and data.
 *
 * @details
 * Provides helper functions to extract integer, floating-point, and string
 * values from Modbus register arrays, handle byte and word swaps, and
 * convert values to hex strings. Also provides packToModbus() for encoding
 * typed values and scaled floating-point values into Modbus register mappings.
 *
 * These functions are commonly used with Fronius SunSpec Modbus maps.
 */

#ifndef MODBUS_UTILS_H_
#define MODBUS_UTILS_H_

#include "fronius_types.h"
#include "modbus_error.h"
#include <arpa/inet.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <modbus/modbus.h>
#include <netinet/in.h>
#include <string>

/**
 * @namespace detail
 * @brief Internal implementation helpers — do not use directly.
 *
 * @details
 * Contains low-level packing primitives used by the ModbusUtils public API.
 * These functions are not part of the stable interface and may change without
 * notice.
 */
namespace detail {

/**
 * @brief Pack an integral value into consecutive 16-bit Modbus registers in
 * big-endian word order.
 *
 * @details
 * Splits @p value into 16-bit words, byte-swaps each word from native
 * little-endian to big-endian, and writes them into @p dest in most-significant
 * word-first order, matching the SunSpec / Modbus-over-TCP wire format.
 *
 * This is an internal helper. Callers should use ModbusUtils::packToModbus
 * instead.
 *
 * @tparam T  Integral type to encode (e.g. @c uint32_t, @c uint64_t).
 *            Must satisfy @c std::is_integral_v<T>.
 * @param dest  Pointer to the first of @c sizeof(T)/2 consecutive
 *              @c uint16_t registers to write into. Must not be null and must
 *              have room for @c sizeof(T)/2 words.
 * @param value Value to encode.
 */
template <typename T> void packInteger(uint16_t *dest, T value) {
  static_assert(std::is_integral_v<T>, "T must be an integral type");

  constexpr int nBytes = sizeof(T);
  constexpr int nWords = nBytes / 2;

  unsigned char buf[nBytes];
  std::memcpy(buf, &value, nBytes);

  for (int i = 0; i < nWords; i++) {
    unsigned char hi = buf[2 * i + 1];
    unsigned char lo = buf[2 * i];
    dest[i] = (static_cast<uint16_t>(hi) << 8) | lo;
  }

  std::reverse(dest, dest + nWords);
}

}; // namespace detail

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

/**
 * @brief Retrieves the remote peer's IP address and port from a connected
 * socket.
 *
 * @details
 * Extracts peer connection information from a socket file descriptor,
 * supporting both IPv4 and IPv6 protocols. Protocol-independent and works
 * for both client (master) and server (slave) socket roles.
 *
 * @param socket Connected socket file descriptor (e.g., returned by accept()
 * or after a successful connect()).
 *
 * @return A FroniusTypes::RemoteEndpoint with:
 *         - @c ip:   Remote IP address as a string (dotted-decimal IPv4 or
 *                    colon-separated IPv6 format).
 *         - @c port: Remote port number as an integer.
 *         Returns {"unknown", 0} if the socket is invalid or peer information
 *         cannot be retrieved.
 */
inline FroniusTypes::RemoteEndpoint getSocketInfo(int socket) {
  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);

  if (getpeername(socket, (struct sockaddr *)&addr, &addr_len) != 0) {
    return {"unknown", 0};
  }

  char ip_str[INET6_ADDRSTRLEN];
  int port = 0;

  if (addr.ss_family == AF_INET) {
    // IPv4
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
    inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, sizeof(ip_str));
    port = ntohs(addr_in->sin_port);
  } else if (addr.ss_family == AF_INET6) {
    // IPv6
    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)&addr;
    inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip_str, sizeof(ip_str));
    port = ntohs(addr_in6->sin6_port);
  } else {
    return {"unknown", 0};
  }

  return {ip_str, port};
}

// --- pack integer, string and float values into Modbus registers ---

/**
 * @brief Write a typed value into a Modbus register mapping.
 *
 * @details
 * Encodes @p value into the register(s) starting at @c reg.ADDR inside
 * @p dest, using the encoding implied by @c reg.TYPE:
 *
 * | Register::Type | Accepted T            | Encoding                        |
 * |----------------|-----------------------|---------------------------------|
 * | INT16          | integral              | cast to int16_t, stored as 1 register |
 * | UINT16         | integral              | stored as 1 register            |
 * | UINT32         | integral              | big-endian word order, 2 registers |
 * | UINT64         | integral              | big-endian word order, 4 registers |
 * | FLOAT          | floating-point        | IEEE 754 ABCD byte order, 2 registers |
 * | STRING         | std::string           | ASCII, high byte first per register, zero-padded to reg.NB registers |
 *
 * The template is intentionally permissive — mismatched combinations (e.g.
 * passing a float for an INT16 register) compile but are no-ops at runtime.
 * Prefer using the correct type for the register's declared type.
 *
 * @tparam T  Type of the value to encode. Must be integral, floating-point,
 *            or @c std::string, matching the register's declared type.
 * @param dest  Target Modbus mapping. Must not be null.
 * @param reg   Register definition specifying the address, width, and type.
 * @param value Value to encode.
 *
 * @return An empty expected on success, or an unexpected ModbusError if:
 *         - @p dest or its @c tab_registers pointer is null (@c EINVAL)
 *         - @c reg.TYPE is UNKNOWN or unsupported (@c EINVAL)
 *         - A STRING value exceeds the register's declared capacity (@c EINVAL)
 */
template <typename T>
std::expected<void, ModbusError> packToModbus(modbus_mapping_t *dest,
                                              Register reg, T value) {

  if (!dest) {
    return std::unexpected(
        ModbusError::custom(EINVAL, "Null modbus_mapping_t pointer"));
  }
  if (!dest->tab_registers) {
    return std::unexpected(
        ModbusError::custom(EINVAL, "modbus_mapping_t has null tab_registers"));
  }

  switch (reg.TYPE) {
  case Register::Type::INT16:
    if constexpr (std::is_integral_v<T>)
      dest->tab_registers[reg.ADDR] = static_cast<uint16_t>(value);
    break;
  case Register::Type::UINT16:
    if constexpr (std::is_integral_v<T>)
      dest->tab_registers[reg.ADDR] = value;
    break;
  case Register::Type::UINT32:
    if constexpr (std::is_integral_v<T>)
      detail::packInteger<uint32_t>(&dest->tab_registers[reg.ADDR], value);
    break;
  case Register::Type::UINT64:
    if constexpr (std::is_integral_v<T>)
      detail::packInteger<uint64_t>(&dest->tab_registers[reg.ADDR], value);
    break;
  case Register::Type::FLOAT:
    if constexpr (std::is_floating_point_v<T>)
      modbus_set_float_abcd(static_cast<float>(value),
                            &dest->tab_registers[reg.ADDR]);
    break;
  case Register::Type::STRING: {
    if constexpr (std::is_same_v<T, std::string>) {
      size_t maxLength = reg.NB * 2;
      if (value.length() > maxLength) {
        return std::unexpected(ModbusError::custom(
            EINVAL,
            "String length {} exceeds maximum {} characters for register {}",
            value.length(), maxLength, reg.describe()));
      }

      // Pack string into modbus mapping
      for (size_t i = 0; i < value.length() / 2; i++) {
        unsigned char hi = value[2 * i];
        unsigned char lo = value[2 * i + 1];
        dest->tab_registers[reg.ADDR + i] =
            (static_cast<uint16_t>(hi) << 8) | lo;
      }
      if (value.length() % 2) {
        dest->tab_registers[reg.ADDR + value.length() / 2] =
            (static_cast<uint16_t>(value[value.length() - 1]) << 8);
      }

      // Zero out remaining registers
      for (size_t i = (value.length() + 1) / 2; i < static_cast<size_t>(reg.NB);
           i++) {
        dest->tab_registers[reg.ADDR + i] = 0;
      }
    }
    break;
  }
  case Register::Type::UNKNOWN:
  default:
    return std::unexpected(ModbusError::custom(
        EINVAL, std::format("Unsupported target register type: {}",
                            Register::typeToString(reg.TYPE))));
  }

  return {};
}

/**
 * @brief Encode a floating-point value into an integer register plus a SunSpec
 * scale-factor register.
 *
 * @details
 * Scales @p realValue by 10^@p decimals, rounds to the nearest integer, and
 * writes the result into the register at @c reg.ADDR. The corresponding scale
 * factor register at @c sf.ADDR is written as @c -decimals (a signed INT16),
 * following the SunSpec convention where the reconstructed value equals
 * @c register_value × 10^sf.
 *
 * Supported register types for @p reg: INT16, UINT16, UINT32, UINT64.
 *
 * Example — storing 230.5 V with 1 decimal place into a UINT16 voltage
 * register and its SF register:
 * @code
 *   packToModbus(mapping, M20X::PHVPHA, M20X::V_SF, 230.5, 1);
 *   // tab_registers[PHVPHA.ADDR] = 2305
 *   // tab_registers[V_SF.ADDR]   = 0xFFFF  (-1 as INT16)
 * @endcode
 *
 * @param dest      Target Modbus mapping. Must not be null.
 * @param reg       Register definition for the scaled integer value.
 * @param sf        Register definition for the INT16 scale factor.
 * @param realValue The physical value to encode (e.g. watts, volts).
 * @param decimals  Number of decimal places to preserve (i.e. the negated
 *                  SunSpec scale factor). Must be ≥ 0.
 *
 * @return An empty expected on success, or an unexpected ModbusError if:
 *         - @p dest or its @c tab_registers pointer is null (@c EINVAL)
 *         - @c reg.TYPE is unsupported (@c EINVAL)
 */
inline std::expected<void, ModbusError> packToModbus(modbus_mapping_t *dest,
                                                     Register reg, Register sf,
                                                     double realValue,
                                                     int decimals) {
  if (!dest) {
    return std::unexpected(
        ModbusError::custom(EINVAL, "Null modbus_mapping_t pointer"));
  }
  if (!dest->tab_registers) {
    return std::unexpected(
        ModbusError::custom(EINVAL, "modbus_mapping_t has null tab_registers"));
  }

  // encode float into integer register
  switch (reg.TYPE) {
  case Register::Type::INT16: {
    int16_t value =
        static_cast<int16_t>(std::round(realValue * std::pow(10, decimals)));
    dest->tab_registers[reg.ADDR] = static_cast<uint16_t>(value);
    break;
  }
  case Register::Type::UINT16: {
    uint16_t value =
        static_cast<uint16_t>(std::round(realValue * std::pow(10, decimals)));
    dest->tab_registers[reg.ADDR] = value;
    break;
  }

  case Register::Type::UINT32: {
    uint32_t value =
        static_cast<uint32_t>(std::round(realValue * std::pow(10, decimals)));
    detail::packInteger<uint32_t>(&dest->tab_registers[reg.ADDR], value);
    break;
  }
  case Register::Type::UINT64: {
    uint64_t value =
        static_cast<uint64_t>(std::round(realValue * std::pow(10, decimals)));
    detail::packInteger<uint64_t>(&dest->tab_registers[reg.ADDR], value);
    break;
  }

  default:
    return std::unexpected(ModbusError::custom(
        EINVAL,
        std::format("Unsupported target register type for encoding float: {}",
                    Register::typeToString(reg.TYPE))));
  }

  // encode scale factor (scale factors are INT16 in SunSpec)
  int16_t sf_value = -static_cast<int16_t>(decimals);
  dest->tab_registers[sf.ADDR] = static_cast<uint16_t>(sf_value);

  return {};
}

}; // namespace ModbusUtils

#endif /* MODBUS_UTILS_H_ */
