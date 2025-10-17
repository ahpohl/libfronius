#include "fronius.h"
#include "common_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <cerrno>
#include <cmath>
#include <expected>
#include <modbus/modbus.h>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

/* ------------------------------ public API -------------------------------*/

Fronius::Fronius(const ModbusConfig &cfg) : cfg_(cfg) {
  cfg.validate();
  regs_.resize(0xFFFF, 0);
}

Fronius::~Fronius() {
  {
    std::lock_guard<std::mutex> lock(mtx_);
    running_.store(false);
    cv_.notify_all();
  }

  if (connectionThread_.joinable())
    connectionThread_.join();

  if (ctx_) {
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }
}

std::expected<void, ModbusError> Fronius::connect() {
  running_.store(true);
  connectionThread_ = std::thread(&Fronius::connectionLoop, this);
  return {};
}

void Fronius::waitForConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cv_.wait(lock, [this]() { return connected_.load(); });
}

void Fronius::triggerReconnect() {
  std::lock_guard<std::mutex> lock(mtx_);

  if (!connected_.load())
    return; // already disconnected

  connected_.store(false);

  // Wake up connection loop to reconnect
  cv_.notify_all();
}

std::expected<std::string, ModbusError> Fronius::getManufacturer() {
  return getModbusString(regs_, C001::MN);
}

std::expected<std::string, ModbusError> Fronius::getDeviceModel() {
  return getModbusString(regs_, C001::MD);
}

std::expected<std::string, ModbusError> Fronius::getOptions() {
  return getModbusString(regs_, C001::OPT);
}

std::expected<std::string, ModbusError> Fronius::getFwVersion() {
  return getModbusString(regs_, C001::VR);
}

std::expected<std::string, ModbusError> Fronius::getSerialNumber() {
  return getModbusString(regs_, C001::SN);
}

std::expected<uint16_t, ModbusError> Fronius::getModbusDeviceAddress() {
  uint16_t val = regs_[C001::DA.ADDR];

  if ((val < 1) || (val > 247))
    return reportError<uint16_t>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid Modbus slave address: received {}, expected 1-247",
        val)));
  return val;
}

/* ------------------------- protected methods ----------------------------*/

std::expected<bool, ModbusError> Fronius::validateSunSpecRegisters(void) {
  if (!ctx_) {
    return reportError<bool>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  // Get registers
  int rc = modbus_read_registers(ctx_, C001::SID.ADDR, 4,
                                 regs_.data() + C001::SID.ADDR);
  if (rc == -1) {
    return reportError<bool>(std::unexpected(
        ModbusError::fromErrno("Receive register {} failed", C001::SID.ADDR)));
  }

  // Test for "SunS" string
  if (!(regs_[C001::SID.ADDR] == 0x5375 &&
        regs_[C001::SID.ADDR + 1] == 0x6e53)) {
    return reportError<bool>(std::unexpected(
        ModbusError::custom(EINVAL,
                            "SunSpec signature mismatch: expected [0x5375, "
                            "0x6e53], received [0x{}, 0x{}]",
                            ModbusUtils::toHex(regs_[C001::SID.ADDR]),
                            ModbusUtils::toHex(regs_[C001::SID.ADDR + 1]))));
  }

  // Test for Common Register ID
  if (regs_[C001::ID.ADDR] != 0x1) {
    return reportError<bool>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid common register map ID: received {}, expected 1",
        regs_[C001::ID.ADDR])));
  }

  // Test for Common Register Map Length
  if (regs_[C001::L.ADDR] != C001::SIZE) {
    return reportError<bool>(std::unexpected(ModbusError::custom(
        EINVAL, "Invalid common register map size: received {}, expected {}",
        regs_[C001::L.ADDR], C001::SIZE)));
  }

  return {};
}

std::expected<void, ModbusError> Fronius::fetchCommonRegisters(void) {
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOTCONN, "Modbus context is null")));
  }

  int rc = modbus_read_registers(ctx_, C001::MN.ADDR, C001::SIZE,
                                 regs_.data() + C001::MN.ADDR);
  if (rc == -1) {
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno("Receive register {} failed", C001::MN.ADDR)));
  }
  return {};
}

std::expected<std::string, ModbusError>
Fronius::getModbusString(const std::vector<uint16_t> &regs,
                         const Register &reg) const {
  std::string str;

  try {
    if (reg.TYPE != RegType::STRING) {
      throw ModbusError::custom(EINVAL,
                                "Invalid register type for getString()");
    }

    if (reg.ADDR + reg.NB > regs.size()) {
      return std::unexpected(
          ModbusError::custom(EINVAL, "Register range out of bounds"));
    }

    str.reserve(reg.NB * 2); // avoid reallocations

    for (size_t i = 0; i < reg.NB; ++i) {
      uint16_t word = regs[reg.ADDR + i];

      char hi = static_cast<char>((word >> 8) & 0xFF);
      char lo = static_cast<char>(word & 0xFF);

      if (hi != '\0')
        str.push_back(hi);
      if (lo != '\0')
        str.push_back(lo);
    }

    // Validate that the string is printable (allow spaces)
    for (unsigned char c : str) {
      if (!std::isprint(c) && c != ' ') {
        throw ModbusError::custom(
            EINVAL, "String at address {} contains unprintable characters",
            reg.ADDR);
      }
    }
  } catch (const ModbusError &e) {
    if (onError_)
      onError_(e);
    return std::unexpected(e);
  }

  return str;
}

