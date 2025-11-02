/**
 * @file modbus_error.h
 * @brief Defines Modbus error representation and severity classification.
 *
 * @details
 * Provides a struct to encapsulate Modbus errors, including the numeric error
 * code, contextual message, a precomputed libmodbus error string, and severity
 * (transient vs. fatal). Factory methods allow creation from system errno or
 * custom error codes. The libmodbus error text is computed inside the library
 * at creation time, so consumers do not need to link against libmodbus.
 */

#ifndef MODBUS_ERROR_H_
#define MODBUS_ERROR_H_

#include <cerrno>
#include <expected>
#include <format>
#include <modbus/modbus.h>
#include <string>
#include <utility>

/**
 * @struct ModbusError
 * @brief Encapsulates a Modbus error with code, message, and severity.
 *
 * @details
 * This struct standardizes error handling for Modbus operations.
 * The severity indicates whether an error is transient (retryable) or fatal
 * (requires intervention). Factory methods support creation from errno or
 * explicit codes. The libmodbus error text is precomputed at creation time
 * within the library and stored in this object.
 */
struct ModbusError {
public:
  /** @brief Error severity classification. */
  enum class Severity {
    TRANSIENT, /**< Temporary error — may succeed on retry. */
    FATAL      /**< Fatal error — requires intervention. */
  };

  /** @brief Modbus or system error code (as set in `errno`). */
  int code{0};

  /** @brief Contextual human-readable message (e.g. "Receive register 40329
   * failed"). */
  std::string message;

  /** @brief Classified severity of the error. */
  Severity severity{Severity::TRANSIENT};

  /** @brief Precomputed libmodbus textual description for @c code. */
  std::string libText;

  /**
   * @brief Create a ModbusError from the current system @c errno using a plain
   * message.
   */
  static ModbusError fromErrno(const std::string &msg) {
    return make(errno, msg);
  }

  /**
   * @brief Create a ModbusError from the current system @c errno using a
   * formatted message.
   */
  template <typename... Args>
  static ModbusError fromErrno(std::format_string<Args...> fmt,
                               Args &&...args) {
    return make(errno, std::format(fmt, std::forward<Args>(args)...));
  }

  /**
   * @brief Create a ModbusError with a custom error code and a plain message.
   */
  static ModbusError custom(int c, const std::string &msg) {
    return make(c, msg);
  }

  /**
   * @brief Create a ModbusError with a custom error code and a formatted
   * message.
   */
  template <typename... Args>
  static ModbusError custom(int code, std::format_string<Args...> fmt,
                            Args &&...args) {
    return make(code, std::format(fmt, std::forward<Args>(args)...));
  }

  /**
   * @brief Unwraps a std::expected<T, ModbusError> or throws the contained
   * ModbusError.
   */
  template <typename T> static T getOrThrow(std::expected<T, ModbusError> res) {
    if (res)
      return *res;
    throw res.error();
  }

  /**
   * @brief Convenience function combining context and libmodbus message.
   */
  std::string toString() const {
    return std::format("{}: {} (code {})", message, libText, code);
  }

private:
  /**
   * @brief Build a ModbusError, computing severity and libmodbus text
   * internally.
   */
  static ModbusError make(int code, std::string msg);

  /**
   * @brief Deduce severity based on the error code.
   */
  static Severity deduceSeverity(int c) {
    switch (c) {
    case EINVAL:    // Invalid argument
    case ENOMEM:    // Out of memory
    case ENOENT:    // No such file or directory
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