/**
 * @file common_registers.h
 * @brief Defines SunSpec Common Model (C001) register mappings for Fronius
 * devices.
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * Common Model block (C001), which describes general device information such as
 * manufacturer, model, firmware versions, and serial number.
 *
 * These definitions are used for Modbus communication with Fronius inverters,
 * meters, and other compatible devices.
 */

#ifndef COMMON_REGISTERS_H_
#define COMMON_REGISTERS_H_

#include <cstdint>

/**
 * @namespace C001
 * @brief SunSpec Common Model registers.
 *
 * @details
 * This namespace defines the register addresses and sizes for the SunSpec
 * Common Model (ID 1). The Common Model contains generic device information
 * such as manufacturer, model identifier, firmware version, and serial number.
 *
 * It is implemented consistently across all SunSpec-compliant devices including
 * Fronius inverters, meters, and batteries, serving as a standardized metadata
 * block for identification and management.
 */
namespace C001 {

/**
 * @brief Total length of the Common Model block (in registers).
 */
constexpr uint16_t SIZE = 65;

/**
 * @struct SID
 * @brief SunSpec identifier register.
 *
 * @details
 * Uniquely identifies this Modbus map as a SunSpec device.
 * The returned value corresponds to the ASCII string `"SunS"`.
 *
 * @return 0x53756e53 ('SunS')
 */
struct SID {
  static constexpr uint16_t ADDR = 40001 - 1; ///< Starting register address.
  static constexpr uint16_t NB = 2;           ///< Number of registers.
};

/**
 * @struct ID
 * @brief Common Model ID register.
 *
 * @details
 * Identifies this block as the SunSpec Common Model.
 *
 * @return Always returns 1.
 */
struct ID {
  static constexpr uint16_t ADDR = 40003 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of the Common Model block.
 *
 * @details
 * Indicates the number of registers (65) used by this model.
 *
 * @return Always returns 65.
 */
struct L {
  static constexpr uint16_t ADDR = 40004 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MN
 * @brief Manufacturer name.
 *
 * @details
 * Contains the manufacturer string, typically "Fronius".
 *
 * @return Manufacturer name as a string (e.g. "Fronius").
 */
struct MN {
  static constexpr uint16_t ADDR = 40005 - 1;
  static constexpr uint16_t NB = 16;
};

/**
 * @struct MD
 * @brief Device model.
 *
 * @details
 * Specifies the model name of the device.
 *
 * @return Model string (e.g. "IG+150V [3p]").
 */
struct MD {
  static constexpr uint16_t ADDR = 40021 - 1;
  static constexpr uint16_t NB = 16;
};

/**
 * @struct OPT
 * @brief Software version of installed option.
 *
 * @details
 * Indicates firmware version of optional components,
 * such as the Datamanager board.
 *
 * @return Firmware version string.
 */
struct OPT {
  static constexpr uint16_t ADDR = 40037 - 1;
  static constexpr uint16_t NB = 8;
};

/**
 * @struct VR
 * @brief Main device firmware version.
 *
 * @details
 * Provides the firmware version of the primary device,
 * such as inverter, meter, or battery.
 *
 * @return Firmware version string.
 */
struct VR {
  static constexpr uint16_t ADDR = 40045 - 1;
  static constexpr uint16_t NB = 8;
};

/**
 * @struct SN
 * @brief Device serial number.
 *
 * @details
 * Contains the serial number of the device. Depending on
 * device type and firmware, this field may not always represent the
 * printed serial number on the nameplate.
 *
 * Fallbacks if the inverter serial number is not supported:
 *  - **1:** Serial of inverter controller (PMC)
 *  - **2:** Unique ID (UID) of inverter controller
 *
 * @note On SYMOHYBRID devices, only fallback values are available.
 * @note This field may change dynamically during startup or synchronization.
 *
 * @return Serial number or fallback unique identifier.
 */
struct SN {
  static constexpr uint16_t ADDR = 40053 - 1;
  static constexpr uint16_t NB = 16;
};

/**
 * @struct DA
 * @brief Modbus device address register.
 *
 * @details
 * Contains the current Modbus slave ID of the device.
 *
 * @return Device address (1–247).
 */
struct DA {
  static constexpr uint16_t ADDR = 40069 - 1;
  static constexpr uint16_t NB = 1;
};

} // namespace C001

#endif /* COMMON_REGISTERS_H_ */
