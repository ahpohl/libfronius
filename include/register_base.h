#ifndef REGISTER_BASE_H_
#define REGISTER_BASE_H_

#include <cstdint>

/**
 * @enum RegType
 * @brief Enumerates the possible types of values stored in Modbus registers.
 *
 * @details
 * This enum allows the code to explicitly distinguish between different kinds
 * of register contents, such as signed vs. unsigned integers, floating-point
 * values, strings, or enumerations. It improves type safety and makes functions
 * like `modbus_get_double` more robust and readable.
 */
enum class RegType : uint8_t {
  UINT16, /**< 16-bit unsigned integer */
  INT16,  /**< 16-bit signed integer */
  UINT32, /**< 32-bit unsigned integer (two consecutive 16-bit registers) */
  UINT64, /**< 64-bit unsigned integer (four consecutive 16-bit registers) */
  FLOAT,  /**< 32-bit floating-point value (single-precision) */
  STRING, /**< ASCII string stored across multiple 16-bit registers */
  UNKNOWN /**< Type not specified or unknown */
};

/**
 * @struct Register
 * @brief Represents a Modbus register definition.
 *
 * @details
 * Each instance of `Register` specifies the address (`ADDR`), the number of
 * consecutive 16-bit registers (`NB`) used to store the value, and the type
 * of data (`TYPE`). This allows higher-level functions to interpret raw Modbus
 * data correctly, whether it is integer, floating-point, or string data.
 *
 * @note When working with scaled values, the `TYPE` field helps determine
 * whether the register should be interpreted as signed or unsigned.
 */
struct Register {
  uint16_t ADDR{0}; /**< Modbus register address (0-based or device-specific) */
  uint16_t NB{0};   /**< Number of consecutive registers used by this value */
  RegType TYPE{RegType::UNKNOWN}; /**< Type of value stored in the register */

  /**
   * @brief Construct a register definition.
   * @param addr Starting register address.
   * @param nb Number of consecutive 16-bit registers used for this value.
   * @param type Type of value stored in the register.
   */
  constexpr Register(uint16_t addr, uint16_t nb, RegType type)
      : ADDR(addr), NB(nb), TYPE(type) {}
};

#endif /* REGISTER_BASE_H_ */
