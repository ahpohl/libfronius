/**
 * @file inverter_float_registers.h
 * @brief Defines SunSpec Inverter (Float) Model registers (I11X) for Fronius
 * devices.
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * inverter model that uses 32-bit floating-point registers. These registers
 * provide AC/DC electrical measurements, power, energy, temperature, operating
 * state, and event flags for Fronius inverters.
 *
 * Supported inverter model IDs:
 *  - **111** – Single-phase inverter
 *  - **112** – Split-phase inverter
 *  - **113** – Three-phase inverter
 */

#ifndef INVERTER_FLOAT_REGISTERS_H_
#define INVERTER_FLOAT_REGISTERS_H_

#include <cstdint>

/**
 * @namespace I11X_FLOAT
 * @brief SunSpec Inverter (Float) Model registers (I11X) for Fronius devices
 *
 * This namespace contains symbolic register definitions for SunSpec inverter
 * models 111, 112, and 113 (single-phase, split-phase, and three-phase).
 * Registers provide AC/DC measurements, power, energy, temperature, operating
 * state, and event flags.
 */
namespace I11X_FLOAT {

/** @brief Total number of registers in the inverter model (float). */
constexpr uint16_t SIZE = 60;

/**
 * @struct ID
 * @brief SunSpec inverter model identifier (float).
 * @details Identifies the type of inverter register map. The value indicates
 * whether the device is single-, split-, or three-phase.
 * @returns
 *  - 111: Single-phase
 *  - 112: Split-phase
 *  - 113: Three-phase
 */
struct ID {
  static constexpr uint16_t ADDR = 40069;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of inverter model block.
 * @returns Always returns 60.
 */
struct L {
  static constexpr uint16_t ADDR = 40070;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct A
 * @brief AC total current.
 * @unit Ampere [A]
 */
struct A {
  static constexpr uint16_t ADDR = 40071;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct APHA
 * @brief AC current in phase A.
 * @unit Ampere [A]
 */
struct APHA {
  static constexpr uint16_t ADDR = 40073;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct APHB
 * @brief AC current in phase B.
 * @unit Ampere [A]
 */
struct APHB {
  static constexpr uint16_t ADDR = 40075;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct APHC
 * @brief AC current in phase C.
 * @unit Ampere [A]
 */
struct APHC {
  static constexpr uint16_t ADDR = 40077;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPVPHAB
 * @brief AC voltage phase-to-phase AB.
 * @unit Volt [V]
 */
struct PPVPHAB {
  static constexpr uint16_t ADDR = 40079;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPVPHBC
 * @brief AC voltage phase-to-phase BC.
 * @unit Volt [V]
 */
struct PPVPHBC {
  static constexpr uint16_t ADDR = 40081;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PPVPHCA
 * @brief AC voltage phase-to-phase CA.
 * @unit Volt [V]
 */
struct PPVPHCA {
  static constexpr uint16_t ADDR = 40083;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PHVPHA
 * @brief AC voltage phase-A to neutral.
 * @unit Volt [V]
 */
struct PHVPHA {
  static constexpr uint16_t ADDR = 40085;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PHVPHB
 * @brief AC voltage phase-B to neutral.
 * @unit Volt [V]
 */
struct PHVPHB {
  static constexpr uint16_t ADDR = 40087;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PHVPHC
 * @brief AC voltage phase-C to neutral.
 * @unit Volt [V]
 */
struct PHVPHC {
  static constexpr uint16_t ADDR = 40089;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct W
 * @brief AC total power.
 * @unit Watt [W]
 */
struct W {
  static constexpr uint16_t ADDR = 40091;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct HZ
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
struct HZ {
  static constexpr uint16_t ADDR = 40093;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VA
 * @brief Apparent power.
 * @unit Volt-ampere [VA]
 */
struct VA {
  static constexpr uint16_t ADDR = 40095;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct VAR
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
struct VAR {
  static constexpr uint16_t ADDR = 40097;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct PF
 * @brief Power factor.
 * @unit Percent [%]
 */
struct PF {
  static constexpr uint16_t ADDR = 40099;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct WH
 * @brief Lifetime energy production.
 * @unit Watt-hour [Wh]
 */
struct WH {
  static constexpr uint16_t ADDR = 40101;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct DCA
 * @brief DC current.
 * @unit Ampere [A]
 */
struct DCA {
  static constexpr uint16_t ADDR = 40103;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct DCV
 * @brief DC voltage.
 * @unit Volt [V]
 */
struct DCV {
  static constexpr uint16_t ADDR = 40105;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct DCW
 * @brief DC power.
 * @unit Watt [W]
 */
struct DCW {
  static constexpr uint16_t ADDR = 40107;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMPCAB
 * @brief Cabinet temperature.
 * @unit Celsius [°C]
 */
struct TMPCAB {
  static constexpr uint16_t ADDR = 40109;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMPSNK
 * @brief Heat sink or coolant temperature.
 * @unit Celsius [°C]
 */
struct TMPSNK {
  static constexpr uint16_t ADDR = 40111;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMPTRNS
 * @brief Transformer temperature.
 * @unit Celsius [°C]
 */
struct TMPTRNS {
  static constexpr uint16_t ADDR = 40113;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMPOT
 * @brief Other temperature (unspecified).
 * @unit Celsius [°C]
 */
struct TMPOT {
  static constexpr uint16_t ADDR = 40115;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct ST
 * @brief Inverter operating state.
 */
struct ST {
  static constexpr uint16_t ADDR = 40117;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STVND
 * @brief Vendor-defined operating state.
 */
struct STVND {
  static constexpr uint16_t ADDR = 40118;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct EVT1
 * @brief Event flags (bits 0–31).
 */
struct EVT1 {
  static constexpr uint16_t ADDR = 40119;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVT2
 * @brief Event flags (bits 32–63).
 */
struct EVT2 {
  static constexpr uint16_t ADDR = 40121;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND1
 * @brief Vendor-defined event flags (bits 0–31).
 */
struct EVTVND1 {
  static constexpr uint16_t ADDR = 40123;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND2
 * @brief Vendor-defined event flags (bits 32–63).
 */
struct EVTVND2 {
  static constexpr uint16_t ADDR = 40125;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND3
 * @brief Vendor-defined event flags (bits 64–95).
 */
struct EVTVND3 {
  static constexpr uint16_t ADDR = 40127;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND4
 * @brief Vendor-defined event flags (bits 96–127).
 */
struct EVTVND4 {
  static constexpr uint16_t ADDR = 40129;
  static constexpr uint16_t NB = 2;
};

} // namespace I11X_FLOAT

namespace I160_FLOAT {

/** @brief Total number of registers in the multiple MPPT model (float). */
constexpr uint16_t SIZE = 48;

/**
 * @struct ID
 * @brief Uniquely identifies this as a SunSpec Multiple MPPT Inverter Extension
 * Model.
 * @returns 160
 */
struct ID {
  static constexpr uint16_t REG = 40263;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of Multiple MPPT Inverter Extension Model.
 * @returns 48
 */
struct L {
  static constexpr uint16_t REG = 40264;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct EVT
 * @brief Global events register.
 */
struct EVT {
  static constexpr uint16_t REG = 40269;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct N
 * @brief Number of DC modules.
 * @returns 2
 */
struct N {
  static constexpr uint16_t REG = 40271;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_ID
 * @brief Input ID of first DC input.
 * @returns 1
 */
struct INPUT1_ID {
  static constexpr uint16_t REG = 40273;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_IDSTR
 * @brief Input ID string of first DC input.
 * @returns "String 1"
 */
struct INPUT1_IDSTR {
  static constexpr uint16_t REG = 40274;
  static constexpr uint16_t NB = 8;
};

/**
 * @struct INPUT1_DCA
 * @brief DC current of first input.
 * @unit Ampere [A]
 */
struct INPUT1_DCA {
  static constexpr uint16_t REG = 40282;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCV
 * @brief DC voltage of first input.
 * @unit Volt [V]
 */
struct INPUT1_DCV {
  static constexpr uint16_t REG = 40283;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCW
 * @brief DC power of first input.
 * @unit Watt [W]
 */
struct INPUT1_DCW {
  static constexpr uint16_t REG = 40284;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCWH
 * @brief DC lifetime energy of first input.
 * @note Not supported for Fronius Hybrid inverters
 * @unit Watt-hour [Wh]
 */
struct INPUT1_DCWH {
  static constexpr uint16_t REG = 40285;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT1_TMS
 * @brief Timestamp of first DC input since 01-Jan-2000 00:00 UTC.
 */
struct INPUT1_TMS {
  static constexpr uint16_t REG = 40287;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT1_TMP
 * @brief Temperature of first DC input.
 * @unit Celsius [°C]
 */
struct INPUT1_TMP {
  static constexpr uint16_t REG = 40289;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCST
 * @brief Operating state of first DC input.
 */
struct INPUT1_DCST {
  static constexpr uint16_t REG = 40290;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCEVT
 * @brief Module events of first DC input.
 */
struct INPUT1_DCEVT {
  static constexpr uint16_t REG = 40291;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT2_ID
 * @brief Input ID of second DC input.
 * @returns 2
 */
struct INPUT2_ID {
  static constexpr uint16_t REG = 40293;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_IDSTR
 * @brief Input ID string of second DC input.
 * @returns "String 2" or "not supported"
 */
struct INPUT2_IDSTR {
  static constexpr uint16_t REG = 40294;
  static constexpr uint16_t NB = 8;
};

/**
 * @struct INPUT2_DCA
 * @brief DC current of second input.
 * @unit Ampere [A]
 * @note Not supported if only one DC input
 */
struct INPUT2_DCA {
  static constexpr uint16_t REG = 40302;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCV
 * @brief DC voltage of second input.
 * @unit Volt [V]
 * @note Not supported if only one DC input
 */
struct INPUT2_DCV {
  static constexpr uint16_t REG = 40303;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCW
 * @brief DC power of second input.
 * @unit Watt [W]
 * @note Not supported if only one DC input
 */
struct INPUT2_DCW {
  static constexpr uint16_t REG = 40304;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCWH
 * @brief DC lifetime energy of second input.
 * @note Not supported for Fronius Hybrid inverters
 * @unit Watt-hour [Wh]
 */
struct INPUT2_DCWH {
  static constexpr uint16_t REG = 40305;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT2_TMS
 * @brief Timestamp of second DC input since 01-Jan-2000 00:00 UTC.
 * @note Not supported if only one DC input
 */
struct INPUT2_TMS {
  static constexpr uint16_t REG = 40307;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT2_TMP
 * @brief Temperature of second DC input.
 * @unit Celsius [°C]
 * @note Not supported if only one DC input
 */
struct INPUT2_TMP {
  static constexpr uint16_t REG = 40309;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCST
 * @brief Operating state of second DC input.
 * @note Not supported if only one DC input
 */
struct INPUT2_DCST {
  static constexpr uint16_t REG = 40310;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCEVT
 * @brief Module events of second DC input.
 * @note Not supported if only one DC input
 */
struct INPUT2_DCEVT {
  static constexpr uint16_t REG = 40311;
  static constexpr uint16_t NB = 2;
};

} // namespace I160_FLOAT

namespace END_FLOAT {

/**
 * @struct ID
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @returns Always returns 0xFFFF.
 */
struct ID {
  static constexpr uint16_t ADDR = 40339; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

/**
 * @struct L
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @returns Always returns 0.
 */
struct L {
  static constexpr uint16_t ADDR = 40340; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

} // namespace END_FLOAT

#endif /* INVERTER_FLOAT_REGISTERS_H_ */
