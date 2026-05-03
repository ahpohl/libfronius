/**
 * @file modbus_utils.h
 * @brief Utility helpers for Modbus register encoding and decoding.
 *
 * @details
 * Free functions that read 32- and 64-bit integers from register arrays
 * (with optional word/byte swap), produce hex strings, retrieve TCP peer
 * info, and pack typed or scaled-floating-point values into a
 * `modbus_mapping_t` (server-side write helpers).
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
 * @brief Internal implementation helpers — not part of the public API.
 */
namespace detail {

/**
 * @brief Pack an integral value into consecutive 16-bit Modbus registers
 *        in big-endian word order.
 *
 * Splits @p value into 16-bit words, byte-swaps each word from native
 * little-endian to big-endian, and writes them most-significant-word first
 * — the SunSpec / Modbus-over-TCP wire format.
 *
 * @tparam T  Integral type (e.g. `uint32_t`, `uint64_t`).
 * @param dest  Pointer to `sizeof(T)/2` consecutive registers.
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
 * @brief Get the remote IP and port of a connected socket.
 *
 * Works with IPv4 and IPv6, on either client or server sockets.
 *
 * @param socket  Connected socket file descriptor.
 * @return `{ip, port}` or `{"unknown", 0}` if peer info is unavailable.
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
 * Encodes @p value into the register(s) starting at `reg.ADDR` inside
 * `dest->tab_registers`, using the encoding implied by `reg.TYPE`:
 *
 * | reg.TYPE | Accepted T       | Encoding                                   |
 * |----------|------------------|--------------------------------------------|
 * | INT16    | integral         | cast to `int16_t`, 1 register              |
 * | UINT16   | integral         | 1 register                                 |
 * | UINT32   | integral         | big-endian word order, 2 registers         |
 * | UINT64   | integral         | big-endian word order, 4 registers         |
 * | FLOAT    | floating-point   | IEEE 754 ABCD byte order, 2 registers      |
 * | STRING   | `std::string`    | ASCII, hi-byte first, zero-padded to NB    |
 *
 * Mismatched type combinations (e.g. a float for an INT16 register) compile
 * but are silent no-ops at runtime — pass the type that matches `reg.TYPE`.
 *
 * @tparam T   Integral, floating-point, or `std::string`.
 * @param dest   Target Modbus mapping (must not be null).
 * @param reg    Register definition.
 * @param value  Value to encode.
 *
 * @return Empty on success, or `ModbusError` (`EINVAL`) if `dest` is null,
 *         `reg.TYPE` is unsupported, or a STRING value exceeds `reg.NB * 2`.
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
 * @brief Encode a floating-point value into an integer register plus a
 *        SunSpec scale-factor register.
 *
 * Scales `realValue` by `10^decimals`, rounds to the nearest integer, and
 * writes the result into `reg`. Writes `-decimals` (as INT16) into `sf`,
 * matching the SunSpec convention `value = register × 10^sf`.
 *
 * Supported `reg.TYPE`: `INT16`, `UINT16`, `UINT32`, `UINT64`.
 *
 * Example — store 230.5 V with 1 decimal place:
 * @code
 *   packToModbus(mapping, M20X::PHVPHA, M20X::V_SF, 230.5, 1);
 *   // tab_registers[PHVPHA.ADDR] = 2305
 *   // tab_registers[V_SF.ADDR]   = 0xFFFF  (-1 as INT16)
 * @endcode
 *
 * @param dest      Target mapping (must not be null).
 * @param reg       Register for the scaled integer value.
 * @param sf        Register for the INT16 scale factor.
 * @param realValue Physical value to encode.
 * @param decimals  Decimal places to preserve (≥ 0).
 *
 * @return Empty on success, or `ModbusError` (`EINVAL`) if `dest` is null
 *         or `reg.TYPE` is unsupported.
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
