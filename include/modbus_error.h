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
  /**
   * @brief Error severity classification.
   *
   * `RECONNECT` is declared last, out of severity order, deliberately:
   * `ModbusError` is header-only, so these values are compiled into every
   * consumer. Inserting it mid-enum would renumber `FATAL` and `SHUTDOWN`
   * and silently change their meaning for any binary built against an older
   * header. Nothing orders severities — all comparisons are `==` — so the
   * declaration order carries no meaning beyond the numbering.
   */
  enum class Severity {
    TRANSIENT, /**< Temporary error — may succeed on retry. */
    FATAL,     /**< Fatal error — requires intervention. */
    SHUTDOWN,  /**< Operation interrupted by a shutdown signal. */
    RECONNECT  /**< Endpoint lost — retry needs a fresh connection. */
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
   * mapped to `FATAL`; codes meaning the endpoint is gone map to
   * `RECONNECT`; everything else is treated as `TRANSIENT`.
   *
   * `RECONNECT` ranks between the two: retrying the same descriptor can
   * never succeed, but the fault is routine and needs no intervention
   * (`FATAL` exits the process via the application's error callback). It is
   * declared last in the enum for ABI reasons; see `Severity`.
   *
   * It is transport-independent. `ECONNRESET` reaches RTU too, since
   * libmodbus sets it when `recv()` returns 0 and the RTU `recv()` is a
   * `read()` on the tty; with `O_NONBLOCK` and a preceding `select()`, a
   * zero return can only be a hangup. `ETIMEDOUT` is excluded: it is the
   * ordinary silent-slave transient, and a dropped peer follows it with
   * `ECONNRESET`/`EPIPE`.
   *
   * `EIO`, `ENODEV` and `EBADF` are `RECONNECT`, not `TRANSIENT`. On an
   * established descriptor they mean the device is gone — an unplugged
   * USB-to-serial adapter fails reads with `EIO`, not with EOF — and every
   * retry on that descriptor then fails instantly forever. They are not
   * `FATAL` either: a reconnect almost always recovers, so tearing down the
   * process would be wrong.
   *
   * `ENOENT` stays `TRANSIENT`. It can only come from `open()`, so it means
   * the connection was never established rather than lost, and Phase 1
   * retries it with backoff whatever the severity. Keeping it `TRANSIENT`
   * also preserves the visible warning each cycle for a mistyped device
   * path, rather than a silent exit during a boot-time enumeration race.
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
    // Endpoint lost: only a fresh connect() recovers.
    case EPIPE:        // Broken pipe (wrote to an endpoint the peer closed)
    case ECONNRESET:   // Connection reset by peer; also a hung-up tty (EOF)
    case ENOTCONN:     // Transport endpoint is not connected
    case ESHUTDOWN:    // Cannot send after transport endpoint shutdown
    case ECONNABORTED: // Software caused connection abort
    case EIO:          // I/O error: serial device unplugged or tty hung up
    case ENODEV:       // Device removed
    case EBADF:        // Descriptor no longer valid
    case ENETDOWN:     // Network is down
    case ENETRESET:    // Connection aborted by the network
    case ENETUNREACH:  // Network is unreachable
    case EHOSTUNREACH: // No route to host
    case EHOSTDOWN:    // Host is down
      return Severity::RECONNECT;
    case EINTR: // Call was interrupted by a signal
      return Severity::SHUTDOWN;
    default:
      // Connect-time transients (ENOENT) and everything else (ETIMEDOUT,
      // EPROTO, ...) land here. See the function-level comment.
      return Severity::TRANSIENT;
    }
  }
};

#endif /* MODBUS_ERROR_H_ */