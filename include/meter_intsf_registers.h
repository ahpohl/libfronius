/**
 * @file meter_intsf_registers.h
 * @brief Defines SunSpec Meter (Integer + Scale Factor) Model registers (M20X)
 * for Fronius devices.
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * meter model that uses 16-bit integer registers with scale factors. These
 * registers provide AC electrical measurements, power, energy, power factors,
 * and event flags for Fronius meters.
 *
 * Supported meter model IDs:
 *  - **201** – Single-phase meter
 *  - **202** – Split-phase meter
 *  - **203** – Three-phase meter
 *
 * @note Many registers require applying the associated scale factor (e.g.,
 * A_SF, V_SF) to obtain the actual physical values.
 */

#ifndef METER_INTSF_REGISTERS_H_
#define METER_INTSF_REGISTERS_H_

#include <cstdint>

/** @brief Total number of registers in the meter model (integer + scale
 * factor). */
constexpr uint16_t M20X_SIZE = 105;

/** @struct M20X_ID
 *  @brief SunSpec meter model identifier (integer)
 *  @return
 *    - **201** – Single-phase meter
 *    - **202** – Split-phase meter
 *    - **203** – Three-phase meter
 */
struct M20X_ID {
  static constexpr uint16_t ADDR = 40070 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_L
 *  @brief Length of meter model block
 *  @return Always 105
 */
struct M20X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC current registers ---

/** @struct M20X_A
 *  @brief Total AC current
 *  @unit Amperes [A]
 */
struct M20X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_APHA
 *  @brief AC current phase A
 *  @unit Amperes [A]
 */
struct M20X_APHA {
  static constexpr uint16_t ADDR = 40073 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_APHB
 *  @brief AC current phase B
 *  @unit Amperes [A]
 */
struct M20X_APHB {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_APHC
 *  @brief AC current phase C
 *  @unit Amperes [A]
 */
struct M20X_APHC {
  static constexpr uint16_t ADDR = 40075 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_A_SF
 *  @brief AC current scale factor
 */
struct M20X_A_SF {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC voltage registers ---

/** @struct M20X_PHV
 *  @brief Average AC voltage phase-to-neutral
 *  @unit Volts [V]
 */
struct M20X_PHV {
  static constexpr uint16_t ADDR = 40077 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PHVPHA
 *  @brief AC voltage phase A to neutral
 *  @unit Volts [V]
 */
struct M20X_PHVPHA {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PHVPHB
 *  @brief AC voltage phase B to neutral
 *  @unit Volts [V]
 */
struct M20X_PHVPHB {
  static constexpr uint16_t ADDR = 40079 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PHVPHC
 *  @brief AC voltage phase C to neutral
 *  @unit Volts [V]
 */
struct M20X_PHVPHC {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PPV
 *  @brief Average AC voltage phase-to-phase
 *  @unit Volts [V]
 */
struct M20X_PPV {
  static constexpr uint16_t ADDR = 40081 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PPVPHAB
 *  @brief AC voltage phase AB
 *  @unit Volts [V]
 */
struct M20X_PPVPHAB {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PPVPHBC
 *  @brief AC voltage phase BC
 *  @unit Volts [V]
 */
struct M20X_PPVPHBC {
  static constexpr uint16_t ADDR = 40083 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PPVPHCA
 *  @brief AC voltage phase CA
 *  @unit Volts [V]
 */
struct M20X_PPVPHCA {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_V_SF
 *  @brief Voltage scale factor
 */
struct M20X_V_SF {
  static constexpr uint16_t ADDR = 40085 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC frequency registers ---

/** @struct M20X_HZ
 *  @brief AC frequency
 *  @unit Hertz [Hz]
 */
struct M20X_HZ {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_HZ_SF
 *  @brief Frequency scale factor
 */
struct M20X_HZ_SF {
  static constexpr uint16_t ADDR = 40087 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC active power registers ---

/** @struct M20X_W
 *  @brief Total AC active power
 *  @unit Watts [W]
 */
struct M20X_W {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_WPHA
 *  @brief AC active power phase A
 *  @unit Watts [W]
 */
struct M20X_WPHA {
  static constexpr uint16_t ADDR = 40089 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_WPHB
 *  @brief AC active power phase B
 *  @unit Watts [W]
 */
struct M20X_WPHB {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_WPHC
 *  @brief AC active power phase C
 *  @unit Watts [W]
 */
struct M20X_WPHC {
  static constexpr uint16_t ADDR = 40091 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_W_SF
 *  @brief Power scale factor
 */
struct M20X_W_SF {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC apparent power registers ---

/** @struct M20X_VA
 *  @brief Total AC apparent power
 *  @unit Volt-amperes [VA]
 */
struct M20X_VA {
  static constexpr uint16_t ADDR = 40093 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VAPHA
 *  @brief AC apparent power phase A
 *  @unit Volt-amperes [VA]
 */
struct M20X_VAPHA {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VAPHB
 *  @brief AC apparent power phase B
 *  @unit Volt-amperes [VA]
 */
struct M20X_VAPHB {
  static constexpr uint16_t ADDR = 40095 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VAPHC
 *  @brief AC apparent power phase C
 *  @unit Volt-amperes [VA]
 */
struct M20X_VAPHC {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VA_SF
 *  @brief Apparent power scale factor
 */
struct M20X_VA_SF {
  static constexpr uint16_t ADDR = 40097 - 1;
  static constexpr uint16_t NB = 1;
};

// --- AC reactive power registers ---

/** @struct M20X_VAR
 *  @brief Total AC reactive power
 *  @unit Volt-ampere reactive [VAr]
 */
struct M20X_VAR {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VARPHA
 *  @brief AC reactive power phase A
 *  @unit Volt-ampere reactive [VAr]
 */
struct M20X_VARPHA {
  static constexpr uint16_t ADDR = 40099 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VARPHB
 *  @brief AC reactive power phase B
 *  @unit Volt-ampere reactive [VAr]
 */
struct M20X_VARPHB {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VARPHC
 *  @brief AC reactive power phase C
 *  @unit Volt-ampere reactive [VAr]
 */
struct M20X_VARPHC {
  static constexpr uint16_t ADDR = 40101 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_VAR_SF
 *  @brief Reactive power scale factor
 */
struct M20X_VAR_SF {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 1;
};

// --- Power factor registers ---

/** @struct M20X_PF
 *  @brief Total power factor
 *  @unit Percent [%]
 */
struct M20X_PF {
  static constexpr uint16_t ADDR = 40103 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PFPHA
 *  @brief Power factor phase A
 *  @unit Percent [%]
 */
struct M20X_PFPHA {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PFPHB
 *  @brief Power factor phase B
 *  @unit Percent [%]
 */
struct M20X_PFPHB {
  static constexpr uint16_t ADDR = 40105 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PFPHC
 *  @brief Power factor phase C
 *  @unit Percent [%]
 */
struct M20X_PFPHC {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct M20X_PF_SF
 *  @brief Power factor scale factor
 */
struct M20X_PF_SF {
  static constexpr uint16_t ADDR = 40107 - 1;
  static constexpr uint16_t NB = 1;
};

// --- Energy registers ---

/** @struct M20X_TOTWH_EXP
 *  @brief Total energy exported
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_EXP {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_EXPPHA
 *  @brief Total energy exported phase A
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_EXPPHA {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_EXPPHB
 *  @brief Total energy exported phase B
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_EXPPHB {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_EXPPHC
 *  @brief Total energy exported phase C
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_EXPPHC {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_IMP
 *  @brief Total energy imported
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_IMP {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_IMPPHA
 *  @brief Total energy imported phase A
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_IMPPHA {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_IMPPHB
 *  @brief Total energy imported phase B
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_IMPPHB {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_IMPPHC
 *  @brief Total energy imported phase C
 *  @unit Watt-hours [Wh]
 */
struct M20X_TOTWH_IMPPHC {
  static constexpr uint16_t ADDR = 40122 - 1;
  static constexpr uint16_t NB = 2;
};

/** @struct M20X_TOTWH_SF
 *  @brief Energy scale factor
 */
struct M20X_TOTWH_SF {
  static constexpr uint16_t ADDR = 40124 - 1;
  static constexpr uint16_t NB = 1;
};

// --- Event registers ---

/** @struct M20X_EVT
 *  @brief Event flags
 */
struct M20X_EVT {
  static constexpr uint16_t ADDR = 40175 - 1;
  static constexpr uint16_t NB = 2;
};

// --- End-of-block registers ---

/** @struct E20X_ID
 *  @brief End block identifier
 *  @return Always 0xFFFF
 */
struct E20X_ID {
  static constexpr uint16_t ADDR = 40177 - 1;
  static constexpr uint16_t NB = 1;
};

/** @struct E20X_L
 *  @brief End block length
 *  @return Always 0
 */
struct E20X_L {
  static constexpr uint16_t ADDR = 40178 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* METER_INTSF_REGISTERS_H_ */
