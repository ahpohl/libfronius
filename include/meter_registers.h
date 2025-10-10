/**
 * @file meter_registers.h
 * @brief SunSpec Meter Model registers (Integer + Scale Factor and Float) for
 * Fronius devices.
 *
 * @details
 * This header consolidates the symbolic register definitions for Fronius
 * SunSpec meters, supporting both floating-point (M21X) and integer + scale
 * factor (M20X) models.
 *
 * The two namespaces represent the two different representations:
 *
 * - **M21X::** Floating-point registers (M21X)
 *   - Measurements are 32-bit IEEE floats and can generally be used directly as
 * physical values.
 *   - Supported meter model IDs:
 *     - 211 – Single-phase
 *     - 212 – Split-phase
 *     - 213 – Three-phase
 *
 * - **M20X::** Integer + scale factor registers (M20X)
 *   - Measurements are stored as 16-bit integers and require the associated
 * scale factor registers (e.g., A_SF, V_SF) to convert to physical units.
 *   - Supported meter model IDs:
 *     - 201 – Single-phase
 *     - 202 – Split-phase
 *     - 203 – Three-phase
 *
 * @note Many registers in the integer + scale factor model require applying the
 * scale factor to obtain correct physical values, while float registers do not.
 */

#ifndef METER_REGISTERS_H_
#define METER_REGISTERS_H_

#include <cstdint>

/**
 * @namespace M21X
 * @brief Contains floating-point SunSpec Meter registers for Fronius inverters.
 *
 * All measurements in this namespace are represented as 32-bit IEEE
 * floating-point values. Register addresses follow the standard SunSpec float
 * model layout.
 */