std::expected<double, ModbusError>
Fronius::getModbusDouble(const std::vector<uint16_t> &regs, const Register &reg,
                         std::optional<Register> sf) const {
  double value = 0.0;

  try {
    double scale = 1.0;
    if (sf.has_value()) {
      scale = std::pow(10.0, static_cast<int16_t>(regs[sf->ADDR]));
    }

    switch (reg.TYPE) {
    case RegType::INT16:
      value = static_cast<double>(static_cast<int16_t>(regs[reg.ADDR])) * scale;
      break;
    case RegType::UINT16:
      value = static_cast<double>(regs[reg.ADDR]) * scale;
      break;
    case RegType::UINT32:
      value = static_cast<double>(
                  ModbusUtils::modbus_get_uint32(regs.data() + reg.ADDR)) *
              scale;
      break;
    case RegType::FLOAT:
      value =
          static_cast<double>(modbus_get_float_abcd(regs.data() + reg.ADDR));
    default:
      throw ModbusError::custom(EINVAL,
                                "Unsupported register type for getDouble()");
    }
  } catch (const ModbusError &e) {
    if (onError_)
      onError_(e);
    return std::unexpected(e);
  }

  return value;
}

/* -------------------------- private methods ------------------------------*/

std::expected<void, ModbusError> Fronius::tryConnect() {

  // If context already exists and connection is healthy, skip
  if (ctx_ && connected_.load())
    return {}; // Connection healthy, return

  if (ctx_) {
    // Connection lost → transient error
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }

  // Start with a fresh register map
  std::fill(regs_.begin(), regs_.end(), 0);

  // Create new context based on config
  if (cfg_.useTcp) {
    ctx_ =
        modbus_new_tcp_pi(cfg_.host.c_str(), std::to_string(cfg_.port).c_str());
  } else {
    ctx_ = modbus_new_rtu(cfg_.device.c_str(), cfg_.baud, 'N', 8, 1);
  }
  if (!ctx_) {
    return reportError<void>(std::unexpected(
        ModbusError::custom(ENOMEM, "Unable to create the libmodbus {} context",
                            (cfg_.useTcp ? "TCP" : "RTU"))));
  }

  // Set libmodbus debug
  if (cfg_.debug) {
    if (modbus_set_debug(ctx_, true) == -1) {
      modbus_free(ctx_);
      ctx_ = nullptr;
      return reportError<void>(std::unexpected(
          ModbusError::fromErrno("Unable to set the libmodbus debug flag")));
    }

    // --- Extend timeout for debugging ---
    modbus_set_response_timeout(ctx_, 60, 0);
  }

  // Set slave/unit ID
  if (modbus_set_slave(ctx_, cfg_.slaveId) == -1) {
    modbus_free(ctx_);
    ctx_ = nullptr;
    return reportError<void>(std::unexpected(
        ModbusError::fromErrno("Setting slave id '{}' failed", cfg_.slaveId)));
  }

  // Attempt connection
  if (modbus_connect(ctx_) == -1) {
    modbus_free(ctx_);
    ctx_ = nullptr;
    return reportError<void>(std::unexpected(ModbusError::fromErrno(
        "Connection to '{}' failed", (cfg_.useTcp ? cfg_.host : cfg_.device))));
  }

  return {};
}

void Fronius::connectionLoop() {
  int reconnectDelay = cfg_.reconnectDelay;

  while (running_.load()) {

    // --- Try to connect if not already connected ---
    if (!connected_.load()) {
      auto res = tryConnect();
      if (res) {
        // --- Successful connection
        {
          std::lock_guard<std::mutex> lock(mtx_);
          connected_.store(true);
          cv_.notify_all(); // <--- notify waitForConnection
        }

        if (onConnect_)
          onConnect_();

        if (cfg_.exponential)
          reconnectDelay = cfg_.reconnectDelay;

      } else {
        auto &err = res.error();

        // --- Update connected state if necessary
        connected_.store(false);

        if (onDisconnect_)
          onDisconnect_();

        // --- Notify via onError (callback handles logging and severity)
        if (onError_)
          onError_(err);

        // --- Wait for next attempt or shutdown  ---
        {
          std::unique_lock<std::mutex> lock(mtx_);
          cv_.wait_for(lock, std::chrono::seconds(reconnectDelay), [this] {
            return !running_.load() || connected_.load();
          });
        }

        // --- Exponential backoff for next retry ---
        if (cfg_.exponential && !connected_.load())
          reconnectDelay = std::min(reconnectDelay * 2, cfg_.reconnectDelayMax);

        continue;
      }
    }

    // --- Already connected, wait until disconnected or shutdown ---
    {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait(lock, [this] { return !running_.load() || !connected_.load(); });
    }
  }
}