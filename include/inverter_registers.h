/**
 * @file inverter_registers.h
 * @brief Defines SunSpec Inverter Model registers (I10X and I11X) for Fronius
 * devices.
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * inverter models implemented by Fronius. It consolidates both the integer (+
 * scale factor) and floating-point variants into one file.
 *
 * - The **I10X** namespace contains integer-based register definitions with
 * scale factors. These models represent the SunSpec Inverter Integer + Scale
 * Factor types:
 *     - **101** – Single-phase inverter
 *     - **102** – Split-phase inverter
 *     - **103** – Three-phase inverter
 *
 * - The **I11X** namespace contains 32-bit floating-point register definitions.
 *   These models represent the SunSpec Inverter Float types:
 *     - **111** – Single-phase inverter
 *     - **112** – Split-phase inverter
 *     - **113** – Three-phase inverter
 *
 * Both namespaces define registers for AC/DC measurements, power, energy,
 * temperature, operating state, and event flags, following the SunSpec
 * specification.
 */

#ifndef INVERTER_REGISTERS_H_
#define INVERTER_REGISTERS_H_

#include <cstdint>

/**
 * @namespace I11X
 * @brief SunSpec Inverter (Float) Model registers.
 *
 * This namespace provides register mappings for SunSpec inverter models
 * 111–113, which use 32-bit floating-point values for all primary quantities.
 * These registers cover AC/DC electrical data, power and energy values,
 * temperatures, inverter state, and event flags.
 */
