#ifndef REGISTER_BASE_H_
#define REGISTER_BASE_H_

#include <cstdint>
#include <format>
#include <string>

/**
 * @struct Register
 * @brief Describes one Modbus register or a contiguous register range.
 *
 * Each `Register` carries a starting address (`ADDR`), the number of
 * consecutive 16-bit registers (`NB`) used to store the value, and a
 * type (`TYPE`) that tells decoding helpers how to interpret the bytes.
 */
struct Register {
  /**
   * @enum Type
   * @brief Tags the encoding of a register value.
   */
  enum class Type : uint8_t {
    UINT16, /**< 16-bit unsigned integer */
    INT16,  /**< 16-bit signed integer */
    UINT32, /**< 32-bit unsigned integer (two consecutive 16-bit registers) */
    INT32,  /**< 32-bit signed integer (two consecutive 16-bit registers) */
    UINT64, /**< 64-bit unsigned integer (four consecutive 16-bit registers) */
    FLOAT,  /**< 32-bit IEEE 754 single-precision */
    STRING, /**< ASCII string spanning multiple 16-bit registers */
    UNKNOWN /**< Type not specified or unrecognised */
  };

  uint16_t ADDR{0};         /**< Modbus register address. */
  uint16_t NB{0};           /**< Number of consecutive 16-bit registers. */
  Type TYPE{Type::UNKNOWN}; /**< Encoding of the register value. */

  /**
   * @brief Construct a register definition.
   * @param addr Starting register address.
   * @param nb Number of consecutive 16-bit registers.
   * @param type Encoding of the value.
   */
  constexpr Register(uint16_t addr, uint16_t nb, Type type)
      : ADDR(addr), NB(nb), TYPE(type) {}

  /** @brief Default-construct a register: addr 0, width 0, `Type::UNKNOWN`. */
  constexpr Register() = default;

  /**
   * @brief Return a copy of this register with `offset` added to the address.
   * @param offset Offset to add to `ADDR` (may be negative).
   */
  [[nodiscard]] constexpr Register withOffset(int16_t offset) const {
    return Register{static_cast<uint16_t>(ADDR + offset), NB, TYPE};
  }

  /**
   * @brief Convert a `Type` value to its name as a C-string.
   */
  static constexpr const char *typeToString(Type type) {
    switch (type) {
    case Type::UINT16:
      return "UINT16";
    case Type::INT16:
      return "INT16";
    case Type::UINT32:
      return "UINT32";
    case Type::INT32:
      return "INT32";
    case Type::UINT64:
      return "UINT64";
    case Type::FLOAT:
      return "FLOAT";
    case Type::STRING:
      return "STRING";
    case Type::UNKNOWN:
      return "UNKNOWN";
    default:
      return "INVALID";
    }
  }

  /**
   * @brief Format the register as `"[ADDR=..., NB=..., TYPE=...]"`.
   *
   * Useful for logging and error messages.
   */
  std::string describe() const {
    return std::format("[ADDR={}, NB={}, TYPE={}]", ADDR, NB,
                       typeToString(TYPE));
  }
};

#endif /* REGISTER_BASE_H_ */
