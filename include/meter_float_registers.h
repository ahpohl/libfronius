/**
 * @file meter_float_registers.h
 * @brief Defines SunSpec Meter (Float) Model registers (M21X) for Fronius
 * devices.
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * meter model that uses floating-point registers. All measurements are already
 * scaled and can be used directly as physical values.
 *
 * Supported meter model IDs:
 *  - **211** – Single-phase
 *  - **212** – Split-phase
 *  - **213** – Three-phase
 */

#ifndef METER_FLOAT_REGISTERS_H_
#define METER_FLOAT_REGISTERS_H_

#include <cstdint>

/** @brief Total number of registers in the meter model (float). */
constexpr uint16_t M21X_SIZE = 124;

/**
 * @struct M21X_ID
 * @brief SunSpec meter model identifier (float).
 *
 * @return
 *  - 211: Single-phase
 *  - 212: Split-phase
 *  - 213: Three-phase
 */
struct M21X_ID {
  static constexpr uint16_t ADDR = 40070 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct M21X_L
 * @brief Length of meter model block.
 * @return Always 124
 */
struct M21X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC current ---

/** @struct M21X_A
 *  @brief AC total current
 *  @unit Amperes [A]
 */
struct M21X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_APHA
 *  @brief AC current phase A
 *  @unit Amperes [A]
 */
struct M21X_APHA {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_APHB
 *  @brief AC current phase B
 *  @unit Amperes [A]
 */
struct M21X_APHB {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_APHC
 *  @brief AC current phase C
 *  @unit Amperes [A]
 */
struct M21X_APHC {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 2;
};

// --- AC voltage ---

/** @struct M21X_PHV
 *  @brief Average AC voltage phase-to-neutral
 *  @unit Volts [V]
 */
struct M21X_PHV {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PHVPHA
 *  @brief AC voltage phase A to neutral
 *  @unit Volts [V]
 */
struct M21X_PHVPHA {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PHVPHB
 *  @brief AC voltage phase B to neutral
 *  @unit Volts [V]
 */
struct M21X_PHVPHB {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PHVPHC
 *  @brief AC voltage phase C to neutral
 *  @unit Volts [V]
 */
struct M21X_PHVPHC {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PPV
 *  @brief Average AC voltage phase-to-phase
 *  @unit Volts [V]
 */
struct M21X_PPV {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PPVPHAB
 *  @brief AC voltage phase AB
 *  @unit Volts [V]
 */
struct M21X_PPVPHAB {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PPVPHBC
 *  @brief AC voltage phase BC
 *  @unit Volts [V]
 */
struct M21X_PPVPHBC {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PPVPHCA
 *  @brief AC voltage phase CA
 *  @unit Volts [V]
 */
struct M21X_PPVPHCA {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 2;
};

// --- AC frequency ---

/** @struct M21X_HZ
 *  @brief AC frequency
 *  @unit Hertz [Hz]
 */
struct M21X_HZ {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 2;
};

// --- AC power ---

/** @struct M21X_W
 *  @brief Total AC active power
 *  @unit Watts [W]
 */
struct M21X_W {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_WPHA
 *  @brief AC active power phase A
 *  @unit Watts [W]
 */
struct M21X_WPHA {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_WPHB
 *  @brief AC active power phase B
 *  @unit Watts [W]
 */
struct M21X_WPHB {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_WPHC
 *  @brief AC active power phase C
 *  @unit Watts [W]
 */
struct M21X_WPHC {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 2;
};

// --- AC apparent power ---

/** @struct M21X_VA
 *  @brief Total AC apparent power
 *  @unit Volt-amperes [VA]
 */
struct M21X_VA {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_VAPHA
 *  @brief AC apparent power phase A
 *  @unit Volt-amperes [VA]
 */
struct M21X_VAPHA {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_VAPHB
 *  @brief AC apparent power phase B
 *  @unit Volt-amperes [VA]
 */
struct M21X_VAPHB {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_VAPHC
 *  @brief AC apparent power phase C
 *  @unit Volt-amperes [VA]
 */
struct M21X_VAPHC {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

// --- AC reactive power ---

/** @struct M21X_VAR
 *  @brief Total AC reactive power
 *  @unit Volt-ampere reactive [VAr]
 */
struct M21X_VAR {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_VARPHA
 *  @brief AC reactive power phase A
 *  @unit Volt-ampere reactive [VAr]
 */
struct M21X_VARPHA {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_VARPHB
 *  @brief AC reactive power phase B
 *  @unit Volt-ampere reactive [VAr]
 */
struct M21X_VARPHB {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_VARPHC
 *  @brief AC reactive power phase C
 *  @unit Volt-ampere reactive [VAr]
 */
struct M21X_VARPHC {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

// --- Power factor ---

/** @struct M21X_PF
 *  @brief Total power factor
 *  @unit Percent [%]
 */
struct M21X_PF {
  static constexpr uint16_t ADDR = 40122 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PFPHA
 *  @brief Power factor phase A
 *  @unit Percent [%]
 */
struct M21X_PFPHA {
  static constexpr uint16_t ADDR = 40124 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PFPHB
 *  @brief Power factor phase B
 *  @unit Percent [%]
 */
struct M21X_PFPHB {
  static constexpr uint16_t ADDR = 40126 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_PFPHC
 *  @brief Power factor phase C
 *  @unit Percent [%]
 */
struct M21X_PFPHC {
  static constexpr uint16_t ADDR = 40128 - 1;
  static constexpr uint16_t NB = 2;
};

// --- Energy registers ---

/** @struct M21X_TOTWH_EXP
 *  @brief Total energy exported
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_EXP {
  static constexpr uint16_t ADDR = 40130 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_EXPPHA
 *  @brief Total energy exported phase A
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_EXPPHA {
  static constexpr uint16_t ADDR = 40132 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_EXPPHB
 *  @brief Total energy exported phase B
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_EXPPHB {
  static constexpr uint16_t ADDR = 40134 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_EXPPHC
 *  @brief Total energy exported phase C
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_EXPPHC {
  static constexpr uint16_t ADDR = 40136 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_IMP
 *  @brief Total energy imported
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_IMP {
  static constexpr uint16_t ADDR = 40138 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_IMPPHA
 *  @brief Total energy imported phase A
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_IMPPHA {
  static constexpr uint16_t ADDR = 40140 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_IMPPHB
 *  @brief Total energy imported phase B
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_IMPPHB {
  static constexpr uint16_t ADDR = 40142 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTWH_IMPPHC
 *  @brief Total energy imported phase C
 *  @unit Watt-hours [Wh]
 */
struct M21X_TOTWH_IMPPHC {
  static constexpr uint16_t ADDR = 40144 - 1;
  static constexpr uint16_t NB = 2;
};

// --- Apparent energy registers ---

/** @struct M21X_TOTVAH_EXP
 *  @brief Total apparent energy exported
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_EXP {
  static constexpr uint16_t ADDR = 40146 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_EXPPHA
 *  @brief Total apparent energy exported phase A
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_EXPPHA {
  static constexpr uint16_t ADDR = 40148 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_EXPPHB
 *  @brief Total apparent energy exported phase B
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_EXPPHB {
  static constexpr uint16_t ADDR = 40150 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_EXPPHC
 *  @brief Total apparent energy exported phase C
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_EXPPHC {
  static constexpr uint16_t ADDR = 40152 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_IMP
 *  @brief Total apparent energy imported
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_IMP {
  static constexpr uint16_t ADDR = 40154 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_IMPPHA
 *  @brief Total apparent energy imported phase A
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_IMPPHA {
  static constexpr uint16_t ADDR = 40156 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_IMPPHB
 *  @brief Total apparent energy imported phase B
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_IMPPHB {
  static constexpr uint16_t ADDR = 40158 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M21X_TOTVAH_IMPPHC
 *  @brief Total apparent energy imported phase C
 *  @unit Volt-ampere hours [VAh]
 */
struct M21X_TOTVAH_IMPPHC {
  static constexpr uint16_t ADDR = 40160 - 1;
  static constexpr uint16_t NB = 2;
};

// --- Event registers ---

/** @struct M21X_EVT
 *  @brief Event flags
 */
struct M21X_EVT {
  static constexpr uint16_t ADDR = 40194 - 1;
  static constexpr uint16_t NB = 2;
};

// --- End-of-block registers ---

/** @struct E21X_ID
 *  @brief End block identifier
 *  @return Always 0xFFFF
 */
struct E21X_ID {
  static constexpr uint16_t ADDR = 40196 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct E21X_L
 *  @brief End block length
 *  @return Always 0
 */
struct E21X_L {
  static constexpr uint16_t ADDR = 40197 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* METER_FLOAT_REGISTERS_H_ */
