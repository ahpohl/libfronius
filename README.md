# libfronius

[![Build Status](https://github.com/ahpohl/libfronius/actions/workflows/build.yml/badge.svg)](https://github.com/ahpohl/libfronius/actions/workflows/build.yml)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue)](https://ahpohl.github.io/libfronius/) <!-- Update this link if your docs are elsewhere -->
[![License: MIT](https://img.shields.io/github/license/ahpohl/libfronius)](LICENSE)

**libfronius** is a modern C++ library designed to communicate with Fronius inverters and smart meters using Modbus. It automatically detects register types, device inputs (MPPT trackers, phases), and supports robust reconnection strategies. The library is structured around a base `Fronius` class, with specialized `Inverter` and `Meter` subclasses for device representation. It supports multiple Modbus transports, including Modbus TCP over IPv4 and IPv6 networks and Modbus RTU over serial connections.

## Features

- **Multiple Transport Protocols**: Modbus TCP (IPv4/IPv6) and Modbus RTU (serial).
- **Automatic Register Detection**: Supports both integer/scale factor and float register types.
- **Automatic Input & Phase Detection**: Finds the number of inverter inputs (MPPT trackers) and electrical phases.
- **Hybrid Inverter Detection**: Identifies hybrid devices capable of battery connection (battery state reading not yet supported).
- **Robust Connection Handling**:
  - Manages connection to Modbus master.
  - Automatically reconnects after connection loss (e.g., inverter standby at night).
  - Configurable exponential backoff for reconnection attempts.
- **Object-Oriented Design**: 
  - `Fronius` base class.
  - `Inverter` and `Meter` subclasses.
- **Doxygen Documentation**: [Browse the docs](https://ahpohl.github.io/libfronius/) <!-- Update this link if needed -->
- **Depends on [libmodbus](https://libmodbus.org/)** for low-level Modbus communication.

## Installation

**1. Install libmodbus**  

See [libmodbus installation guide](https://libmodbus.org/getting_started/).

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

Include `libfronius` in your C++ project and instantiate an `Inverter` or `Meter` to begin communication.  
See [Doxygen documentation](https://ahpohl.github.io/libfronius/) for detailed API usage.

### Example: Read current active power (W)

The example below shows how to configure a TCP connection and read the current active power from an inverter using `std::expected`-based error handling provided by the library.

```cpp
#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "modbus_config.h"
#include "modbus_error.h"
#include "inverter.h"
#include "fronius_types.h"

int main() {
  ModbusConfig cfg;

  // Modbus TCP (IPv4/IPv6)
  cfg.host = "192.168.6.78";
  cfg.secTimeout = 5;

  // Optionally enable libmodus debug output
  // cfg.debug = true;

  // Validate configuration before use
  cfg.validate();

  // Create inverter client
  Inverter inverter(cfg);
  
  // Register error callback function
  inverter.setErrorCallback([](const ModbusError &err) {
    std::cerr << "FATAL Modbus error: " <<  err.message << ": "
              <<  modbus_strerror(err.code) << "\n";
  });

  // Start connect (async)
  inverter.connect();

  // Wait until connected or timeout
  const auto timeout = std::chrono::seconds(5);
  const auto pollInterval = std::chrono::milliseconds(100);
  const auto start = std::chrono::steady_clock::now();

  while (!inverter.isConnected() &&
         (std::chrono::steady_clock::now() - start) < timeout) {
    std::this_thread::sleep_for(pollInterval);
  }

  if (!inverter.isConnected()) {
    std::cerr << "Connection timed out after " << timeout.count()
              << " seconds" << "\n";
    return EXIT_FAILURE;
  }

  try {
    // Validate inverter registers before use
    ModbusError::getOrThrow(inverter.validateDevice());

    // Fetch inverter registers
    ModbusError::getOrThrow(inverter.fetchInverterRegisters());

    // Read the current active power in watts (W).
    // getAcPower() returns std::expected<double, ModbusError>.
    const double watts =
        ModbusError::getOrThrow(inverter.getAcPower(FroniusTypes::Output::ACTIVE));

    std::cout << "Active power: " << watts << " W\n";

  } catch (const ModbusError &e) {
    std::cerr << "Failed to read active power (" << e.code << "): "
              << e.message << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
```

Compile (examples):
- GCC/Clang:
  ```sh
  g++ -std=c++23 -I../include inverter.cpp ./lib/libfronius.a -lmodbus -o inverter
  ```
- MSVC:
  ```bat
  cl /std:c++23 /I ..\include inverter.cpp .\lib\fronius_static.lib modbus.lib
  ```

Note:
- The library uses C++ standard features such as `std::expected` (and may use `std::format`), so make sure to select a suitable C++ standard for your compiler (e.g., C++23 with GCC/Clang).

## Configuration

All configurable parameters are defined in [include/modbus_config.h](https://github.com/ahpohl/libfronius/blob/4b433bcb3af9fcceb235198372ff97b40447a941/include/modbus_config.h) via the `ModbusConfig` struct. Call `validate()` to ensure values are within allowed ranges before use.

- debug
  - Type: bool
  - Default: false
  - Description: Enable debug logging.

- slaveId
  - Type: int
  - Default: 1
  - Valid range: 1–247
  - Description: Modbus slave ID (unit ID).

- useTcp
  - Type: bool
  - Default: true
  - Description: If true, use Modbus TCP; if false, use Modbus RTU (serial).

- host
  - Type: string
  - Default: ""
  - Used when: useTcp == true
  - Description: TCP hostname or IP address. IPv4 and IPv6 are supported.

- port
  - Type: int
  - Default: 502
  - Valid range: 1–65535
  - Description: TCP port.

- device
  - Type: string
  - Default: ""
  - Used when: useTcp == false
  - Example: "/dev/ttyUSB0"
  - Description: Serial device path for RTU.

- baud
  - Type: int
  - Default: 9600
  - Valid range: > 0
  - Used when: useTcp == false
  - Description: Serial baud rate for RTU.

- secTimeout
  - Type: int
  - Default: 0
  - Valid range: ≥ 0
  - Description: Response timeout in seconds. At least one of secTimeout or usecTimeout must be non-zero.

- usecTimeout
  - Type: int
  - Default: 200000
  - Valid range: 0–999999
  - Description: Response timeout in microseconds. At least one of secTimeout or usecTimeout must be non-zero.

- reconnectDelay
  - Type: int
  - Default: 5
  - Valid range: ≥ 0 and strictly less than reconnectDelayMax
  - Description: Initial delay (seconds) before attempting to reconnect.

- reconnectDelayMax
  - Type: int
  - Default: 320
  - Valid range: ≥ 0 and strictly greater than reconnectDelay
  - Description: Maximum delay (seconds) between reconnection attempts.

- exponential
  - Type: bool
  - Default: true
  - Description: If true, use exponential backoff for reconnection delays; otherwise use a constant delay.

## Limitations

- Battery state reading is not supported yet (awaiting hybrid device testing).
- Requires [libmodbus](https://libmodbus.org/) installed on your system.

## License

[MIT](LICENSE)

---

*libfronius* is not affiliated with or endorsed by Fronius International GmbH.