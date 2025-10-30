# libfronius

[![Build Status](https://github.com/ahpohl/libfronius/actions/workflows/build.yml/badge.svg)](https://github.com/ahpohl/libfronius/actions/workflows/build.yml)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue)](https://ahpohl.github.io/libfronius/) <!-- Update this link if your docs are elsewhere -->
[![License: MIT](https://img.shields.io/github/license/ahpohl/libfronius)](LICENSE)

**libfronius** is a modern C++ library designed to communicate with Fronius inverters and smart meters using Modbus. It automatically detects register types, device inputs (MPPT trackers, phases), and supports robust reconnection strategies. The library is structured around a base `Fronius` class, with specialized `Inverter` and `Meter` subclasses for device representation.

## Features

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

1. **Install libmodbus**  
   See [libmodbus installation guide](https://libmodbus.org/getting_started/).

2. **Clone libfronius**
   ```sh
   git clone https://github.com/ahpohl/libfronius.git
   cd libfronius
   ```

3. **Build**
   ```sh
   mkdir build && cd build
   cmake ..
   make
   ```

## Usage

Include `libfronius` in your C++ project and instantiate an `Inverter` or `Meter` to begin communication.  
See [Doxygen documentation](https://ahpohl.github.io/libfronius/) for detailed API usage.

## Limitations

- Battery state reading is not supported yet (awaiting hybrid device testing).
- Requires [libmodbus](https://libmodbus.org/) installed on your system.

## License

[MIT](LICENSE)

---

*libfronius* is not affiliated with or endorsed by Fronius International GmbH.