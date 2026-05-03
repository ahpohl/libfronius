/**
 * @file common_registers.h
 * @brief SunSpec Common Model (C001) register definitions for Fronius
 *        devices.
 *
 * @details
 * Symbolic register addresses for the SunSpec Common Model block, which
 * carries generic device metadata (manufacturer, model, firmware versions,
 * serial number) and is implemented identically across all SunSpec-
 * compliant Fronius devices.
 */

#ifndef COMMON_REGISTERS_H_
#define COMMON_REGISTERS_H_

#include "register_base.h"
#include <cstdint>

/**
 * @namespace C001
 * @brief SunSpec Common Model (ID 1) registers.
 *
 * Generic device metadata block, identical across all SunSpec-compliant
 * devices (inverters, meters, batteries).
 */
namespace C001 {

/** @brief Total length of the Common Model block, in registers. */
constexpr uint16_t SIZE = 65;

/**
 * @brief SunSpec identifier register.
 *
 * Marks the start of the SunSpec map. The 32-bit value reads as ASCII
 * `"SunS"` (0x53756E53).
 */
constexpr Register SID(40000, 2, Register::Type::UINT32);

/**
 * @brief Common Model ID register.
 *
 * @return Always 1 — identifies this block as the Common Model.
 */
constexpr Register ID(40002, 1, Register::Type::UINT16);

/**
 * @brief Length of the Common Model block.
 *
 * @return Always 65.
 */
constexpr Register L(40003, 1, Register::Type::UINT16);

/** @brief Manufacturer name (typically "Fronius"). */
constexpr Register MN(40004, 16, Register::Type::STRING);

/** @brief Device model name (e.g. "IG+150V [3p]"). */
constexpr Register MD(40020, 16, Register::Type::STRING);

/** @brief Firmware version of an installed option (e.g. Datamanager). */
constexpr Register OPT(40036, 8, Register::Type::STRING);

/** @brief Main device firmware version. */
constexpr Register VR(40044, 8, Register::Type::STRING);

/**
 * @brief Device serial number.
 *
 * May not always be the serial number printed on the nameplate; depending
 * on the device family and firmware, the value can fall back to an
 * inverter-controller serial (PMC) or the controller's unique ID (UID).
 *
 * @note On SYMOHYBRID devices, only the fallback values are available.
 * @note May change dynamically during startup or synchronisation.
 */
constexpr Register SN(40052, 16, Register::Type::STRING);

/**
 * @brief Modbus device address (slave ID, range 1–247).
 */
constexpr Register DA(40068, 1, Register::Type::UINT16);

} // namespace C001

#endif /* COMMON_REGISTERS_H_ */
