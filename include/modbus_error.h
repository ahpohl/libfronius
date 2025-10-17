/**
 * @file modbus_error.h
 * @brief Defines Modbus error representation and severity classification.
 *
 * @details
 * Provides a struct to encapsulate Modbus errors, including the numeric error
 * code, human-readable message, and severity (transient vs. fatal). Factory
 * methods allow creation from system errno or custom error codes, and automatic
 * translation via modbus_strerror() ensures clear diagnostic output.
 */

#ifndef MODBUS_ERROR_H_
#define MODBUS_ERROR_H_

#include <cerrno>
#include <format>
#include <modbus/modbus.h>
#include <string>

/**
 * @struct ModbusError
 * @brief Encapsulates a Modbus error with code, message, and severity.
 *
 * @details
 * This struct standardizes error handling for Modbus operations.
 * The severity indicates whether an error is transient (retryable) or fatal
 * (requires intervention). Factory methods support creation from errno or
 * explicit codes, and the `toString()` method combines both context and
 * Modbus-specific information in human-readable form.
 */
struct ModbusError {
public:
  /** @brief Error severity classification. */
  enum class Severity {
    TRANSIENT, /**< Temporary error — may succeed on retry. */
    FATAL      /**< Fatal error — requires intervention. */
  };

  /** @brief Modbus or system error code (as set in `errno`). */
  int code;

  /** @brief Contextual human-readable message (e.g. "Receive register 40329
   * failed"). */
  std::string message;

  /** @brief Classified severity of the error. */
  Severity severity;

  /**
   * @brief Create a ModbusError from the current system @c errno using a plain
   * message.
   *
   * This overload should be used when no formatting is required.
   * The message is stored as-is in the resulting ModbusError.
   *
   * @param msg Context message describing the error.
   * @return A ModbusError instance with @c code = errno and a severity deduced
   * via @c deduceSeverity().
   *
   * @see fromErrno(std::format_string<Args...>, Args&&...)
   * @see custom(int, const std::string&)
   *
   * @code
   * auto err = ModbusError::fromErrno("Failed to connect to Modbus device");
   * @endcode
   */
  static ModbusError fromErrno(const std::string &msg) {
    return {errno, msg, deduceSeverity(errno)};
  }

  /**
   * @brief Create a ModbusError from the current system @c errno using a
   * formatted message.
   *
   * This overload supports C++23-style @c std::format syntax for type-safe,
   * compile-time-checked formatting. The resulting message is formatted
   * according to the provided format string and arguments.
   *
   * @tparam Args Argument types deduced from the format string.
   * @param fmt Format string with {} placeholders (validated at compile time).
   * @param args Arguments to substitute into the format string.
   * @return A ModbusError instance with @c code = errno and a severity deduced
   * via @c deduceSeverity().
   *
   * @see fromErrno(const std::string&)
   * @see custom(int, std::format_string<Args...>, Args&&...)
   *
   * @code
   * auto err = ModbusError::fromErrno("Failed to read register {} from {}",
   * 40261, "inverter-1");
   * @endcode
   */
  template <typename... Args>
  static ModbusError fromErrno(std::format_string<Args...> fmt,
                               Args &&...args) {
    return {errno, std::format(fmt, std::forward<Args>(args)...),
            deduceSeverity(errno)};
  }

  /**
   * @brief Create a ModbusError with a custom error code and a plain message.
   *
   * This overload is used when the error code is not derived from @c errno,
   * but is manually provided by the caller. The message is stored as-is.
   *
   * @param c Custom error code.
   * @param msg Context message describing the error.
   * @return A ModbusError instance with the given code and deduced severity.
   *
   * @see custom(int, std::format_string<Args...>, Args&&...)
   * @see fromErrno(const std::string&)
   *
   * @code
   * auto err = ModbusError::custom(1234, "Invalid Modbus address");
   * @endcode
   */
  static ModbusError custom(int c, const std::string &msg) {
    return {c, msg, deduceSeverity(c)};
  }

  /**
   * @brief Create a ModbusError with a custom error code and a formatted
   * message.
   *
   * This overload supports C++23-style @c std::format syntax for compile-time
   * checked formatting. The formatted message is constructed with the provided
   * format string and arguments.
   *
   * @tparam Args Argument types deduced from the format string.
   * @param code Custom error code.
   * @param fmt Format string with {} placeholders (validated at compile time).
   * @param args Arguments to substitute into the format string.
   * @return A ModbusError instance with the given code and deduced severity.
   *
   * @see custom(int, const std::string&)
   * @see fromErrno(std::format_string<Args...>, Args&&...)
   *
   * @code
   * auto err = ModbusError::custom(1002, "Register {} invalid for {}", 40001,
   * "hybrid inverter");
   * @endcode
   */
  template <typename... Args>
  static ModbusError custom(int code, std::format_string<Args...> fmt,
                            Args &&...args) {
    return {code, std::format(fmt, std::forward<Args>(args)...),
            deduceSeverity(code)};
  }

private:
  /**
   * @brief Deduce severity based on the error code.
   * @param c Error code (errno or custom).
   * @return Severity::FATAL for unrecoverable errors; otherwise TRANSIENT.
   */
  static Severity deduceSeverity(int c) {
    switch (c) {
    case EINVAL:    // Invalid argument
    case ENOMEM:    // Out of memory
    case EMBMDATA:  // Too many registers requested
    case EMBXILFUN: // Illegal function
    case EMBXILADD: // Illegal data address
    case EMBXILVAL: // Illegal data value
    case EMBXSFAIL: // Slave device or server failure
    case EMBXGTAR:  // Gateway target device failed to respond
      return Severity::FATAL;
    default:
      return Severity::TRANSIENT;
    }
  }
};

#endif /* MODBUS_ERROR_H_ */