namespace I11X {

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
 * @struct FREQ
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
struct FREQ {
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

/**
 * @struct END_ID
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @returns Always returns 0xFFFF.
 */
struct END_ID {
  static constexpr uint16_t ADDR = 40339; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

/**
 * @struct END_L
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @returns Always returns 0.
 */
struct END_L {
  static constexpr uint16_t ADDR = 40340; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

} // namespace I11X

/**
 * @namespace I10X
 * @brief SunSpec Inverter (Integer + Scale Factor) Model registers.
 *
 * This namespace provides register mappings for SunSpec inverter models
 * 101–103, which use integer values with associated scale-factor registers to
 * obtain physically meaningful quantities. These registers cover AC/DC
 * electrical data, power and energy values, temperatures, inverter state, and
 * event flags.
 */
namespace I10X {

/** @brief Total number of registers in the inverter model (integer + scale
 * factor). */
constexpr uint16_t SIZE = 50;

/**
 * @struct ID
 * @brief SunSpec inverter model identifier (integer + scale factor).
 * @returns
 *  - 101: Single-phase
 *  - 102: Split-phase
 *  - 103: Three-phase
 */
struct ID {
  static constexpr uint16_t ADDR = 40069;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of inverter model block.
 * @returns Always returns 50.
 */
struct L {
  static constexpr uint16_t ADDR = 40070;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct A
 * @brief AC total current.
 * @unit Amperes [A]
 */
struct A {
  static constexpr uint16_t ADDR = 40071;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct APHA
 * @brief AC current in phase A.
 * @unit Amperes [A]
 */
struct APHA {
  static constexpr uint16_t ADDR = 40072;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct APHB
 * @brief AC current in phase B.
 * @unit Amperes [A]
 */
struct APHB {
  static constexpr uint16_t ADDR = 40073;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct APHC
 * @brief AC current in phase C.
 * @unit Amperes [A]
 */
struct APHC {
  static constexpr uint16_t ADDR = 40074;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct A_SF
 * @brief AC current scale factor.
 */
struct A_SF {
  static constexpr uint16_t ADDR = 40075;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPVPHAB
 * @brief AC voltage phase-to-phase AB.
 * @unit Volts [V]
 */
struct PPVPHAB {
  static constexpr uint16_t ADDR = 40076;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPVPHBC
 * @brief AC voltage phase-to-phase BC.
 * @unit Volts [V]
 */
struct PPVPHBC {
  static constexpr uint16_t ADDR = 40077;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PPVPHCA
 * @brief AC voltage phase-to-phase CA.
 * @unit Volts [V]
 */
struct PPVPHCA {
  static constexpr uint16_t ADDR = 40078;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PHVPHA
 * @brief AC voltage phase A to neutral.
 * @unit Volts [V]
 */
struct PHVPHA {
  static constexpr uint16_t ADDR = 40079;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PHVPHB
 * @brief AC voltage phase B to neutral.
 * @unit Volts [V]
 */
struct PHVPHB {
  static constexpr uint16_t ADDR = 40080;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PHVPHC
 * @brief AC voltage phase C to neutral.
 * @unit Volts [V]
 */
struct PHVPHC {
  static constexpr uint16_t ADDR = 40081;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct V_SF
 * @brief AC voltage scale factor.
 */
struct V_SF {
  static constexpr uint16_t ADDR = 40082;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct W
 * @brief AC active power.
 * @unit Watts [W]
 */
struct W {
  static constexpr uint16_t ADDR = 40083;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct W_SF
 * @brief AC active power scale factor.
 */
struct W_SF {
  static constexpr uint16_t ADDR = 40084;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct FREQ
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
struct FREQ {
  static constexpr uint16_t ADDR = 40085;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct FREQ_SF
 * @brief AC frequency scale factor.
 */
struct FREQ_SF {
  static constexpr uint16_t ADDR = 40086;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VA
 * @brief Apparent power.
 * @unit Volt-amperes [VA]
 */
struct VA {
  static constexpr uint16_t ADDR = 40087;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VA_SF
 * @brief Apparent power scale factor.
 */
struct VA_SF {
  static constexpr uint16_t ADDR = 40088;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VAR
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
struct VAR {
  static constexpr uint16_t ADDR = 40089;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VAR_SF
 * @brief Reactive power scale factor.
 */
struct VAR_SF {
  static constexpr uint16_t ADDR = 40090;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PF
 * @brief Power factor.
 * @unit Percent [%]
 */
struct PF {
  static constexpr uint16_t ADDR = 40091;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PF_SF
 * @brief Power factor scale factor.
 */
struct PF_SF {
  static constexpr uint16_t ADDR = 40092;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WH
 * @brief AC lifetime energy production.
 * @unit Watt-hours [Wh]
 */
struct WH {
  static constexpr uint16_t ADDR = 40093;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct WH_SF
 * @brief AC lifetime energy scale factor.
 */
struct WH_SF {
  static constexpr uint16_t ADDR = 40095;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCA
 * @brief DC current.
 * @unit Amperes [A]
 */
struct DCA {
  static constexpr uint16_t ADDR = 40096;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCA_SF
 * @brief DC current scale factor.
 */
struct DCA_SF {
  static constexpr uint16_t ADDR = 40097;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCV
 * @brief DC voltage.
 * @unit Volts [V]
 */
struct DCV {
  static constexpr uint16_t ADDR = 40098;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCV_SF
 * @brief DC voltage scale factor.
 */
struct DCV_SF {
  static constexpr uint16_t ADDR = 40099;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCW
 * @brief DC power.
 * @unit Watts [W]
 */
struct DCW {
  static constexpr uint16_t ADDR = 40100;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCW_SF
 * @brief DC power scale factor.
 */
struct DCW_SF {
  static constexpr uint16_t ADDR = 40101;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct TMPCAB
 * @brief Cabinet temperature.
 * @unit Celsius [°C]
 */
struct TMPCAB {
  static constexpr uint16_t ADDR = 40102;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct TMPSNK
 * @brief Heat sink/coolant temperature.
 * @unit Celsius [°C]
 */
struct TMPSNK {
  static constexpr uint16_t ADDR = 40103;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct TMPTRNS
 * @brief Transformer temperature.
 * @unit Celsius [°C]
 */
struct TMPTRNS {
  static constexpr uint16_t ADDR = 40104;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct TMPOT
 * @brief Other temperature.
 * @unit Celsius [°C]
 */
struct TMPOT {
  static constexpr uint16_t ADDR = 40105;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct TMP_SF
 * @brief Temperature scale factor.
 */
struct TMP_SF {
  static constexpr uint16_t ADDR = 40106;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ST
 * @brief Inverter operating state.
 */
struct ST {
  static constexpr uint16_t ADDR = 40107;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STVND
 * @brief Vendor-defined operating state.
 */
struct STVND {
  static constexpr uint16_t ADDR = 40108;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct EVT1
 * @brief Event flags bits 0–31.
 */
struct EVT1 {
  static constexpr uint16_t ADDR = 40109;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVT2
 * @brief Event flags bits 32–63.
 */
struct EVT2 {
  static constexpr uint16_t ADDR = 40111;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND1
 * @brief Vendor-defined event flags bits 0–31.
 */
struct EVTVND1 {
  static constexpr uint16_t ADDR = 40113;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND2
 * @brief Vendor-defined event flags bits 32–63.
 */
struct EVTVND2 {
  static constexpr uint16_t ADDR = 40115;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND3
 * @brief Vendor-defined event flags bits 64–95.
 */
struct EVTVND3 {
  static constexpr uint16_t ADDR = 40117;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct EVTVND4
 * @brief Vendor-defined event flags bits 96–127.
 */
struct EVTVND4 {
  static constexpr uint16_t ADDR = 40119;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct END_ID
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @returns Always returns 0xFFFF.
 */
struct END_ID {
  static constexpr uint16_t ADDR = 40329; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

/**
 * @struct END_L
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @returns Always returns 0.
 */
struct END_L {
  static constexpr uint16_t ADDR = 40330; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

} // namespace I10X

#endif /* INVERTER_REGISTERS_H_ */
