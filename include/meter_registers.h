/**
 * @file meter_registers.h
 * @brief SunSpec and proprietary meter register definitions for Fronius
 *        meters.
 *
 * @details
 * Three register namespaces:
 *
 * - **M20X** – SunSpec integer + scale-factor meter models (201–203):
 *              measurements as 16-bit integers paired with `*_SF` scale
 *              factors. IDs: 201 = single-phase, 202 = split-phase,
 *              203 = three-phase.
 * - **M21X** – SunSpec float meter models (211–213): measurements as
 *              32-bit IEEE floats; no scale factors needed.
 *              IDs: 211 = single-phase, 212 = split-phase,
 *              213 = three-phase.
 * - **REG**  – Fronius proprietary RTU register map for the
 *              Smart Meter TS 65A-3 (INT32 values with compile-time scale
 *              factors).
 *
 * The **M_END** namespace defines the end-of-map marker shared by both
 * SunSpec variants, and provides `FLOAT_OFFSET` to derive the float-map
 * end address from the integer-map end address.
 */

#ifndef METER_REGISTERS_H_
#define METER_REGISTERS_H_

#include "register_base.h"
#include <cstdint>

/**
 * @namespace M20X
 * @brief SunSpec meter model — integer + scale factor variant (201–203).
 *
 * Measurements are 16-bit integers; apply the corresponding `*_SF`
 * scale-factor register to obtain physical units.
 */
