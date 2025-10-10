/**
 * @file fronius_registers.h
 * @brief Defines Fronius-specific Modbus register mappings.
 *
 * @details
 * This header defines additional Modbus registers that are specific to
 * Fronius devices and extend the standard SunSpec register map. These registers
 * allow control over stored data, inverter states, model configuration, and
 * access to system-wide energy statistics.
 *
 * These registers are used primarily by Fronius inverters and Datamanager units
 * for system-level communication, monitoring, and maintenance operations.
 */

#ifndef FRONIUS_REGISTERS_H_
#define FRONIUS_REGISTERS_H_

#include <cstdint>

/**
 * @namespace F
 * @brief Fronius-specific Modbus register definitions.
 *
 * @details
 * Contains Fronius proprietary Modbus registers that extend or complement the
 * SunSpec register maps. These registers provide device management functions,
 * inverter status reporting, and access to site-level power and energy data.
 */
namespace F {

/**
 * @struct DELETE_DATA
 * @brief Delete stored rating data of the current inverter.
 *
 * @details
 * Writing the value `0xFFFF` deletes stored rating data for the
 * currently addressed inverter.
 */
struct DELETE_DATA {
  static constexpr uint16_t ADDR = 212; ///< Register address.
  static constexpr uint16_t NB = 1;     ///< Number of registers.
};

/**
 * @struct STORE_DATA
 * @brief Store rating data persistently.
 *
 * @details
 * Writing `0xFFFF` stores the rating data of all inverters connected to the
 * Fronius Datamanager persistently.
 */
struct STORE_DATA {
  static constexpr uint16_t ADDR = 213;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ACTIVE_STATE_CODE
 * @brief Current active inverter state code.
 *
 * @details
 * Reports the current operational state of the inverter.
 * Refer to the inverter manual for detailed state descriptions.
 *
 * @note Not supported for Fronius Hybrid inverters. Status may differ during
 * night-time operation compared to other inverter models.
 */
struct ACTIVE_STATE_CODE {
  static constexpr uint16_t ADDR = 214;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct RESET_ALL_EVENT_FLAGS
 * @brief Reset all event flags and active state code.
 *
 * @details
 * Writing the value `0xFFFF` resets all event flags and clears
 * the active state code register.
 */
struct RESET_ALL_EVENT_FLAGS {
  static constexpr uint16_t ADDR = 215;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MODEL_TYPE
 * @brief Select SunSpec model type used for inverter and meter data.
 *
 * @details
 * Defines whether the SunSpec data is represented using floating-point or
 * integer & scale-factor models.
 * After setting, the value `6` must be written to confirm the change.
 *
 * - `1`: Floating point model
 * - `2`: Integer & scale factor model
 */
struct MODEL_TYPE {
  static constexpr uint16_t ADDR = 216;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STORAGE_RESTRICTIONS_VIEW_MODE
 * @brief Defines scope of storage restriction reporting.
 *
 * @details
 * Selects which type of restriction data is reported by the
 * BasicStorageControl model (IC124).
 *
 * - `0`: Local restrictions (default, set by Modbus interface)
 * - `1`: Global restrictions (system-wide)
 */
struct STORAGE_RESTRICTIONS_VIEW_MODE {
  static constexpr uint16_t ADDR = 217;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct SITE_POWER
 * @brief Total site power of all connected inverters.
 *
 * @unit Watt [W]
 *
 * @details
 * Reports the instantaneous total active power output across all
 * connected inverters in the system.
 */
struct SITE_POWER {
  static constexpr uint16_t ADDR = 500;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct SITE_ENERGY_DAY
 * @brief Total energy produced today by all connected inverters.
 *
 * @unit Watt-hour [Wh]
 *
 * @details
 * Accumulates the total daily energy production across all connected
 * inverters since midnight.
 */
struct SITE_ENERGY_DAY {
  static constexpr uint16_t ADDR = 502;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct SITE_ENERGY_YEAR
 * @brief Total energy produced during the current year.
 *
 * @unit Watt-hour [Wh]
 *
 * @details
 * Represents the cumulative energy produced by all connected inverters
 * since January 1st of the current year.
 */
struct SITE_ENERGY_YEAR {
  static constexpr uint16_t ADDR = 506;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct SITE_ENERGY_TOTAL
 * @brief Lifetime total energy produced by all connected inverters.
 *
 * @unit Watt-hour [Wh]
 *
 * @details
 * Indicates the total lifetime energy yield of all connected
 * inverters in the system.
 */
struct SITE_ENERGY_TOTAL {
  static constexpr uint16_t ADDR = 510;
  static constexpr uint16_t NB = 4;
};

} // namespace F

#endif /* FRONIUS_REGISTERS_H_ */
