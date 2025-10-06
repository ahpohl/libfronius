/**
 * @file modbus_error.h
 * @brief Defines Modbus error representation and severity classification.
 *
 * @details
 * Provides a struct to encapsulate Modbus errors, including the error code,
 * human-readable message, and severity (transient vs. fatal). Factory methods
 * allow creating errors from system errno or custom error codes.
 */

#ifndef MODBUS_ERROR_H_
#define MODBUS_ERROR_H_

#include <cerrno>
#include <modbus/modbus.h>
#include <string>

/** @struct ModbusError
 *  @brief Encapsulates a Modbus error with code, message, and severity.
 *
 *  @details
 *  This struct standardizes error handling for Modbus operations. The severity
 *  indicates whether an error is transient (retryable) or fatal (requires
 *  intervention). Use the factory methods `fromErrno` and `custom` to
 *  conveniently create error instances.
 */
struct ModbusError {
  /** @brief Error severity type */
  enum class Severity {
    TRANSIENT, /**< Temporary error, can retry */
    FATAL      /**< Fatal error, requires intervention */
  };

  /** @brief Modbus or system error code */
  int code;

  /** @brief Human-readable error message */
  std::string message;

  /** @brief Severity of the error */
  Severity severity;

public:
  /**
   * @brief Create a ModbusError from the current errno.
   * @param msg Human-readable description of the context.
   * @return ModbusError instance with code = errno and deduced severity.
   */
  static ModbusError fromErrno(const std::string &msg) {
    return {errno, msg, deduceSeverity(errno)};
  }

  /**
   * @brief Create a ModbusError with a custom error code.
   * @param c Error code
   * @param msg Human-readable description
   * @return ModbusError instance with specified code and deduced severity
   */
  static ModbusError custom(int c, const std::string &msg) {
    return {c, msg, deduceSeverity(c)};
  }

private:
  /**
   * @brief Deduce severity based on error code.
   * @param c Error code (errno or custom)
   * @return Severity::FATAL for unrecoverable errors, otherwise
   * Severity::TRANSIENT
   */
  static Severity deduceSeverity(int c) {
    switch (c) {
    case EINVAL:   // Invalid argument
    case ENOMEM:   // Out of memory
    case EMBMDATA: // Too many registers requested
      return Severity::FATAL;
    default:
      return Severity::TRANSIENT;
    }
  }
};

#endif /* MODBUS_ERROR_H_ */