namespace M21X {

/**
 * @brief Total number of registers in the meter model (float).
 */
constexpr uint16_t SIZE = 124;

/**
 * @struct ID
 * @brief SunSpec meter model identifier (float).
 *
 * @return
 *  - 211: Single-phase
 *  - 212: Split-phase
 *  - 213: Three-phase
 */
struct ID {
  static constexpr uint16_t ADDR = 40069;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of meter model block.
 * @return Always 124
 */
struct L {
  static constexpr uint16_t ADDR = 40070;
  static constexpr uint16_t NB = 1;
};

// --- AC current ---

/**
 * @struct A
 * @brief AC total current
 * @unit Amperes [A]
 */
struct A {
  static constexpr uint16_t ADDR = 40071;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct APHA
 * @brief AC current phase A
 * @unit Amperes [A]
 */
struct APHA {
  static constexpr uint16_t ADDR = 40073;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct APHB
 * @brief AC current phase B
 * @unit Amperes [A]
 */
struct APHB {
  static constexpr uint16_t ADDR = 40075;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct APHC
 * @brief AC current phase C
 * @unit Amperes [A]
 */
struct APHC {
  static constexpr uint16_t ADDR = 40077;
  static constexpr uint16_t NB = 2;
};

// --- AC voltage ---

/**
 * @struct PHV
 * @brief Average AC voltage phase-to-neutral
 * @unit Volts [V]
 */
struct PHV {
  static constexpr uint16_t ADDR = 40079;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PHVPHA
 * @brief AC voltage phase A to neutral
 * @unit Volts [V]
 */
struct PHVPHA {
  static constexpr uint16_t ADDR = 40081;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PHVPHB
 * @brief AC voltage phase B to neutral
 * @unit Volts [V]
 */
struct PHVPHB {
  static constexpr uint16_t ADDR = 40083;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PHVPHC
 * @brief AC voltage phase C to neutral
 * @unit Volts [V]
 */
struct PHVPHC {
  static constexpr uint16_t ADDR = 40085;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPV
 * @brief Average AC voltage phase-to-phase
 * @unit Volts [V]
 */
struct PPV {
  static constexpr uint16_t ADDR = 40087;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPVPHAB
 * @brief AC voltage phase AB
 * @unit Volts [V]
 */
struct PPVPHAB {
  static constexpr uint16_t ADDR = 40089;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPVPHBC
 * @brief AC voltage phase BC
 * @unit Volts [V]
 */
struct PPVPHBC {
  static constexpr uint16_t ADDR = 40091;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPVPHCA
 * @brief AC voltage phase CA
 * @unit Volts [V]
 */
struct PPVPHCA {
  static constexpr uint16_t ADDR = 40093;
  static constexpr uint16_t NB = 2;
};

// --- AC frequency ---

/**
 * @struct FREQ
 * @brief AC frequency
 * @unit Hertz [Hz]
 */
struct FREQ {
  static constexpr uint16_t ADDR = 40095;
  static constexpr uint16_t NB = 2;
};

// --- AC power ---

/**
 * @struct W
 * @brief Total AC active power
 * @unit Watts [W]
 */
struct W {
  static constexpr uint16_t ADDR = 40097;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct WPHA
 * @brief AC active power phase A
 * @unit Watts [W]
 */
struct WPHA {
  static constexpr uint16_t ADDR = 40099;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct WPHB
 * @brief AC active power phase B
 * @unit Watts [W]
 */
struct WPHB {
  static constexpr uint16_t ADDR = 40101;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct WPHC
 * @brief AC active power phase C
 * @unit Watts [W]
 */
struct WPHC {
  static constexpr uint16_t ADDR = 40103;
  static constexpr uint16_t NB = 2;
};

// --- AC apparent power ---

/**
 * @struct VA
 * @brief Total AC apparent power
 * @unit Volt-amperes [VA]
 */
struct VA {
  static constexpr uint16_t ADDR = 40105;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VAPHA
 * @brief AC apparent power phase A
 * @unit Volt-amperes [VA]
 */
struct VAPHA {
  static constexpr uint16_t ADDR = 40107;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VAPHB
 * @brief AC apparent power phase B
 * @unit Volt-amperes [VA]
 */
struct VAPHB {
  static constexpr uint16_t ADDR = 40109;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VAPHC
 * @brief AC apparent power phase C
 * @unit Volt-amperes [VA]
 */
struct VAPHC {
  static constexpr uint16_t ADDR = 40111;
  static constexpr uint16_t NB = 2;
};

// --- AC reactive power ---

/**
 * @struct VAR
 * @brief Total AC reactive power
 * @unit Volt-ampere reactive [VAr]
 */
struct VAR {
  static constexpr uint16_t ADDR = 40113;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VARPHA
 * @brief AC reactive power phase A
 * @unit Volt-ampere reactive [VAr]
 */
struct VARPHA {
  static constexpr uint16_t ADDR = 40115;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VARPHB
 * @brief AC reactive power phase B
 * @unit Volt-ampere reactive [VAr]
 */
struct VARPHB {
  static constexpr uint16_t ADDR = 40117;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VARPHC
 * @brief AC reactive power phase C
 * @unit Volt-ampere reactive [VAr]
 */
struct VARPHC {
  static constexpr uint16_t ADDR = 40119;
  static constexpr uint16_t NB = 2;
};

// --- Power factor ---

/**
 * @struct PF
 * @brief Total power factor
 * @unit Percent [%]
 */
struct PF {
  static constexpr uint16_t ADDR = 40121;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PFPHA
 * @brief Power factor phase A
 * @unit Percent [%]
 */
struct PFPHA {
  static constexpr uint16_t ADDR = 40123;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PFPHB
 * @brief Power factor phase B
 * @unit Percent [%]
 */
struct PFPHB {
  static constexpr uint16_t ADDR = 40125;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PFPHC
 * @brief Power factor phase C
 * @unit Percent [%]
 */
struct PFPHC {
  static constexpr uint16_t ADDR = 40127;
  static constexpr uint16_t NB = 2;
};

// --- Energy registers ---

/**
 * @struct TOTWH_EXP
 * @brief Total AC energy exported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXP {
  static constexpr uint16_t ADDR = 40129;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_EXPPHA
 * @brief Total AC energy phase A exported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXPPHA {
  static constexpr uint16_t ADDR = 40131;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_EXPPHB
 * @brief Total AC energy phase B exported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXPPHB {
  static constexpr uint16_t ADDR = 40133;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_EXPPHC
 * @brief Total AC energy phase C exported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXPPHC {
  static constexpr uint16_t ADDR = 40135;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMP
 * @brief Total AC energy imported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMP {
  static constexpr uint16_t ADDR = 40137;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMPPHA
 * @brief Total AC energy phase A imported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMPPHA {
  static constexpr uint16_t ADDR = 40139;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMPPHB
 * @brief Total AC energy phase B imported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMPPHB {
  static constexpr uint16_t ADDR = 40141;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMPPHC
 * @brief Total AC energy phase C imported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMPPHC {
  static constexpr uint16_t ADDR = 40143;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_EXP
 * @brief Total AC apparent energy exported
 * @unit VA-hours [VA]
 */
struct TOTVAH_EXP {
  static constexpr uint16_t ADDR = 40145;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_EXPPHA
 * @brief Total AC apparent energy phase A exported
 * @unit VA-hours [VA]
 */
struct TOTVAH_EXPPHA {
  static constexpr uint16_t ADDR = 40147;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_EXPPHB
 * @brief Total AC apparent energy phase B exported
 * @unit VA-hours [VA]
 */
struct TOTVAH_EXPPHB {
  static constexpr uint16_t ADDR = 40149;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_EXPPHC
 * @brief Total AC apparent energy phase C exported
 * @unit VA-hours [VA]
 */
struct TOTVAH_EXPPHC {
  static constexpr uint16_t ADDR = 40151;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_IMP
 * @brief Total AC apparent energy imported
 * @unit VA-hours [VA]
 */
struct TOTVAH_IMP {
  static constexpr uint16_t ADDR = 40153;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_IMPPHA
 * @brief Total AC apparent energy phase A imported
 * @unit VA-hours [VA]
 */
struct TOTVAH_IMPPHA {
  static constexpr uint16_t ADDR = 40155;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_IMPPHB
 * @brief Total AC apparent energy phase B imported
 * @unit VA-hours [VA]
 */
struct TOTVAH_IMPPHB {
  static constexpr uint16_t ADDR = 40157;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTVAH_IMPPHC
 * @brief Total AC apparent energy phase C imported
 * @unit VA-hours [VA]
 */
struct TOTVAH_IMPPHC {
  static constexpr uint16_t ADDR = 40159;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct END_ID
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @returns Always returns 0xFFFF.
 */
struct END_ID {
  static constexpr uint16_t ADDR = 40195; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

/**
 * @struct END_L
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @returns Always returns 0.
 */
struct END_L {
  static constexpr uint16_t ADDR = 40196; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

} // namespace M21X

/**
 * @namespace M20X
 * @brief Contains integer + scale factor SunSpec Meter registers for Fronius
 * inverters.
 *
 * All measurements in this namespace are represented as 16-bit integers. Scale
 * factors must be applied to convert raw register values into physical units.
 * Register addresses follow the standard SunSpec integer + scale factor model
 * layout.
 */
namespace M20X {

/**
 * @brief Total number of registers in the meter model (integer + scale factor).
 */
constexpr uint16_t SIZE = 105;

/**
 * @struct ID
 * @brief SunSpec meter model identifier
 * @return
 * - 201 – Single-phase meter
 * - 202 – Split-phase meter
 * - 203 – Three-phase meter
 */
struct ID {
  static constexpr uint16_t ADDR = 40069;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of meter model block
 * @return Always 105
 */
struct L {
  static constexpr uint16_t ADDR = 40070;
  static constexpr uint16_t NB = 1;
};

// --- AC current registers ---

/**
 * @struct A
 * @brief Total AC current
 * @unit Amperes [A]
 */
struct A {
  static constexpr uint16_t ADDR = 40071;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct APHA
 * @brief AC current phase A
 * @unit Amperes [A]
 */
struct APHA {
  static constexpr uint16_t ADDR = 40072;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct APHB
 * @brief AC current phase B
 * @unit Amperes [A]
 */
struct APHB {
  static constexpr uint16_t ADDR = 40073;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct APHC
 * @brief AC current phase C
 * @unit Amperes [A]
 */
struct APHC {
  static constexpr uint16_t ADDR = 40074;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct A_SF
 * @brief AC current scale factor
 */
struct A_SF {
  static constexpr uint16_t ADDR = 40075;
  static constexpr uint16_t NB = 1;
};

// --- AC voltage registers ---

/**
 * @struct PHV
 * @brief Average AC voltage phase-to-neutral
 * @unit Volts [V]
 */
struct PHV {
  static constexpr uint16_t ADDR = 40076;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PHVPHA
 * @brief AC voltage phase A to neutral
 * @unit Volts [V]
 */
struct PHVPHA {
  static constexpr uint16_t ADDR = 40077;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PHVPHB
 * @brief AC voltage phase B to neutral
 * @unit Volts [V]
 */
struct PHVPHB {
  static constexpr uint16_t ADDR = 40078;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PHVPHC
 * @brief AC voltage phase C to neutral
 * @unit Volts [V]
 */
struct PHVPHC {
  static constexpr uint16_t ADDR = 40079;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPV
 * @brief Average AC voltage phase-to-phase
 * @unit Volts [V]
 */
struct PPV {
  static constexpr uint16_t ADDR = 40080;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPVPHAB
 * @brief AC voltage phase AB
 * @unit Volts [V]
 */
struct PPVPHAB {
  static constexpr uint16_t ADDR = 40081;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPVPHBC
 * @brief AC voltage phase BC
 * @unit Volts [V]
 */
struct PPVPHBC {
  static constexpr uint16_t ADDR = 40082;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPVPHCA
 * @brief AC voltage phase CA
 * @unit Volts [V]
 */
struct PPVPHCA {
  static constexpr uint16_t ADDR = 40083;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct V_SF
 * @brief Voltage scale factor
 */
struct V_SF {
  static constexpr uint16_t ADDR = 40084;
  static constexpr uint16_t NB = 1;
};

// --- AC frequency registers ---

/**
 * @struct FREQ
 * @brief AC frequency
 * @unit Hertz [Hz]
 */
struct FREQ {
  static constexpr uint16_t ADDR = 40085;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct FREQ_SF
 * @brief Frequency scale factor
 */
struct FREQ_SF {
  static constexpr uint16_t ADDR = 40086;
  static constexpr uint16_t NB = 1;
};

// --- AC active power registers ---

/**
 * @struct W
 * @brief Total AC active power
 * @unit Watts [W]
 */
struct W {
  static constexpr uint16_t ADDR = 40087;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WPHA
 * @brief AC active power phase A
 * @unit Watts [W]
 */
struct WPHA {
  static constexpr uint16_t ADDR = 40088;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WPHB
 * @brief AC active power phase B
 * @unit Watts [W]
 */
struct WPHB {
  static constexpr uint16_t ADDR = 40089;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WPHC
 * @brief AC active power phase C
 * @unit Watts [W]
 */
struct WPHC {
  static constexpr uint16_t ADDR = 40090;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct W_SF
 * @brief Power scale factor
 */
struct W_SF {
  static constexpr uint16_t ADDR = 40091;
  static constexpr uint16_t NB = 1;
};

// --- AC apparent power registers ---

/**
 * @struct VA
 * @brief Total AC apparent power
 * @unit Volt-amperes [VA]
 */
struct VA {
  static constexpr uint16_t ADDR = 40092;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VAPHA
 * @brief AC apparent power phase A
 * @unit Volt-amperes [VA]
 */
struct VAPHA {
  static constexpr uint16_t ADDR = 40093;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VAPHB
 * @brief AC apparent power phase B
 * @unit Volt-amperes [VA]
 */
struct VAPHB {
  static constexpr uint16_t ADDR = 40094;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VAPHC
 * @brief AC apparent power phase C
 * @unit Volt-amperes [VA]
 */
struct VAPHC {
  static constexpr uint16_t ADDR = 40095;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VA_SF
 * @brief Apparent power scale factor
 */
struct VA_SF {
  static constexpr uint16_t ADDR = 40096;
  static constexpr uint16_t NB = 1;
};

// --- AC reactive power registers ---

/**
 * @struct VAR
 * @brief Total AC reactive power
 * @unit Volt-ampere reactive [VAr]
 */
struct VAR {
  static constexpr uint16_t ADDR = 40097;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPHA
 * @brief AC reactive power phase A
 * @unit Volt-ampere reactive [VAr]
 */
struct VARPHA {
  static constexpr uint16_t ADDR = 40098;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPHB
 * @brief AC reactive power phase B
 * @unit Volt-ampere reactive [VAr]
 */
struct VARPHB {
  static constexpr uint16_t ADDR = 40099;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPHC
 * @brief AC reactive power phase C
 * @unit Volt-ampere reactive [VAr]
 */
struct VARPHC {
  static constexpr uint16_t ADDR = 40100;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VAR_SF
 * @brief Reactive power scale factor
 */
struct VAR_SF {
  static constexpr uint16_t ADDR = 40101;
  static constexpr uint16_t NB = 1;
};

// --- Power factor registers ---

/**
 * @struct PF
 * @brief Total power factor
 * @unit Percent [%]
 */
struct PF {
  static constexpr uint16_t ADDR = 40102;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFPHA
 * @brief Power factor phase A
 * @unit Percent [%]
 */
struct PFPHA {
  static constexpr uint16_t ADDR = 40103;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFPHB
 * @brief Power factor phase B
 * @unit Percent [%]
 */
struct PFPHB {
  static constexpr uint16_t ADDR = 40104;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFPHC
 * @brief Power factor phase C
 * @unit Percent [%]
 */
struct PFPHC {
  static constexpr uint16_t ADDR = 40105;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PF_SF
 * @brief Power factor scale factor
 */
struct PF_SF {
  static constexpr uint16_t ADDR = 40106;
  static constexpr uint16_t NB = 1;
};

// --- Energy registers ---

/**
 * @struct TOTWH_EXP
 * @brief Total energy exported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXP {
  static constexpr uint16_t ADDR = 40107;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_EXPPHA
 * @brief Total energy exported phase A
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXPPHA {
  static constexpr uint16_t ADDR = 40109;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_EXPPHB
 * @brief Total energy exported phase B
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXPPHB {
  static constexpr uint16_t ADDR = 40111;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_EXPPHC
 * @brief Total energy exported phase C
 * @unit Watt-hours [Wh]
 */
struct TOTWH_EXPPHC {
  static constexpr uint16_t ADDR = 40113;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMP
 * @brief Total energy imported
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMP {
  static constexpr uint16_t ADDR = 40115;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMPPHA
 * @brief Total energy imported phase A
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMPPHA {
  static constexpr uint16_t ADDR = 40117;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMPPHB
 * @brief Total energy imported phase B
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMPPHB {
  static constexpr uint16_t ADDR = 40119;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_IMPPHC
 * @brief Total energy imported phase C
 * @unit Watt-hours [Wh]
 */
struct TOTWH_IMPPHC {
  static constexpr uint16_t ADDR = 40121;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TOTWH_SF
 * @brief Energy scale factor
 */
struct TOTWH_SF {
  static constexpr uint16_t ADDR = 40123;
  static constexpr uint16_t NB = 1;
};

// --- Event registers ---

/**
 * @struct EVT
 * @brief Event flags
 */
struct EVT {
  static constexpr uint16_t ADDR = 40174;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct END_ID
 * @brief End block identifier
 * @return Always 0xFFFF
 */
struct END_ID {
  static constexpr uint16_t ADDR = 40176;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct END_L
 * @brief End block length
 * @return Always 0
 */
struct END_L {
  static constexpr uint16_t ADDR = 40177;
  static constexpr uint16_t NB = 1;
};

} // namespace M20X

#endif /* METER_REGISTERS_H_ */
