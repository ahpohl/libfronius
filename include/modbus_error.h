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

public:
  /**
   * @brief Create a ModbusError from the current errno.
   * @param msg Context message for the error.
   * @return A ModbusError instance with code = errno and deduced severity.
   */
  static ModbusError fromErrno(const std::string &msg) {
    return {errno, msg, deduceSeverity(errno)};
  }

  /**
   * @brief Create a ModbusError with a custom error code.
   * @param c Error code.
   * @param msg Context message.
   * @return A ModbusError instance with deduced severity.
   */
  static ModbusError custom(int c, const std::string &msg) {
    return {c, msg, deduceSeverity(c)};
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