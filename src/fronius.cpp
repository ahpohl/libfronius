#include "fronius.h"
#include "common_registers.h"
#include "modbus_error.h"
#include "modbus_utils.h"
#include <cerrno>
#include <expected>
#include <modbus/modbus.h>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

Fronius::Fronius(const ModbusConfig &cfg) : cfg_(cfg) {
  regs_.resize(0xFFFF, 0);
}

Fronius::~Fronius() {
  running_.store(false);
  if (connectionThread_.joinable())
    connectionThread_.join();

  if (ctx_) {
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
  }
}

bool Fronius::isConnected() const { return connected_.load(); }

std::expected<void, ModbusError> Fronius::connect() {
  running_.store(true);
  connectionThread_ = std::thread(&Fronius::connectionLoop, this);
  return {};
}

void Fronius::waitForConnection() {
  std::unique_lock<std::mutex> lock(mtx_);
  cv_.wait(lock, [this]() { return connected_.load(); });
}

void Fronius::setConnectCallback(std::function<void()> cb) {
  onConnect = std::move(cb);
}
void Fronius::setDisconnectCallback(std::function<void()> cb) {
  onDisconnect = std::move(cb);
}

void Fronius::setErrorCallback(std::function<void(const ModbusError &)> cb) {
  onError = std::move(cb);
}

std::expected<void, ModbusError> Fronius::tryConnect() {
  // If we already have a context, check if the connection is still alive
  if (ctx_) {
    uint16_t dummy;
    int rc = modbus_read_registers(ctx_, C001_ID::ADDR, C001_ID::NB, &dummy);
    if (rc != -1) {
      return {}; // Connection healthy
    }

    // Connection lost → cleanup
    modbus_close(ctx_);
    modbus_free(ctx_);
    ctx_ = nullptr;
    return std::unexpected(ModbusError::fromErrno("Connection lost"));
  }

  // Create new context based on config
  if (cfg_.useTcp) {
    ctx_ =
        modbus_new_tcp_pi(cfg_.host.c_str(), std::to_string(cfg_.port).c_str());
  } else {
    ctx_ = modbus_new_rtu(cfg_.device.c_str(), cfg_.baud, 'N', 8, 1);
  }

  if (!ctx_) {
    return std::unexpected(ModbusError::custom(
        ENOMEM, "Unable to create the libmodbus TCP context"));
  }

  if (cfg_.debug) {
    int rc = modbus_set_debug(ctx_, true);
    if (rc == -1) {
      modbus_free(ctx_);
      ctx_ = nullptr;
      return std::unexpected(ModbusError::fromErrno(
          std::string("Unable to set the libmodbus debug flag")));
    }
  }

  // Attempt connection
  if (modbus_connect(ctx_) == -1) {
    modbus_free(ctx_);
    ctx_ = nullptr;
    return std::unexpected(ModbusError::fromErrno(
        "Connection to '" + (cfg_.useTcp ? cfg_.host : cfg_.device) +
        "' failed"));
  }

  return {};
}

void Fronius::connectionLoop() {
  int retryDelay = cfg_.minRetryDelay;

  while (true) {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      if (!running_)
        break;
    }
    auto res = tryConnect();

    std::lock_guard<std::mutex> lock(mtx_);
    if (res) {
      if (!connected_.load()) {
        connected_.store(true);
        cv_.notify_all();
        if (onConnect)
          onConnect();
      }
      retryDelay = cfg_.minRetryDelay;
    } else {
      if (connected_.load()) {
        connected_.store(false);
        if (onDisconnect)
          onDisconnect();
        if (onError)
          onError(res.error());
      }
    }

    for (int i = 0; i < retryDelay * 10 && running_; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!res && (retryDelay <= cfg_.maxRetryDelay)) {
      retryDelay = std::min(retryDelay * 2, cfg_.maxRetryDelay);
    }
  }
}

std::expected<bool, ModbusError> Fronius::isSunSpecDevice(void) {

  // Get registers
  int rc = modbus_read_registers(ctx_, C001_SID::ADDR, 4,
                                 regs_.data() + C001_SID::ADDR);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_SID::ADDR) + " failed"));
  }

  // Test for "SunS" string
  if (!(regs_[C001_SID::ADDR] == 0x5375 &&
        regs_[C001_SID::ADDR + 1] == 0x6e53)) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "SunSpec signature mismatch: expected [0x5375, 0x6e53], "
                "received [0x" +
                    modbus_utils::to_hex(regs_[C001_SID::ADDR]) + ", 0x" +
                    modbus_utils::to_hex(regs_[C001_SID::ADDR + 1]) + "]"));
  }

  // Test for Common Register ID
  if (!(regs_[C001_ID::ADDR] = 0x1)) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Not a SunSpec common register map: received " +
                    std::to_string(regs_[C001_ID::ADDR]) + ", expected 1"));
  }

  // Test for Common Register Map Length
  if (regs_[C001_L::ADDR] != C001_SIZE) {
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid common register map size: received " +
                    std::to_string(regs_[C001_L::ADDR]) + ", expected " +
                    std::to_string(C001_SIZE)));
  }

  return {};
}

std::expected<void, ModbusError> Fronius::fetchCommonRegisters(void) {

  int rc = modbus_read_registers(ctx_, C001_MN::ADDR, C001_SIZE,
                                 regs_.data() + C001_MN::ADDR);
  if (rc == -1) {
    return std::unexpected(
        ModbusError::fromErrno(std::string("Receive register ") +
                               std::to_string(C001_MN::ADDR) + " failed"));
  }
  return {};
}

std::expected<std::string, ModbusError> Fronius::getManufacturer() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_MN::ADDR,
                                         C001_MN::NB);
}

std::expected<std::string, ModbusError> Fronius::getDeviceModel() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_MD::ADDR,
                                         C001_MD::NB);
}

std::expected<std::string, ModbusError> Fronius::getOptions() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_OPT::ADDR,
                                         C001_OPT::NB);
}

std::expected<std::string, ModbusError> Fronius::getFwVersion() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_VR::ADDR,
                                         C001_VR::NB);
}

std::expected<std::string, ModbusError> Fronius::getSerialNumber() {
  return modbus_utils::modbus_get_string(regs_.data() + C001_SN::ADDR,
                                         C001_SN::NB);
}

std::expected<uint16_t, ModbusError> Fronius::getModbusDeviceAddress() {
  uint16_t val = regs_[C001_DA::ADDR];

  if ((val < 1) || (val > 247))
    return std::unexpected(ModbusError::custom(
        EINVAL, "Invalid Modbus slave address: received " +
                    std::to_string(val) + ", expected 1-247"));
  return val;
}
