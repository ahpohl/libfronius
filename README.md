# libfronius

[![Build Status](https://img.shields.io/github/actions/workflow/status/yourusername/libfronius/cmake.yml?branch=main)](https://github.com/yourusername/libfronius/actions)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Doxygen](https://img.shields.io/badge/docs-Doxygen-brightgreen)](https://ahpohl.github.io/libfronius/)

**libfronius** is a modern C++ library for communicating with Fronius inverters and smart meters.  
It provides type-safe, high-level access to inverter and meter data via Modbus, handling device detection and connection management automatically.

---

## Features

| Feature | Description |
|---------|-------------|
| **Automatic register detection** | Supports integer registers with scale factors and floating-point registers. |
| **Automatic device detection** | Detects number of MPPT inputs, number of phases, and hybrid inverter capability. |
| **Hybrid inverter support** | Detects hybrid capability (battery connection on second input). Battery state reading currently **not supported**. |
| **Connection management** | Handles Modbus master connections, auto-reconnects after connection loss, supports configurable exponential backoff. |
| **Device abstraction** | `Fronius` base class with `Inverter` and `Meter` child classes representing real devices. |
| **Documentation** | Generated with [Doxygen](https://yourusername.github.io/libfronius/). |
| **Dependencies** | Requires [libmodbus](https://libmodbus.org/) for low-level communication. |

---

## Installation

1. Clone the repository:

```bash
git clone https://github.com/ahpohl/libfronius.git
cd libfronius
```

2. Build with CMake:

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

3. Ensure `libmodbus` is installed on your system.

---

## Getting Started with CMake

To use `libfronius` in your own project:

1. Create a `CMakeLists.txt` for your project:

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyFroniusApp)

set(CMAKE_CXX_STANDARD 20)
find_package(libfronius REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE libfronius::libfronius)
```

2. In your `main.cpp`:

```cpp
#include <libfronius/Inverter.h>
#include <iostream>

int main() {
    Fronius::Inverter inverter("192.168.178.30"); // replace with your inverter IP

    if (auto result = inverter.getAcOutput(FroniusTypes::Output::ACTIVE); result) {
        std::cout << "Active power: " << *result << " W\n";
    } else {
        std::cerr << "Error reading AC output: " << result.error().message() << "\n";
    }

    return 0;
}
```

3. Build your project:

```bash
mkdir build
cd build
cmake ..
make
./my_app
```

---

## Supported Outputs

| Output Type | Description |
|-------------|-------------|
| `ACTIVE` | Active (real) power in W |
| `APPARENT` | Apparent power in VA |
| `Q1_REACTIVE` | Reactive power Q1 (inductive) in VAr |
| `Q4_REACTIVE` | Reactive power Q4 (capacitive) in VAr |

---

## Contributing

Contributions are welcome! Please submit issues or pull requests via GitHub.  
Follow the repository's coding style and ensure all new features are documented.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

