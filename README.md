# libfronius

[![Build Status](https://github.com/ahpohl/libfronius/actions/workflows/build.yml/badge.svg)](https://github.com/ahpohl/libfronius/actions/workflows/build.yml)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue)](https://ahpohl.github.io/libfronius/)
[![License: MIT](https://img.shields.io/github/license/ahpohl/libfronius)](LICENSE)

**libfronius** is a modern C++ library for communicating with Fronius inverters and smart meters over Modbus. It automatically detects register maps, device inputs (MPPT trackers), phases, and hybrid storage capability, and supports robust per-bus and per-device reconnection strategies. The library supports Modbus TCP over IPv4/IPv6 and Modbus RTU over serial connections.

## Features

- **Multiple transport protocols**: Modbus TCP (IPv4/IPv6) and Modbus RTU (serial).
- **Shared RTU bus**: An inverter and a meter on the same RS-485 port share a single `FroniusBus` instance. All register reads are serialised through a thread-safe transaction queue, so the physical bus is never contended. Devices on different ports each get their own bus instance.
- **Per-device reconnection**: When one device on a shared bus times out or becomes temporarily unavailable, only that device is retried — the bus itself and any other device on it continue unaffected.
- **Automatic register detection**: Supports both integer/scale-factor (I10X, M20X) and float (I11X, M21X) SunSpec register models, as well as the proprietary Fronius RTU map for the Smart Meter TS 65A-3.
- **Automatic input and phase detection**: Determines the number of MPPT tracker inputs and AC phases by probing the I160 multi-MPPT extension block.
- **Hybrid inverter detection**: Identifies hybrid inverters by the presence of the I124 basic storage control block. Battery state reading is not yet supported.
- **Dual meter access modes**:
  - **Via the Fronius inverter** — meter values exposed through the inverter's SunSpec register map when the TS 65A-3 is configured in the inverter's web interface.
  - **Direct proprietary access** — reads the TS 65A-3 register map over a direct Modbus RTU connection, without routing through the inverter.
- **Granular device callbacks**: Separate ready, unavailable, retry-pending, and error callbacks let the application wire only the events it cares about.
- **`std::expected`-based error handling**: All accessors return `std::expected<T, ModbusError>`. `ModbusError` carries a severity classification (TRANSIENT, FATAL, SHUTDOWN) so the application can decide whether to retry, shut down, or ignore.
- **Doxygen documentation**: [Browse the docs](https://ahpohl.github.io/libfronius/)
- **Depends on [libmodbus](https://libmodbus.org/)** for low-level Modbus communication.

## Architecture

The library separates bus-level and device-level concerns into two distinct configuration structs and two runtime objects:

```
ModbusBusConfig  ──►  FroniusBus   (one per physical port or TCP endpoint)
                           │
                    ┌──────┴──────┐
                    ▼             ▼
ModbusDeviceConfig ► Inverter   Meter   (one per Modbus slave)
```

`FroniusBus` owns the `modbus_t` context, the connection/reconnection thread, and the transaction queue. Multiple `FroniusDevice` instances (e.g. an inverter and a meter sharing a single RS-485 port) register with the same `FroniusBus` and submit read transactions through it. The bus thread drains the queue strictly sequentially, issuing one `modbus_read_registers()` call at a time. When a device-level error occurs the bus retries only the affected device via `scheduleDeviceRetry()`, leaving the bus and all other devices running normally.

## Installation

**1. Install libmodbus**

See the [libmodbus installation guide](https://libmodbus.org/getting_started/).

**2. Clone libfronius**

```sh
git clone https://github.com/ahpohl/libfronius.git
cd libfronius
```

**3. Build**

```sh
mkdir build && cd build
cmake ..
make
```

## Usage

The typical setup is: create one `FroniusBus` per physical transport, create one device object per slave, register callbacks, then call `bus->connect()`.

### Example: Inverter on Modbus TCP

```cpp
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "modbus_config.h"
#include "modbus_error.h"
#include "fronius_bus.h"
#include "inverter.h"
#include "fronius_types.h"

int main() {
  // --- Bus configuration (one per TCP endpoint) ---
  ModbusBusConfig busCfg;
  busCfg.transport = ModbusTcpTransport{
      .host = "192.168.6.78",
      .port = 502,
  };
  busCfg.reconnectDelay    = 5;
  busCfg.reconnectDelayMax = 320;
  busCfg.exponential       = true;
  busCfg.validate();

  // --- Device configuration (one per Modbus slave) ---
  ModbusDeviceConfig devCfg;
  devCfg.slaveId     = 1;
  devCfg.secTimeout  = 5;
  devCfg.usecTimeout = 0;
  devCfg.validate();

  // --- Create bus and device ---
  auto bus      = std::make_shared<FroniusBus>(busCfg);
  auto inverter = std::make_shared<Inverter>(bus, devCfg);
  bus->registerDevice(inverter);

  // --- Bus-level callbacks ---
  bus->addBusConnectCallback([&bus] {
    auto ep = bus->getRemoteEndpoint();
    std::cout << "Bus connected to " << ep.ip << ":" << ep.port << "\n";
  });
  bus->addBusDisconnectCallback([](int delay) {
    std::cerr << "Bus disconnected, retrying in " << delay << "s\n";
  });

  // --- Device state callbacks ---
  inverter->setDeviceReadyCallback([](FroniusTypes::RegisterMap map) {
    std::cout << "Inverter ready ("
              << FroniusTypes::toString(map) << " map)\n";
  });
  inverter->setDeviceUnavailableCallback([] {
    std::cerr << "Inverter disconnected\n";
  });
  inverter->setDeviceRetryCallback([](int retryDelay) {
    std::cerr << "Inverter unavailable, retrying in "
              << retryDelay << "s\n";
  });

  // --- Device error callback ---
  inverter->setDeviceErrorCallback([](const ModbusError &err) {
    std::cerr << "Modbus error: " << err.describe() << "\n";
  });

  // --- Start the bus (launches connection thread) ---
  bus->connect();

  // --- Wait until device is ready ---
  while (!inverter->isReady()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  try {
    ModbusError::getOrThrow(inverter->fetchInverterRegisters());

    double watts = ModbusError::getOrThrow(
        inverter->getAcPower(FroniusTypes::Output::ACTIVE));

    std::cout << "Active power: " << watts << " W\n";

  } catch (const ModbusError &e) {
    std::cerr << "Read failed: " << e.describe() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
```

### Example: Inverter and meter sharing a single RS-485 bus

When both devices sit on the same serial port, pass the same `FroniusBus` to both. The bus thread serialises all reads automatically, and a timeout on one device does not affect the other.

```cpp
ModbusBusConfig busCfg;
busCfg.transport = ModbusRtuTransport{
    .device   = "/dev/ttyUSB0",
    .baud     = 9600,
    .dataBits = 8,
    .stopBits = 1,
    .parity   = 'N',
};
busCfg.validate();

auto bus = std::make_shared<FroniusBus>(busCfg);

ModbusDeviceConfig inverterDev;
inverterDev.slaveId     = 1;
inverterDev.secTimeout  = 5;
inverterDev.usecTimeout = 0;

ModbusDeviceConfig meterDev;
meterDev.slaveId     = 3;
meterDev.secTimeout  = 5;
meterDev.usecTimeout = 0;

auto inverter = std::make_shared<Inverter>(bus, inverterDev);
auto meter    = std::make_shared<Meter>(bus, meterDev);

bus->registerDevice(inverter);
bus->registerDevice(meter);

// Register callbacks for each device independently ...

bus->connect();  // single connect call covers both devices
```

If the meter becomes temporarily unavailable, `scheduleDeviceRetry()` retries it in the background with exponential backoff. The inverter keeps reading without interruption.

### Compile

```sh
g++ -std=c++23 -I../include example.cpp ./lib/libfronius.a -lmodbus -o example
```

The library requires C++23 for `std::expected` and `std::format`.

## Configuration

Configuration is split into two structs defined in `include/modbus_config.h`.

### `ModbusBusConfig` — one per physical bus

Controls the transport and bus-level reconnection policy. Passed to `FroniusBus`.

| Field | Type | Default | Description |
|---|---|---|---|
| `transport` | `variant<ModbusTcpTransport, ModbusRtuTransport>` | — | TCP or RTU transport descriptor (required). |
| `debug` | `bool` | `false` | Enable libmodbus debug output. |
| `reconnectDelay` | `int` | `5` | Initial bus reconnect delay in seconds. |
| `reconnectDelayMax` | `int` | `320` | Maximum bus reconnect delay in seconds. |
| `exponential` | `bool` | `true` | Use exponential backoff for bus reconnects. |

**`ModbusTcpTransport`**

| Field | Type | Default | Description |
|---|---|---|---|
| `host` | `string` | `""` | Hostname or IPv4/IPv6 address. |
| `port` | `int` | `502` | TCP port (1–65535). |

**`ModbusRtuTransport`**

| Field | Type | Default | Description |
|---|---|---|---|
| `device` | `string` | `""` | Serial device path, e.g. `/dev/ttyUSB0`. |
| `baud` | `int` | `9600` | Baud rate (must be positive). |
| `dataBits` | `int` | `8` | Data bits (5–8). |
| `stopBits` | `int` | `1` | Stop bits (1 or 2). |
| `parity` | `char` | `'N'` | Parity: `'N'` none, `'E'` even, `'O'` odd. |

### `ModbusDeviceConfig` — one per Modbus slave

Controls the per-device slave ID, response timeout, and per-device reconnection policy. Passed to `Inverter`, `Meter`, etc.

| Field | Type | Default | Description |
|---|---|---|---|
| `slaveId` | `int` | `1` | Modbus slave ID (1–247). |
| `secTimeout` | `int` | `0` | Response timeout, seconds component. |
| `usecTimeout` | `int` | `200000` | Response timeout, microseconds (0–999999). At least one of `secTimeout`/`usecTimeout` must be non-zero. |
| `reconnectDelay` | `int` | `5` | Initial per-device retry delay in seconds. |
| `reconnectDelayMax` | `int` | `320` | Maximum per-device retry delay in seconds. |
| `exponential` | `bool` | `true` | Use exponential backoff for per-device retries. |

Call `validate()` on both structs before use to catch out-of-range parameters early.

## Callbacks

### Bus-level (registered on `FroniusBus`)

Connect, disconnect, and error callbacks may be registered multiple times and are invoked in registration order — useful when two masters share a single bus and each wants its own handler. The log callback is the exception: only the first registration is retained, since bus log output is high-volume and a single sink is usually what you want.

| Method | Callback signature | When fired |
|---|---|---|
| `addBusConnectCallback` | `void()` | Physical bus connected, before device validation begins. |
| `addBusDisconnectCallback` | `void(int delay)` | Bus dropped or connection attempt failed; `delay` is seconds until the next attempt. |
| `addBusErrorCallback` | `void(const ModbusError&)` | Transport-level error not specific to any one slave (CRC, framing, connection timeout). Per-slave errors are delivered via the device error callback. |
| `addBusLogCallback` | `void(const std::string&)` | Internal diagnostic message (queue dequeue, transaction send/response, slave switch, etc.). Single-sink — only the first registered callback is retained. |

### Device-level (registered on `Inverter` / `Meter`)

Each device callback is a single setter; assigning a new handler replaces the previous one.

| Method | Callback signature | When fired |
|---|---|---|
| `setDeviceReadyCallback` | `void(RegisterMap map)` | Device validated successfully. `map` indicates which register map (SunSpec or proprietary) was detected. |
| `setDeviceUnavailableCallback` | `void()` | Device validation failed or the bus dropped. |
| `setDeviceRetryCallback` | `void(int delay)` | A per-device retry has been scheduled; `delay` is seconds until the next attempt. |
| `setDeviceErrorCallback` | `void(const ModbusError&)` | A Modbus error occurred on this device. Inspect `err.severity`: `FATAL` → initiate shutdown; `TRANSIENT` → call `bus->scheduleDeviceRetry(device)` to retry only this slave; `SHUTDOWN` → clean up quietly. |

## Limitations

- Battery state reading is not yet supported (awaiting hybrid device testing).
- Requires [libmodbus](https://libmodbus.org/) installed on the system.

## License

[MIT](LICENSE)

---

*libfronius* is not affiliated with or endorsed by Fronius International GmbH.
