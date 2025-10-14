/**
 * @file inverter_registers.h
 * @brief Defines all SunSpec inverter and Fronius-specific register namespaces.
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * inverter models and Fronius proprietary extensions. It consolidates both
 * integer (with scale factors) and floating-point maps, as well as auxiliary
 * models such as Nameplate, Extended Measurements, Immediate Controls,
 * Storage Controls, and Multiple MPPT Extension.
 *
 * The following namespaces are defined:
 * - **F** – Fronius proprietary Modbus extensions.
 * - **I10X** – SunSpec Inverter Integer + Scale Factor models (101–103).
 * - **I11X** – SunSpec Inverter Float models (111–113).
 * - **I120** – SunSpec Nameplate model.
 * - **I122** – SunSpec Extended Measurements and Status model.
 * - **I123** – SunSpec Immediate Controls model.
 * - **I124** – SunSpec Basic Storage Controls model.
 * - **I160** – SunSpec Multiple MPPT Extension model.
 *
 * Where applicable, namespaces define an `INT_OFFSET` constant used to derive
 * integer register addresses from their corresponding float register addresses
 * by adding the offset value.
 */

#ifndef INVERTER_REGISTERS_H_
#define INVERTER_REGISTERS_H_

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

/**
 * @namespace I11X
 * @brief SunSpec Inverter (Float) Model registers.
 *
 * @details
 * Provides register mappings for SunSpec inverter models 111–113, which use
 * 32-bit floating-point values for all primary quantities. These models cover
 * AC/DC electrical measurements, power and energy values, temperatures,
 * inverter state, and event flags. They represent the canonical SunSpec float
 * inverter types:
 *   - 111 – Single-phase inverter
 *   - 112 – Split-phase inverter
 *   - 113 – Three-phase inverter
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

} // namespace I11X

/**
 * @namespace I10X
 * @brief SunSpec Inverter (Integer + Scale Factor) Model registers.
 *
 * @details
 * Provides register mappings for SunSpec inverter models 101–103, which encode
 * physical quantities as integers combined with scale-factor registers. These
 * models cover AC/DC electrical data, power and energy values, temperatures,
 * inverter operating state, and event flags. They correspond to:
 *   - 101 – Single-phase inverter
 *   - 102 – Split-phase inverter
 *   - 103 – Three-phase inverter
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

} // namespace I10X

/**
 * @namespace I120
 * @brief SunSpec Nameplate Model (ID 120) register definitions.
 *
 * @details
 * Describes the static electrical capabilities of the inverter such as rated
 * power, current, voltage, and supported quadrants.
 *
 * This namespace defines an `INT_OFFSET` constant. Integer register addresses
 * are obtained by adding this offset to the corresponding float register
 * address
 * (`ADDR`). This adjustment accounts for the Fronius representation where
 * float-defined registers are stored as integers with scale factors.
 */

