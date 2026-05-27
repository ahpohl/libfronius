/**
 * @file modbus_error.h
 * @brief Modbus error type with severity classification.
 *
 * @details
 * `ModbusError` carries a numeric error code (errno or libmodbus code), a
 * formatted context message, and a severity (`TRANSIENT`, `FATAL`, or
 * `SHUTDOWN`). Factory methods build instances from `errno` or from a
 * caller-supplied code, with `std::format`-based message templates.
 */

#ifndef MODBUS_ERROR_H_
#define MODBUS_ERROR_H_

#include <cerrno>
#include <expected>
#include <format>
#include <modbus.h>
#include <string>

/**
 * @struct ModbusError
 * @brief Encapsulates a Modbus error with code, message, and severity.
 *
 * Used as the error type of `std::expected<T, ModbusError>` throughout
 * libfronius. Severity is derived from the error code by `deduceSeverity()`.
 */
struct ModbusError {
public:
  /** @brief Error severity classification. */
  enum class Severity {
    TRANSIENT, /**< Temporary error — may succeed on retry. */
    FATAL,     /**< Fatal error — requires intervention. */
    SHUTDOWN   /**< Operation interrupted by a shutdown signal. */
  };

  /** @brief Modbus or system error code (as set in `errno`). */
  int code;

  /** @brief Contextual human-readable message. */
  std::string message;

  /** @brief Classified severity of the error. */
  Severity severity;

  /**
   * @brief Create a ModbusError from the current `errno` with a plain message.
   * @param msg Context message.
   */
  static ModbusError fromErrno(const std::string &msg) {
    return {errno, msg, deduceSeverity(errno)};
  }

  /**
   * @brief Create a ModbusError from the current `errno` with a formatted
   *        message.
   *
   * @tparam Args Argument types deduced from the format string.
   * @param fmt   `std::format` string (validated at compile time).
   * @param args  Arguments substituted into `fmt`.
   *
   * @code
   * auto err = ModbusError::fromErrno("read register {} on slave {}",
   *                                   40261, 1);
   * @endcode
   */
  template <typename... Args>
  static ModbusError fromErrno(std::format_string<Args...> fmt,
                               Args &&...args) {
    return {errno, std::format(fmt, std::forward<Args>(args)...),
            deduceSeverity(errno)};
  }

  /**
   * @brief Create a ModbusError with an explicit code and a plain message.
   * @param c   Error code (e.g. an `errno`-like value or libmodbus code).
   * @param msg Context message.
   */
  static ModbusError custom(int c, const std::string &msg) {
    return {c, msg, deduceSeverity(c)};
  }

  /**
   * @brief Create a ModbusError with an explicit code and a formatted message.
   *
   * @tparam Args Argument types deduced from the format string.
   * @param code  Error code.
   * @param fmt   `std::format` string (validated at compile time).
   * @param args  Arguments substituted into `fmt`.
   */
  template <typename... Args>
  static ModbusError custom(int code, std::format_string<Args...> fmt,
                            Args &&...args) {
    return {code, std::format(fmt, std::forward<Args>(args)...),
            deduceSeverity(code)};
  }

  /**
   * @brief Unwrap an expected, throwing the contained ModbusError on failure.
   *
   * Convenience helper for code paths where throwing is acceptable.
   *
   * @tparam T  Value type contained in the expected.
   * @param res The expected to unwrap.
   * @return The unwrapped value (no return for `T = void`).
   * @throws ModbusError if `res` contains an error.
   */
  template <typename T> static T getOrThrow(std::expected<T, ModbusError> res) {
    if (!res)
      throw res.error();

    if constexpr (!std::is_void_v<T>) {
      return *res;
    }
  }

  /**
   * @brief Format the error as `"<message>: <libmodbus_text> (code <n>)"`.
   *
   * `<libmodbus_text>` comes from `modbus_strerror(code)`.
   */
  std::string describe() const {
    return std::format("{}: {} (code {})", message, modbus_strerror(code),
                       code);
  }

private:
  /**
   * @brief Map an error code to a `Severity`.
   *
   * `EINTR` is mapped to `SHUTDOWN` (used to unwind blocking calls during
   * shutdown). A fixed list of well-known fatal `errno`/libmodbus codes is
   * mapped to `FATAL`; everything else is treated as `TRANSIENT`.
   *
   * Note on transport hiccups: `ENOENT`, `ENODEV`, `EBADF`, and `EIO` are
   * deliberately *not* in the FATAL list. On an RTU bus these are the
   * typical errno values when a USB-to-serial adapter momentarily
   * disappears (device unplug, kernel re-enumeration, udev settle race,
   * adapter firmware glitch). Classifying them as FATAL would tear down
   * the bus thread and, via `onBusError` callbacks at the application
   * layer, the whole process — even though a simple reconnect almost
   * always recovers. `busLoop()`'s Phase 1 already implements that
   * reconnect with exponential backoff, so TRANSIENT is the correct
   * classification.
   *
   * A genuinely-misconfigured RTU device path (typo in YAML) also
   * surfaces as `ENOENT`, but here TRANSIENT is still the right call: it
   * produces a repeated, visible warning every backoff cycle rather than
   * a silent startup exit during a boot-time enumeration race.
   */
  static Severity deduceSeverity(int c) {
    switch (c) {
    case EINVAL:       // Invalid argument
    case ENODATA:      // No data available
    case ENOMEM:       // Out of memory
    case EMBMDATA:     // Too many registers requested
    case EMBXILFUN:    // Illegal function
    case EMBXILADD:    // Illegal data address
    case EMBXILVAL:    // Illegal data value
    case EMBXSFAIL:    // Slave device or server failure
    case EMBXGTAR:     // Gateway target device failed to respond
    case ENXIO:        // No such device or address
    case EACCES:       // Permission denied
    case EPERM:        // Operation not permitted
    case ENOTDIR:      // Not a directory
    case EISDIR:       // Is a directory
    case ENAMETOOLONG: // File name too long
    case ELOOP:        // Too many symbolic links
    case EMFILE:       // Process limit for file descriptors reached
    case ENFILE:       // System-wide file descriptor table full
    case ENOTTY:       // Not a terminal
    case EAGAIN:       // Resource temporarily unavailable
    case EBUSY:        // Device or resource busy
    case EADDRINUSE:   // Address already in use
    case ENOTSUP:      // Not supported
      return Severity::FATAL;
    case EINTR: // Call was interrupted by a signal
      return Severity::SHUTDOWN;
    default:
      // Transport-hiccup transients (ENOENT, ENODEV, EBADF, EIO) and
      // everything else (ETIMEDOUT, ECONNRESET, EPIPE, EPROTO, ...) land
      // here. See the function-level comment for the rationale.
      return Severity::TRANSIENT;
    }
  }
};

#endif /* MODBUS_ERROR_H_ */