namespace M20X {

/** @brief Total number of registers in the meter model (integer + scale
 * factor). */
constexpr uint16_t SIZE = 105;

/**
 * @brief SunSpec meter model identifier
 * @return
 * - 201 – Single-phase meter
 * - 202 – Split-phase meter
 * - 203 – Three-phase meter
 */
constexpr Register ID(40069, 1, Register::Type::UINT16);

/**
 * @brief Length of meter model block
 * @return Always 105
 */
constexpr Register L(40070, 1, Register::Type::UINT16);

/**
 * @brief Total AC current
 * @unit Amperes [A]
 */
constexpr Register A(40071, 1, Register::Type::INT16);

/**
 * @brief AC current phase A
 * @unit Amperes [A]
 */
constexpr Register APHA(40072, 1, Register::Type::INT16);

/**
 * @brief AC current phase B
 * @unit Amperes [A]
 */
constexpr Register APHB(40073, 1, Register::Type::INT16);

/**
 * @brief AC current phase C
 * @unit Amperes [A]
 */
constexpr Register APHC(40074, 1, Register::Type::INT16);

/** @brief AC current scale factor */
constexpr Register A_SF(40075, 1, Register::Type::INT16);

/**
 * @brief Average AC voltage phase-to-neutral
 * @unit Volts [V]
 */
constexpr Register PHV(40076, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase A to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHA(40077, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase B to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHB(40078, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase C to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHC(40079, 1, Register::Type::INT16);

/**
 * @brief Average AC voltage phase-to-phase
 * @unit Volts [V]
 */
constexpr Register PPV(40080, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase AB
 * @unit Volts [V]
 */
constexpr Register PPVPHAB(40081, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase BC
 * @unit Volts [V]
 */
constexpr Register PPVPHBC(40082, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase CA
 * @unit Volts [V]
 */
constexpr Register PPVPHCA(40083, 1, Register::Type::INT16);

/** @brief Voltage scale factor */
constexpr Register V_SF(40084, 1, Register::Type::INT16);

/**
 * @brief AC frequency
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(40085, 1, Register::Type::INT16);

/** @brief Frequency scale factor */
constexpr Register FREQ_SF(40086, 1, Register::Type::INT16);

/**
 * @brief Total AC active power
 * @unit Watts [W]
 */
constexpr Register W(40087, 1, Register::Type::INT16);

/**
 * @brief AC active power phase A
 * @unit Watts [W]
 */
constexpr Register WPHA(40088, 1, Register::Type::INT16);

/**
 * @brief AC active power phase B
 * @unit Watts [W]
 */
constexpr Register WPHB(40089, 1, Register::Type::INT16);

/**
 * @brief AC active power phase C
 * @unit Watts [W]
 */
constexpr Register WPHC(40090, 1, Register::Type::INT16);

/** @brief Power scale factor */
constexpr Register W_SF(40091, 1, Register::Type::INT16);

/**
 * @brief Total AC apparent power
 * @unit Volt-amperes [VA]
 */
constexpr Register VA(40092, 1, Register::Type::INT16);

/**
 * @brief AC apparent power phase A
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHA(40093, 1, Register::Type::INT16);

/**
 * @brief AC apparent power phase B
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHB(40094, 1, Register::Type::INT16);

/**
 * @brief AC apparent power phase C
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHC(40095, 1, Register::Type::INT16);

/** @brief Apparent power scale factor */
constexpr Register VA_SF(40096, 1, Register::Type::INT16);

/**
 * @brief Total AC reactive power
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(40097, 1, Register::Type::INT16);

/**
 * @brief AC reactive power phase A
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHA(40098, 1, Register::Type::INT16);

/**
 * @brief AC reactive power phase B
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHB(40099, 1, Register::Type::INT16);

/**
 * @brief AC reactive power phase C
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHC(40100, 1, Register::Type::INT16);

/** @brief Reactive power scale factor */
constexpr Register VAR_SF(40101, 1, Register::Type::INT16);

/**
 * @brief Total power factor
 * @unit Percent [%]
 */
constexpr Register PF(40102, 1, Register::Type::INT16);

/**
 * @brief Power factor phase A
 * @unit Percent [%]
 */
constexpr Register PFPHA(40103, 1, Register::Type::INT16);

/**
 * @brief Power factor phase B
 * @unit Percent [%]
 */
constexpr Register PFPHB(40104, 1, Register::Type::INT16);

/**
 * @brief Power factor phase C
 * @unit Percent [%]
 */
constexpr Register PFPHC(40105, 1, Register::Type::INT16);

/** @brief Power factor scale factor */
constexpr Register PF_SF(40106, 1, Register::Type::INT16);

/**
 * @brief Total energy exported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXP(40107, 2, Register::Type::UINT32);

/**
 * @brief Total energy exported phase A
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXPPHA(40109, 2, Register::Type::UINT32);

/**
 * @brief Total energy exported phase B
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXPPHB(40111, 2, Register::Type::UINT32);

/**
 * @brief Total energy exported phase C
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXPPHC(40113, 2, Register::Type::UINT32);

/**
 * @brief Total energy imported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMP(40115, 2, Register::Type::UINT32);

/**
 * @brief Total energy imported phase A
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMPPHA(40117, 2, Register::Type::UINT32);

/**
 * @brief Total energy imported phase B
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMPPHB(40119, 2, Register::Type::UINT32);

/**
 * @brief Total energy imported phase C
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMPPHC(40121, 2, Register::Type::UINT32);

/** @brief Active energy scale factor */
constexpr Register TOT_WH_SF(40123, 1, Register::Type::INT16);

/**
 * @brief Total apparent energy exported
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_EXP(40124, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy exported phase A
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_EXPPHA(40126, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy exported phase B
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_EXPPHB(40128, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy exported phase C
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_EXPPHC(40130, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy imported
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_IMP(40132, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy imported phase A
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_IMPPHA(40134, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy imported phase B
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_IMPPHB(40136, 2, Register::Type::UINT32);

/**
 * @brief Total apparent energy imported phase C
 * @unit Volt-ampere-hours [VAh]
 */
constexpr Register TOT_VAH_IMPPHC(40138, 2, Register::Type::UINT32);

/** @brief Apparent energy scale factor */
constexpr Register TOT_VAH_SF(40140, 1, Register::Type::INT16);

/** @brief Event flags */
constexpr Register EVT(40174, 2, Register::Type::UINT32);

} // namespace M20X

/**
 * @namespace M21X
 * @brief SunSpec meter model — float variant (211–213).
 *
 * Measurements are 32-bit IEEE floats; no scale factors required.
 */
namespace M21X {

/** @brief Total number of registers in the meter model (float). */
constexpr uint16_t SIZE = 124;

/**
 * @brief SunSpec meter model identifier (float variant).
 * @returns 211 (single-phase), 212 (split-phase), or 213 (three-phase).
 */
constexpr Register ID(40069, 1, Register::Type::UINT16);

/**
 * @brief Length of meter model block.
 * @return Always 124
 */
constexpr Register L(40070, 1, Register::Type::UINT16);

/**
 * @brief AC total current
 * @unit Amperes [A]
 */
constexpr Register A(40071, 2, Register::Type::FLOAT);

/**
 * @brief AC current phase A
 * @unit Amperes [A]
 */
constexpr Register APHA(40073, 2, Register::Type::FLOAT);

/**
 * @brief AC current phase B
 * @unit Amperes [A]
 */
constexpr Register APHB(40075, 2, Register::Type::FLOAT);

/**
 * @brief AC current phase C
 * @unit Amperes [A]
 */
constexpr Register APHC(40077, 2, Register::Type::FLOAT);

/**
 * @brief Average AC voltage phase-to-neutral
 * @unit Volts [V]
 */
constexpr Register PHV(40079, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase A to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHA(40081, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase B to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHB(40083, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase C to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHC(40085, 2, Register::Type::FLOAT);

/**
 * @brief Average AC voltage phase-to-phase
 * @unit Volts [V]
 */
constexpr Register PPV(40087, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase AB
 * @unit Volts [V]
 */
constexpr Register PPVPHAB(40089, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase BC
 * @unit Volts [V]
 */
constexpr Register PPVPHBC(40091, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase CA
 * @unit Volts [V]
 */
constexpr Register PPVPHCA(40093, 2, Register::Type::FLOAT);

/**
 * @brief AC frequency
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(40095, 2, Register::Type::FLOAT);

/**
 * @brief Total AC active power
 * @unit Watts [W]
 */
constexpr Register W(40097, 2, Register::Type::FLOAT);

/**
 * @brief AC active power phase A
 * @unit Watts [W]
 */
constexpr Register WPHA(40099, 2, Register::Type::FLOAT);

/**
 * @brief AC active power phase B
 * @unit Watts [W]
 */
constexpr Register WPHB(40101, 2, Register::Type::FLOAT);

/**
 * @brief AC active power phase C
 * @unit Watts [W]
 */
constexpr Register WPHC(40103, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent power
 * @unit Volt-amperes [VA]
 */
constexpr Register VA(40105, 2, Register::Type::FLOAT);

/**
 * @brief AC apparent power phase A
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHA(40107, 2, Register::Type::FLOAT);

/**
 * @brief AC apparent power phase B
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHB(40109, 2, Register::Type::FLOAT);

/**
 * @brief AC apparent power phase C
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHC(40111, 2, Register::Type::FLOAT);

/**
 * @brief Total AC reactive power
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(40113, 2, Register::Type::FLOAT);

/**
 * @brief AC reactive power phase A
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHA(40115, 2, Register::Type::FLOAT);

/**
 * @brief AC reactive power phase B
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHB(40117, 2, Register::Type::FLOAT);

/**
 * @brief AC reactive power phase C
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHC(40119, 2, Register::Type::FLOAT);

/**
 * @brief Total power factor
 * @unit Percent [%]
 */
constexpr Register PF(40121, 2, Register::Type::FLOAT);

/**
 * @brief Power factor phase A
 * @unit Percent [%]
 */
constexpr Register PFPHA(40123, 2, Register::Type::FLOAT);

/**
 * @brief Power factor phase B
 * @unit Percent [%]
 */
constexpr Register PFPHB(40125, 2, Register::Type::FLOAT);

/**
 * @brief Power factor phase C
 * @unit Percent [%]
 */
constexpr Register PFPHC(40127, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy exported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXP(40129, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy phase A exported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXPPHA(40131, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy phase B exported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXPPHB(40133, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy phase C exported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_EXPPHC(40135, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy imported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMP(40137, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy phase A imported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMPPHA(40139, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy phase B imported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMPPHB(40141, 2, Register::Type::FLOAT);

/**
 * @brief Total AC energy phase C imported
 * @unit Watt-hours [Wh]
 */
constexpr Register TOT_WH_IMPPHC(40143, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy exported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_EXP(40145, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy phase A exported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_EXPPHA(40147, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy phase B exported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_EXPPHB(40149, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy phase C exported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_EXPPHC(40151, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy imported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_IMP(40153, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy phase A imported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_IMPPHA(40155, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy phase B imported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_IMPPHB(40157, 2, Register::Type::FLOAT);

/**
 * @brief Total AC apparent energy phase C imported
 * @unit VA-hours [VA]
 */
constexpr Register TOT_VAH_IMPPHC(40159, 2, Register::Type::FLOAT);

/** @brief Event flags */
constexpr Register EVT(40193, 2, Register::Type::UINT32);

} // namespace M21X

/**
 * @namespace M_END
 * @brief SunSpec end-of-map marker for the meter map.
 *
 * Two registers (`ID = 0xFFFF`, `L = 0`) terminate the SunSpec register
 * map. Defines `FLOAT_OFFSET` to derive the float-map address from the
 * integer-map address.
 */
namespace M_END {

/**
 * @brief Offset between integer-map and float-map end-block addresses.
 *
 * The float meter model is longer than the integer model; add this offset
 * to the integer-map end address to obtain the float-map end address.
 */
constexpr uint16_t FLOAT_OFFSET = 19;

/**
 * @brief End-of-block identifier.
 * @return Always 0xFFFF.
 */
constexpr Register ID(40176, 1, Register::Type::UINT16);

/**
 * @brief End-of-block length field.
 * @return Always 0.
 */
constexpr Register L(40177, 1, Register::Type::UINT16);

} // namespace M_END

/**
 * @namespace REG
 * @brief Fronius proprietary RTU register map for the Smart Meter
 *        TS 65A-3.
 *
 * Used when no SunSpec common block is present at the standard address
 * range. Values are stored as INT32 with a swapped word order; scale
 * factors are compile-time `double` constants (e.g. `A_SF`, `V_SF`)
 * rather than separate registers.
 *
 * Total energy counters are split across two INT32 register pairs (a
 * kWh/kVArh component plus a Wh/VArh remainder) — see the per-register
 * notes for how to combine them.
 */
namespace REG {

/**
 * @brief Proprietary device-type identifier.
 *
 * Reads as `731` on a Smart Meter TS 65A-3.
 */
constexpr Register ID(11, 1, Register::Type::UINT16);

/**
 * @brief Device serial number.
 *
 * Two consecutive registers; read as a 32-bit unsigned integer (decimal).
 */
constexpr Register SN(20487, 1, Register::Type::UINT32);

/**
 * @brief Meter firmware version — major component.
 *
 * Read together with `VR_MINOR` (the immediately following register) and
 * formatted as `"<major>.<minor>"`.
 */
constexpr Register VR_MAJOR(770, 1, Register::Type::UINT16);

/**
 * @brief Meter firmware version — minor component.
 * @see VR_MAJOR
 */
constexpr Register VR_MINOR(771, 1, Register::Type::UINT16);

/**
 * @brief Total AC current
 * @unit Amperes [A]
 */
constexpr Register A(270, 2, Register::Type::INT32);

/**
 * @brief AC current phase A
 * @unit Amperes [A]
 */
constexpr Register APHA(290, 2, Register::Type::INT32);

/**
 * @brief AC current phase B
 * @unit Amperes [A]
 */
constexpr Register APHB(304, 2, Register::Type::INT32);

/**
 * @brief AC current phase C
 * @unit Amperes [A]
 */
constexpr Register APHC(318, 2, Register::Type::INT32);

/** @brief AC current scale factor */
constexpr double A_SF = 0.001;

/**
 * @brief Average AC voltage phase-to-neutral
 * @unit Volts [V]
 */
constexpr Register PHV(258, 2, Register::Type::INT32);

/**
 * @brief AC voltage phase A to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHA(288, 2, Register::Type::INT32);

/**
 * @brief AC voltage phase B to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHB(302, 2, Register::Type::INT32);

/**
 * @brief AC voltage phase C to neutral
 * @unit Volts [V]
 */
constexpr Register PHVPHC(316, 2, Register::Type::INT32);

/**
 * @brief Average AC voltage phase-to-phase
 * @unit Volts [V]
 */
constexpr Register PPV(260, 2, Register::Type::INT32);

/**
 * @brief AC voltage phase AB
 * @unit Volts [V]
 */
constexpr Register PPVPHAB(286, 2, Register::Type::INT32);

/**
 * @brief AC voltage phase BC
 * @unit Volts [V]
 */
constexpr Register PPVPHBC(300, 2, Register::Type::INT32);

/**
 * @brief AC voltage phase CA
 * @unit Volts [V]
 */
constexpr Register PPVPHCA(314, 2, Register::Type::INT32);

/** @brief Voltage scale factor */
constexpr double V_SF = 0.1;

/**
 * @brief AC frequency
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(272, 2, Register::Type::INT32);

/** @brief Frequency scale factor */
constexpr double FREQ_SF = 0.1;

/**
 * @brief Total AC active power
 * @unit Watts [W]
 */
constexpr Register W(262, 2, Register::Type::INT32);

/**
 * @brief AC active power phase A
 * @unit Watts [W]
 */
constexpr Register WPHA(292, 2, Register::Type::INT32);

/**
 * @brief AC active power phase B
 * @unit Watts [W]
 */
constexpr Register WPHB(306, 2, Register::Type::INT32);

/**
 * @brief AC active power phase C
 * @unit Watts [W]
 */
constexpr Register WPHC(320, 2, Register::Type::INT32);

/** @brief Power scale factor */
constexpr double W_SF = 0.1;

/**
 * @brief Total AC apparent power
 * @unit Volt-amperes [VA]
 */
constexpr Register VA(264, 2, Register::Type::INT32);

/**
 * @brief AC apparent power phase A
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHA(294, 2, Register::Type::INT32);

/**
 * @brief AC apparent power phase B
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHB(308, 2, Register::Type::INT32);

/**
 * @brief AC apparent power phase C
 * @unit Volt-amperes [VA]
 */
constexpr Register VAPHC(322, 2, Register::Type::INT32);

/** @brief Apparent power scale factor */
constexpr double VA_SF = 0.1;

/**
 * @brief Total AC reactive power
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(266, 2, Register::Type::INT32);

/**
 * @brief AC reactive power phase A
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHA(296, 2, Register::Type::INT32);

/**
 * @brief AC reactive power phase B
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHB(310, 2, Register::Type::INT32);

/**
 * @brief AC reactive power phase C
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARPHC(324, 2, Register::Type::INT32);

/** @brief Reactive power scale factor */
constexpr double VAR_SF = 0.1;

/**
 * @brief Total power factor
 * @unit Percent [%]
 */
constexpr Register PF(268, 2, Register::Type::INT32);

/**
 * @brief Power factor phase A
 * @unit Percent [%]
 */
constexpr Register PFPHA(298, 2, Register::Type::INT32);

/**
 * @brief Power factor phase B
 * @unit Percent [%]
 */
constexpr Register PFPHB(312, 2, Register::Type::INT32);

/**
 * @brief Power factor phase C
 * @unit Percent [%]
 */
constexpr Register PFPHC(326, 2, Register::Type::INT32);

/** @brief Power factor scale factor */
constexpr double PF_SF = 0.1;

/**
 * @brief Imported active energy — kWh component.
 *
 * @details
 * The proprietary RTU map stores total energy as a pair of INT32 registers
 * (a kWh component plus a Wh remainder). Reconstruct the watt-hour total
 * with: `total_Wh = TOT_KWH_IMP × 1000 + TOT_WH_IMP`. The same pattern
 * applies to the EXP variants and to the kVArh/VArh reactive-energy
 * pairs.
 *
 * @unit Kilowatt-hour [kWh]
 */
constexpr Register TOT_KWH_IMP(1024, 2, Register::Type::INT32);

/**
 * @brief Imported active energy — Wh remainder. Combine with `TOT_KWH_IMP`.
 * @unit Watt-hour [Wh]
 */
constexpr Register TOT_WH_IMP(1026, 2, Register::Type::INT32);

/**
 * @brief Imported reactive energy — kVArh component.
 *        Combine with `TOT_VARH_IMP`.
 * @unit Kilovolt-ampere-reactive-hour [kVArh]
 */
constexpr Register TOT_KVARH_IMP(1028, 2, Register::Type::INT32);

/**
 * @brief Imported reactive energy — VArh remainder.
 *        Combine with `TOT_KVARH_IMP`.
 * @unit Volt-ampere-reactive-hour [VArh]
 */
constexpr Register TOT_VARH_IMP(1030, 2, Register::Type::INT32);

/**
 * @brief Exported active energy — kWh component. Combine with `TOT_WH_EXP`.
 * @unit Kilowatt-hour [kWh]
 */
constexpr Register TOT_KWH_EXP(1032, 2, Register::Type::INT32);

/**
 * @brief Exported active energy — Wh remainder. Combine with `TOT_KWH_EXP`.
 * @unit Watt-hour [Wh]
 */
constexpr Register TOT_WH_EXP(1034, 2, Register::Type::INT32);

/**
 * @brief Exported reactive energy — kVArh component.
 *        Combine with `TOT_VARH_EXP`.
 * @unit Kilovolt-ampere-reactive-hour [kVArh]
 */
constexpr Register TOT_KVARH_EXP(1036, 2, Register::Type::INT32);

/**
 * @brief Exported reactive energy — VArh remainder.
 *        Combine with `TOT_KVARH_EXP`.
 * @unit Volt-ampere-reactive-hour [VArh]
 */
constexpr Register TOT_VARH_EXP(1038, 2, Register::Type::INT32);

/**
 * @brief Reserved scale factor for the energy-pair sum.
 *
 * The factor of 1000 used to combine kWh/kVArh and Wh/VArh components is
 * applied directly in code; this constant is reserved for any further
 * scaling applied on top of that conversion.
 */
constexpr double TOT_SF = 1.0;

} // namespace REG

#endif /* METER_REGISTERS_H_ */