namespace I120 {

/** @brief Total number of registers in the nameplate model (float). */
constexpr uint16_t SIZE = 26;

/**
 * @brief Offset to convert float register addresses to integer + scale factor
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the integer
 * register address corresponding to a float register, add this offset to
 * the float register address (`ADDR`).
 */
constexpr uint16_t INT_OFFSET = 10;

/**
 * @struct ID
 * @brief Uniquely identifies this as a SunSpec Nameplate Model.
 * @return 120
 */
struct ID {
  static constexpr uint16_t ADDR = 40131;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of Nameplate Model block.
 * @return Always 26
 */
struct L {
  static constexpr uint16_t ADDR = 40132;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DERTYP
 * @brief Type of DER device.
 * Default value is 4 to indicate a PV device.
 */
struct DERTYP {
  static constexpr uint16_t ADDR = 40133;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WRTG
 * @brief Continuous power output capability of the inverter.
 * @return Power output [W]
 */
struct WRTG {
  static constexpr uint16_t ADDR = 40134;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WRTG_SF
 * @brief Continuous power output scale factor.
 */
struct WRTG_SF {
  static constexpr uint16_t ADDR = 40135;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARTG
 * @brief Continuous Volt-Ampere capability of the inverter.
 * @return Apparent power [VA]
 */
struct VARTG {
  static constexpr uint16_t ADDR = 40136;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARTG_SF
 * @brief Continuous Volt-Ampere capability scale factor.
 */
struct VARTG_SF {
  static constexpr uint16_t ADDR = 40137;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARRTGQ1
 * @brief Continuous VAR capability of the inverter in quadrant 1.
 * @return Reactive power Q1 [VAr]
 */
struct VARRTGQ1 {
  static constexpr uint16_t ADDR = 40138;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARRTGQ2
 * @brief Continuous VAR capability of the inverter in quadrant 2.
 * @note Not supported
 * @return Reactive power Q2 [VAr]
 */
struct VARRTGQ2 {
  static constexpr uint16_t ADDR = 40139;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARRTGQ3
 * @brief Continuous VAR capability of the inverter in quadrant 3.
 * @note Not supported
 * @return Reactive power Q3 [VAr]
 */
struct VARRTGQ3 {
  static constexpr uint16_t ADDR = 40140;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARRTGQ4
 * @brief Continuous VAR capability of the inverter in quadrant 4.
 * @return Reactive power Q4 [VAr]
 */
struct VARRTGQ4 {
  static constexpr uint16_t ADDR = 40141;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARRTG_SF
 * @brief Continuous VAR capability scale factor.
 */
struct VARRTG_SF {
  static constexpr uint16_t ADDR = 40142;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ARTG
 * @brief Maximum RMS AC current level capability of the inverter.
 * @return AC current [A]
 */
struct ARTG {
  static constexpr uint16_t ADDR = 40143;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ARTG_SF
 * @brief Maximum RMS AC current level scale factor.
 */
struct ARTG_SF {
  static constexpr uint16_t ADDR = 40144;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFRTGQ1
 * @brief Minimum power factor capability of the inverter in quadrant 1.
 * @return Power factor Q1 [cos(phi)]
 */
struct PFRTGQ1 {
  static constexpr uint16_t ADDR = 40145;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFRTGQ2
 * @brief Minimum power factor capability of the inverter in quadrant 2.
 * @note Not supported
 * @return Power factor Q2 [cos(phi)]
 */
struct PFRTGQ2 {
  static constexpr uint16_t ADDR = 40146;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFRTGQ3
 * @brief Minimum power factor capability of the inverter in quadrant 3.
 * @note Not supported
 * @return Power factor Q3 [cos(phi)]
 */
struct PFRTGQ3 {
  static constexpr uint16_t ADDR = 40147;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFRTGQ4
 * @brief Minimum power factor capability of the inverter in quadrant 4.
 * @return Power factor Q4 [cos(phi)]
 */
struct PFRTGQ4 {
  static constexpr uint16_t ADDR = 40148;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PFRTG_SF
 * @brief Minimum power factor capability scale factor.
 */
struct PFRTG_SF {
  static constexpr uint16_t ADDR = 40149;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WHRTG
 * @brief Nominal energy rating of storage device.
 * @return Energy rating [Wh]
 */
struct WHRTG {
  static constexpr uint16_t ADDR = 40150;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WHRTG_SF
 * @brief Nominal energy rating scale factor.
 */
struct WHRTG_SF {
  static constexpr uint16_t ADDR = 40151;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct AHRRTG
 * @brief The usable capacity of the battery.
 * Maximum charge minus minimum charge from a technology capability perspective
 * (Amp-hour capacity rating).
 * @note Not supported
 * @return Capacity [Ah]
 */
struct AHRRTG {
  static constexpr uint16_t ADDR = 40152;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct AHRRTG_SF
 * @brief Scale factor for amp-hour rating.
 * @note Not supported
 */
struct AHRRTG_SF {
  static constexpr uint16_t ADDR = 40153;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MAXCHARTE
 * @brief Maximum rate of energy transfer into the storage device.
 * @return Transfer power [W]
 */
struct MAXCHARTE {
  static constexpr uint16_t ADDR = 40154;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MAXCHARTE_SF
 * @brief Charge rate scale factor.
 */
struct MAXCHARTE_SF {
  static constexpr uint16_t ADDR = 40155;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MAXDISCHARTE
 * @brief Maximum rate of energy transfer out of the storage device.
 * @return Transfer power [W]
 */
struct MAXDISCHARTE {
  static constexpr uint16_t ADDR = 40156;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MAXDISCHARTE_SF
 * @brief Discharge rate scale factor.
 */
struct MAXDISCHARTE_SF {
  static constexpr uint16_t ADDR = 40157;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PAD
 * @brief Pad register
 */
struct PAD {
  static constexpr uint16_t ADDR = 40158;
  static constexpr uint16_t NB = 1;
};

} // namespace I120

/**
 * @namespace I122
 * @brief SunSpec Extended Measurements and Status Model (ID 122) register
 * definitions.
 *
 * @details
 * Defines extended measurement and inverter status registers beyond the basic
 * inverter models. Includes metrics such as total harmonic distortion, power
 * factor, and frequency deviation.
 *
 * An `INT_OFFSET` is defined in this namespace. Integer register addresses are
 * calculated by adding this offset to the float model addresses (`ADDR`) to
 * match the Fronius integer + scale factor representation.
 */

namespace I122 {

/** @brief Total number of registers in the extended model (float). */
constexpr uint16_t SIZE = 44;

/**
 * @brief Offset to convert float register addresses to integer + scale factor
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the integer
 * register address corresponding to a float register, add this offset to
 * the float register address (`ADDR`).
 */
constexpr uint16_t INT_OFFSET = 10;

/**
 * @struct ID
 * @brief Uniquely identifies this as a SunSpec Extended (Measurements_Status)
 * Model.
 * @return 122
 */
struct ID {
  static constexpr uint16_t ADDR = 40191;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of Extended Model block.
 * @return Always 44
 */
struct L {
  static constexpr uint16_t ADDR = 40192;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct PVCONN
 * @brief PV inverter present/available status.
 * Bit 0: Connected
 * Bit 1: Available
 * Bit 2: Operating
 * Bit 3: Test
 */
struct PVCONN {
  static constexpr uint16_t ADDR = 40193;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STORCONN
 * @brief Storage inverter present/available status.
 * Bit 0: Connected
 * Bit 1: Available
 * Bit 2: Operating
 * Bit 3: Test
 */
struct STORCONN {
  static constexpr uint16_t ADDR = 40194;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ECPCONN
 * @brief ECP connection status.
 * 0: Disconnected
 * 1: Connected
 */
struct ECPCONN {
  static constexpr uint16_t ADDR = 40195;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ACTWH
 * @brief AC lifetime active (real) energy output [Wh].
 */
struct ACTWH {
  static constexpr uint16_t ADDR = 40196;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct ACTVAH
 * @brief AC lifetime apparent energy output [VAh].
 * @note Not supported
 */
struct ACTVAH {
  static constexpr uint16_t ADDR = 40200;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct ACTVARHQ1
 * @brief AC lifetime reactive energy output in quadrant 1 [VArh].
 * @note Not supported
 */
struct ACTVARHQ1 {
  static constexpr uint16_t ADDR = 40204;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct ACTVARHQ2
 * @brief AC lifetime reactive energy output in quadrant 2 [VArh].
 * @note Not supported
 */
struct ACTVARHQ2 {
  static constexpr uint16_t ADDR = 40208;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct ACTVARHQ3
 * @brief AC lifetime reactive energy output in quadrant 3 [VArh].
 * @note Not supported
 */
struct ACTVARHQ3 {
  static constexpr uint16_t ADDR = 40212;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct ACTVARHQ4
 * @brief AC lifetime reactive energy output in quadrant 4 [VArh].
 * @note Not supported
 */
struct ACTVARHQ4 {
  static constexpr uint16_t ADDR = 40216;
  static constexpr uint16_t NB = 4;
};

/**
 * @struct VARAVAL
 * @brief Amount of VARs available without impacting output [VArh].
 * @note Not supported
 */
struct VARAVAL {
  static constexpr uint16_t ADDR = 40220;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARAVAL_SF
 * @brief Scale factor for available VARs.
 * @note Not supported
 */
struct VARAVAL_SF {
  static constexpr uint16_t ADDR = 40221;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WAVAL
 * @brief Amount of power available [W].
 * @note Not supported
 */
struct WAVAL {
  static constexpr uint16_t ADDR = 40222;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WAVAL_SF
 * @brief Scale factor for available power.
 * @note Not supported
 */
struct WAVAL_SF {
  static constexpr uint16_t ADDR = 40223;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STSETLIMMSK
 * @brief Bit Mask indicating set point limit(s) reached.
 * Bits are persistent and must be cleared by the controller.
 * @note Not supported
 */
struct STSETLIMMSK {
  static constexpr uint16_t ADDR = 40224;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct STACTCTL
 * @brief Bit Mask indicating which inverter controls are currently active.
 * Bit 0: FixedW
 * Bit 1: FixedVAR
 * Bit 2: FixedPF
 */
struct STACTCTL {
  static constexpr uint16_t ADDR = 40226;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMS
 * @brief Timestamp seconds since 01-01-2000 00:00 UTC
 */
struct TMS {
  static constexpr uint16_t ADDR = 40232;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct RTST
 * @brief Bit Mask indicating which voltage ride through modes are currently
 * active.
 * @note Not supported
 */
struct RTST {
  static constexpr uint16_t ADDR = 40234;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct RIS
 * @brief Isolation resistance Riso [Ohm].
 * @note Not supported
 */
struct RIS {
  static constexpr uint16_t ADDR = 40235;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct RIS_SF
 * @brief Scale factor for Isolation resistance.
 * @note Not supported
 */
struct RIS_SF {
  static constexpr uint16_t ADDR = 40236;
  static constexpr uint16_t NB = 1;
};

} // namespace I122

/**
 * @namespace I123
 * @brief SunSpec Immediate Controls Model (ID 123) register definitions.
 *
 * @details
 * Provides writable control points for inverter command and setpoint updates,
 * such as power-limit or reactive-power settings.
 *
 * If an `INT_OFFSET` is defined, integer register addresses correspond to the
 * float address (`ADDR`) plus this offset. This follows the Fronius convention
 * where SunSpec float models are implemented using integer registers and
 * associated scale factors.
 */

namespace I123 {

/** @brief Total number of registers in the Immediate Controls model (float). */
constexpr uint16_t SIZE = 24;

/**
 * @brief Offset to convert float register addresses to integer + scale factor
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the integer
 * register address corresponding to a float register, add this offset to
 * the float register address (`ADDR`).
 */
constexpr uint16_t INT_OFFSET = 10;

/**
 * @struct ID
 * @brief Uniquely identifies this as a SunSpec Immediate Controls model.
 * @return 123
 */
struct ID {
  static constexpr uint16_t ADDR = 40237;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of the SunSpec Immediate Controls model.
 * @return 24
 */
struct L {
  static constexpr uint16_t ADDR = 40238;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CONN_WINTMS
 * @brief Time window for connect/disconnect [s].
 * @unit s
 */
struct CONN_WINTMS {
  static constexpr uint16_t ADDR = 40239;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CONN_RVRTTMS
 * @brief Timeout period for connect/disconnect [s].
 * @unit s
 */
struct CONN_RVRTTMS {
  static constexpr uint16_t ADDR = 40240;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CONN
 * @brief Connection control.
 * 0: Disconnected
 * 1: Connected
 */
struct CONN {
  static constexpr uint16_t ADDR = 40241;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WMAXLIMPCT
 * @brief Set power output to specified level [%].
 * @unit %
 */
struct WMAXLIMPCT {
  static constexpr uint16_t ADDR = 40242;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WMAXLIMPCT_WINTMS
 * @brief Time window for power limit change.
 * @return 0-300 [s]
 * @unit s
 */
struct WMAXLIMPCT_WINTMS {
  static constexpr uint16_t ADDR = 40243;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WMAXLIMPCT_RVRTTMS
 * @brief Timeout period for power limit.
 * @return 0-28800 [s]
 * @unit s
 */
struct WMAXLIMPCT_RVRTTMS {
  static constexpr uint16_t ADDR = 40244;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WMAXLIMPCT_RMPTMS
 * @brief Ramp time for moving from current setpoint to new setpoint.
 * @return 0-65534 [s] (0xFFFF has the same effect as 0x0000)
 * @unit s
 */
struct WMAXLIMPCT_RMPTMS {
  static constexpr uint16_t ADDR = 40245;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WMAXLIM_ENA
 * @brief Throttle enable/disable control.
 * 0: Disabled
 * 1: Enabled
 */
struct WMAXLIM_ENA {
  static constexpr uint16_t ADDR = 40246;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTPFSET
 * @brief Set power factor to specific value.
 * @return 0.8 to 1.0 and -0.8 to -0.999 [cos phi]
 */
struct OUTPFSET {
  static constexpr uint16_t ADDR = 40247;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTPFSET_WINTMS
 * @brief Time window for power factor change.
 * @return 0-300 [s]
 * @unit s
 */
struct OUTPFSET_WINTMS {
  static constexpr uint16_t ADDR = 40248;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTPFSET_RVRTTMS
 * @brief Timeout period for power factor.
 * @return 0-28800 [s]
 * @unit s
 */
struct OUTPFSET_RVRTTMS {
  static constexpr uint16_t ADDR = 40249;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTPFSET_RMPTMS
 * @brief Ramp time for moving from current setpoint to new setpoint.
 * @return 0-65534 [s] (0xFFFF has the same effect as 0x0000)
 * @unit s
 */
struct OUTPFSET_RMPTMS {
  static constexpr uint16_t ADDR = 40250;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTPFSET_ENA
 * @brief Fixed power factor enable/disable control.
 * 0: Disabled
 * 1: Enabled
 */
struct OUTPFSET_ENA {
  static constexpr uint16_t ADDR = 40251;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARWMAXPCT
 * @brief Reactive power in percent of I_WMax.
 * @note Not supported
 */
struct VARWMAXPCT {
  static constexpr uint16_t ADDR = 40252;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARMAXPCT
 * @brief Reactive power in percent of I_VArMax.
 */
struct VARMAXPCT {
  static constexpr uint16_t ADDR = 40253;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARAVALPCT
 * @brief Reactive power in percent of I_VArAval.
 * @note Not supported
 */
struct VARAVALPCT {
  static constexpr uint16_t ADDR = 40254;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPCT_WINTMS
 * @brief Time window for VAR limit change.
 * @return 0-300 [s]
 * @unit s
 */
struct VARPCT_WINTMS {
  static constexpr uint16_t ADDR = 40255;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPCT_RVRTTMS
 * @brief Timeout period for VAR limit.
 * @return 0-28800 [s]
 * @unit s
 */
struct VARPCT_RVRTTMS {
  static constexpr uint16_t ADDR = 40256;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPCT_RMPTMS
 * @brief Ramp time for moving from current setpoint to new setpoint.
 * @return 0-65534 [s] (0xFFFF has the same effect as 0x0000)
 * @unit s
 */
struct VARPCT_RMPTMS {
  static constexpr uint16_t ADDR = 40257;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPCT_MOD
 * @brief VAR limit mode.
 * 2: VAR limit as a % of VArMax
 */
struct VARPCT_MOD {
  static constexpr uint16_t ADDR = 40258;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPCT_ENA
 * @brief Fixed VAR enable/disable control.
 * 0: Disabled
 * 1: Enabled
 */
struct VARPCT_ENA {
  static constexpr uint16_t ADDR = 40259;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WMAXLIMPCT_SF
 * @brief Scale factor for power output percent.
 * @return -2
 */
struct WMAXLIMPCT_SF {
  static constexpr uint16_t ADDR = 40260;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTPFSET_SF
 * @brief Scale factor for power factor.
 * @return -3
 */
struct OUTPFSET_SF {
  static constexpr uint16_t ADDR = 40261;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VARPCT_SF
 * @brief Scale factor for reactive power.
 * @return 0
 */
struct VARPCT_SF {
  static constexpr uint16_t ADDR = 40262;
  static constexpr uint16_t NB = 1;
};

} // namespace I123

/**
 * @namespace I124
 * @brief SunSpec Basic Storage Controls Model (ID 124) register definitions.
 *
 * @details
 * Defines the register map for the SunSpec Basic Storage Controls Model.
 * Although specified as a float model, Fronius devices store all values as
 * integer registers with corresponding scale factors, similar to the Multiple
 * MPPT Extension model quirk.
 *
 * The namespace defines an `INT_OFFSET`. Integer register addresses must be
 * obtained by adding this offset to the float register address (`ADDR`).
 * All registers are 16-bit and must be scaled accordingly.
 */

namespace I124 {

/**
 * @brief Total number of registers in the storage model (float)
 */
constexpr uint16_t SIZE = 26;

/**
 * @brief Offset to convert float register addresses to integer + scale factor
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the integer
 * register address corresponding to a float register, add this offset to
 * the float register address (`ADDR`).
 */
constexpr uint16_t INT_OFFSET = 10;

/**
 * @struct ID
 * @brief Model identifier
 * @note Always 124
 */
struct ID {
  static constexpr uint16_t ADDR = 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of Basic Storage Controls block
 * @unit Registers
 * @note Always 24
 */
struct L {
  static constexpr uint16_t ADDR = 2;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WCHAMAX
 * @brief Setpoint for maximum charge
 * @unit Watt [W]
 * @note Multiply by InWRte and OutWRte to define charge/discharge limits.
 */
struct WCHAMAX {
  static constexpr uint16_t ADDR = 3;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WCHAGRA
 * @brief Setpoint for maximum charging rate
 * @unit Percent of WChaMax per second [% WChaMax/s]
 */
struct WCHAGRA {
  static constexpr uint16_t ADDR = 4;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WDISCHAGRA
 * @brief Setpoint for maximum discharging rate
 * @unit Percent of WChaMax per second [% WChaMax/s]
 */
struct WDISCHAGRA {
  static constexpr uint16_t ADDR = 5;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STORCTL_MOD
 * @brief Activate hold/discharge/charge storage control mode
 * @note Bitfield: bit 0 = CHARGE, bit 1 = DISCHARGE
 */
struct STORCTL_MOD {
  static constexpr uint16_t ADDR = 6;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VACHAMAX
 * @brief Setpoint for maximum charging apparent power
 * @unit Volt-ampere [VA]
 * @note Not supported on Fronius devices.
 */
struct VACHAMAX {
  static constexpr uint16_t ADDR = 7;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MINRSVPCT
 * @brief Setpoint for minimum storage reserve
 * @unit Percent of nominal maximum storage [%]
 */
struct MINRSVPCT {
  static constexpr uint16_t ADDR = 8;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CHASTATE
 * @brief Available energy as a percent of capacity rating
 * @unit Percent [%]
 */
struct CHASTATE {
  static constexpr uint16_t ADDR = 9;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STORAVAL
 * @brief State of charge minus storage reserve times capacity rating
 * @unit Ampere-hour [Ah]
 * @note Not supported on Fronius devices.
 */
struct STORAVAL {
  static constexpr uint16_t ADDR = 10;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INBATV
 * @brief Internal battery voltage
 * @unit Volt [V]
 * @note Not supported on Fronius devices.
 */
struct INBATV {
  static constexpr uint16_t ADDR = 11;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CHAST
 * @brief Charge status of the storage device
 * @note Enumerated values: 1=OFF, 2=EMPTY, 3=DISCHARGING, 4=CHARGING, 5=FULL,
 * 6=HOLDING, 7=TESTING
 */
struct CHAST {
  static constexpr uint16_t ADDR = 12;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct OUTWRTE
 * @brief Percent of maximum discharge rate
 * @unit Percent of WChaMax [%]
 * @note Valid range -100.00% to +100.00%; uses scale factor InOutWRte_SF.
 */
struct OUTWRTE {
  static constexpr uint16_t ADDR = 13;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INWRTE
 * @brief Percent of maximum charge rate
 * @unit Percent of WChaMax [%]
 * @note Valid range -100.00% to +100.00%; uses scale factor InOutWRte_SF.
 */
struct INWRTE {
  static constexpr uint16_t ADDR = 14;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INOUTWRTE_WINTMS
 * @brief Time window for charge/discharge rate change
 * @unit Seconds [s]
 * @note Not supported on Fronius devices.
 */
struct INOUTWRTE_WINTMS {
  static constexpr uint16_t ADDR = 15;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INOUTWRTE_RVRTTMS
 * @brief Timeout period for charge/discharge rate
 * @unit Seconds [s]
 * @note Not supported on Fronius devices.
 */
struct INOUTWRTE_RVRTTMS {
  static constexpr uint16_t ADDR = 16;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INOUTWRTE_RMPTMS
 * @brief Ramp time for moving from current setpoint to new setpoint
 * @unit Seconds [s]
 * @note Not supported on Fronius devices.
 */
struct INOUTWRTE_RMPTMS {
  static constexpr uint16_t ADDR = 17;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CHAGRISET
 * @brief Setpoint to enable or disable charging from grid
 * @note Enumerated values: 0=PV (grid charging disabled), 1=GRID (grid charging
 * enabled)
 */
struct CHAGRISET {
  static constexpr uint16_t ADDR = 18;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WCHAMAX_SF
 * @brief Scale factor for maximum charge
 */
struct WCHAMAX_SF {
  static constexpr uint16_t ADDR = 19;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct WCHADISCHAGRA_SF
 * @brief Scale factor for maximum charge and discharge rate
 */
struct WCHADISCHAGRA_SF {
  static constexpr uint16_t ADDR = 20;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct VACHAMAX_SF
 * @brief Scale factor for maximum charging VA
 * @note Not supported on Fronius devices.
 */
struct VACHAMAX_SF {
  static constexpr uint16_t ADDR = 21;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct MINRSVPCT_SF
 * @brief Scale factor for minimum reserve percentage
 */
struct MINRSVPCT_SF {
  static constexpr uint16_t ADDR = 22;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct CHASTATE_SF
 * @brief Scale factor for available energy percent
 */
struct CHASTATE_SF {
  static constexpr uint16_t ADDR = 23;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct STORAVAL_SF
 * @brief Scale factor for state of charge
 * @note Not supported on Fronius devices.
 */
struct STORAVAL_SF {
  static constexpr uint16_t ADDR = 24;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INBATV_SF
 * @brief Scale factor for battery voltage
 * @note Not supported on Fronius devices.
 */
struct INBATV_SF {
  static constexpr uint16_t ADDR = 25;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INOUTWRTE_SF
 * @brief Scale factor for percent charge/discharge rate
 */
struct INOUTWRTE_SF {
  static constexpr uint16_t ADDR = 26;
  static constexpr uint16_t NB = 1;
};
} // namespace I124

/**
 * @namespace I160
 * @brief SunSpec Multiple MPPT Extension Model (ID 160) register definitions.
 *
 * @details
 * Groups all registers of the SunSpec Multiple MPPT Extension Model, which
 * provides per-input DC data for inverters with multiple MPP trackers.
 *
 * An `INT_OFFSET` constant is defined in this namespace. Integer register
 * addresses must be derived by adding this offset to the float address
 * (`ADDR`), reflecting the Fronius implementation where float-mapped registers
 * are stored as integer registers with scale factors.
 */

namespace I160 {

/** @brief Total number of registers in the multiple MPPT model (float). */
constexpr uint16_t SIZE = 48;

/**
 * @brief Offset to convert float register addresses to integer + scale factor
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the integer
 * register address corresponding to a float register, add this offset to
 * the float register address (`ADDR`).
 */
constexpr uint16_t INT_OFFSET = 10;

/**
 * @struct ID
 * @brief Uniquely identifies this as a SunSpec Multiple MPPT Inverter Extension
 * Model.
 * @returns 160
 */
struct ID {
  static constexpr uint16_t ADDR = 40263;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of Multiple MPPT Inverter Extension Model.
 * @returns 48
 */
struct L {
  static constexpr uint16_t ADDR = 40264;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCA_SF
 * @brief DC current scale factor
 */
struct DCA_SF {
  static constexpr uint16_t ADDR = 40265;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCV_SF
 * @brief DC voltage scale factor
 */
struct DCV_SF {
  static constexpr uint16_t ADDR = 40266;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCW_SF
 * @brief DC power scale factor
 */
struct DCW_SF {
  static constexpr uint16_t ADDR = 40267;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCWH_SF
 * @brief DC energy scale factor
 * @note Not supported for Fronius Hybrid inverters
 */
struct DCWH_SF {
  static constexpr uint16_t ADDR = 40268;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct EVT
 * @brief Global events register.
 */
struct EVT {
  static constexpr uint16_t ADDR = 40269;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct N
 * @brief Number of DC modules.
 * @returns 2
 */
struct N {
  static constexpr uint16_t ADDR = 40271;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct ID_1
 * @brief Input ID of first DC input.
 * @returns 1
 */
struct ID_1 {
  static constexpr uint16_t ADDR = 40273;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct IDSTR
 * @brief Input ID string of first DC input.
 * @returns "String 1"
 */
struct IDSTR_1 {
  static constexpr uint16_t ADDR = 40274;
  static constexpr uint16_t NB = 8;
};

/**
 * @struct DCA_1
 * @brief DC current of first input.
 * @unit Ampere [A]
 */
struct DCA_1 {
  static constexpr uint16_t ADDR = 40282;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCV_1
 * @brief DC voltage of first input.
 * @unit Volt [V]
 */
struct DCV_1 {
  static constexpr uint16_t ADDR = 40283;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCW_1
 * @brief DC power of first input.
 * @unit Watt [W]
 */
struct DCW_1 {
  static constexpr uint16_t ADDR = 40284;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCWH_1
 * @brief DC lifetime energy of first input.
 * @note Not supported for Fronius Hybrid inverters
 * @unit Watt-hour [Wh]
 */
struct DCWH_1 {
  static constexpr uint16_t ADDR = 40285;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMS
 * @brief Timestamp of first DC input since 01-Jan-2000 00:00 UTC.
 */
struct TMS_1 {
  static constexpr uint16_t ADDR = 40287;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMP_1
 * @brief Temperature of first DC input.
 * @unit Celsius [°C]
 */
struct TMP_1 {
  static constexpr uint16_t ADDR = 40289;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCST_1
 * @brief Operating state of first DC input.
 */
struct DCST_1 {
  static constexpr uint16_t ADDR = 40290;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCEVT_1
 * @brief Module events of first DC input.
 */
struct DCEVT_1 {
  static constexpr uint16_t ADDR = 40291;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct ID_2
 * @brief Input ID of second DC input.
 * @returns 2
 */
struct ID_2 {
  static constexpr uint16_t ADDR = 40293;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct IDSTR_2
 * @brief Input ID string of second DC input.
 * @returns "String 2" or "not supported"
 */
struct IDSTR_2 {
  static constexpr uint16_t ADDR = 40294;
  static constexpr uint16_t NB = 8;
};

/**
 * @struct DCA_2
 * @brief DC current of second input.
 * @unit Ampere [A]
 * @note Not supported if only one DC input
 */
struct DCA_2 {
  static constexpr uint16_t ADDR = 40302;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCV_2
 * @brief DC voltage of second input.
 * @unit Volt [V]
 * @note Not supported if only one DC input
 */
struct DCV_2 {
  static constexpr uint16_t ADDR = 40303;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCW_2
 * @brief DC power of second input.
 * @unit Watt [W]
 * @note Not supported if only one DC input
 */
struct DCW_2 {
  static constexpr uint16_t ADDR = 40304;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCWH_2
 * @brief DC lifetime energy of second input.
 * @note Not supported for Fronius Hybrid inverters
 * @unit Watt-hour [Wh]
 */
struct DCWH_2 {
  static constexpr uint16_t ADDR = 40305;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMS_2
 * @brief Timestamp of second DC input since 01-Jan-2000 00:00 UTC.
 * @note Not supported if only one DC input
 */
struct TMS_2 {
  static constexpr uint16_t ADDR = 40307;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct TMP_2
 * @brief Temperature of second DC input.
 * @unit Celsius [°C]
 * @note Not supported if only one DC input
 */
struct TMP_2 {
  static constexpr uint16_t ADDR = 40309;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCST_2
 * @brief Operating state of second DC input.
 * @note Not supported if only one DC input
 */
struct DCST_2 {
  static constexpr uint16_t ADDR = 40310;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCEVT_2
 * @brief Module events of second DC input.
 * @note Not supported if only one DC input
 */
struct DCEVT_2 {
  static constexpr uint16_t ADDR = 40311;
  static constexpr uint16_t NB = 2;
};

} // namespace I160

/**
 * @namespace END
 * @brief SunSpec end-of-block registers.
 * @details
 * This namespace defines the SunSpec "end model" block that marks the end of a
 * SunSpec register map within Fronius devices. It is typically composed of two
 * registers — an identifier (`ID`) with a constant value of 0xFFFF and a length
 * field (`L`) that is always 0.
 *
 * Fronius inverters, including hybrid models, follow this pattern at the end of
 * each SunSpec model block. However, when a Basic Storage Control (BSC) model
 * is present, the end-of-block position is shifted by 26 registers to account
 * for the storage model length. When parsing or iterating over SunSpec model
 * blocks, this offset must be considered to correctly locate the next model.
 */
namespace I_END {

/**
 * @brief Offset to convert float register addresses to integer + scale factor
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the integer
 * register address corresponding to a float register, add this offset to
 * the float register address (`ADDR`).
 */
constexpr uint16_t INT_OFFSET = 10;

/**
 * @struct ID
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @returns Always returns 0xFFFF.
 * @note For hybrid inverters that include a Basic Storage Control (BSC)
 * register map, the end register address is offset by 26 registers. Take this
 * into account when iterating over SunSpec blocks or calculating the next
 * model's start address
 */
struct ID {
  static constexpr uint16_t ADDR = 40303; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

/**
 * @struct L
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @returns Always returns 0.
 */
struct L {
  static constexpr uint16_t ADDR = 40304; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

} // namespace I_END

#endif /* INVERTER_REGISTERS_H_ */
