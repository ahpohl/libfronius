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
 * Where applicable, namespaces define an `FLOAT_OFFSET` constant used to derive
 * float register addresses from their corresponding integer register addresses
 * by adding the offset value.
 */

#ifndef INVERTER_REGISTERS_H_
#define INVERTER_REGISTERS_H_

#include "register_base.h"
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
 * @brief Delete stored rating data of the current inverter.
 *
 * @details
 * Writing the value `0xFFFF` deletes stored rating data for the
 * currently addressed inverter.
 */
constexpr Register DELETE_DATA(211, 1, RegType::UINT16);

/**
 * @brief Store rating data persistently.
 *
 * @details
 * Writing `0xFFFF` stores the rating data of all inverters connected to the
 * Fronius Datamanager persistently.
 */
constexpr Register STORE_DATA(212, 1, RegType::UINT16);

/**
 * @brief Current active inverter state code.
 *
 * @details
 * Reports the current operational state of the inverter.
 * Refer to the inverter manual for detailed state descriptions.
 *
 * @note Not supported for Fronius Hybrid inverters. Status may differ during
 * night-time operation compared to other inverter models.
 */
constexpr Register ACTIVE_STATE_CODE(213, 1, RegType::UINT16);

/**
 * @brief Reset all event flags and active state code.
 *
 * @details
 * Writing the value `0xFFFF` resets all event flags and clears
 * the active state code register.
 */
constexpr Register RESET_ALL_EVENT_FLAGS(214, 1, RegType::UINT16);

/**
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
constexpr Register MODEL_TYPE(215, 1, RegType::UINT16);

/**
 * @brief Defines scope of storage restriction reporting.
 *
 * @details
 * Selects which type of restriction data is reported by the
 * BasicStorageControl model (IC124).
 *
 * - `0`: Local restrictions (default, set by Modbus interface)
 * - `1`: Global restrictions (system-wide)
 */
constexpr Register STORAGE_RESTRICTIONS_VIEW_MODE(216, 1, RegType::UINT16);

/**
 * @brief Total site power of all connected inverters.
 * @unit Watt [W]
 *
 * @details
 * Reports the instantaneous total active power output across all
 * connected inverters in the system.
 */
constexpr Register SITE_POWER(499, 2, RegType::UINT32);

/**
 * @brief Total energy produced today by all connected inverters.
 * @unit Watt-hour [Wh]
 *
 * @details
 * Accumulates the total daily energy production across all connected
 * inverters since midnight.
 */
constexpr Register SITE_ENERGY_DAY(501, 4, RegType::UINT64);

/**
 * @brief Total energy produced during the current year.
 * @unit Watt-hour [Wh]
 *
 * @details
 * Represents the cumulative energy produced by all connected inverters
 * since January 1st of the current year.
 */
constexpr Register SITE_ENERGY_YEAR(505, 4, RegType::UINT64);

/**
 * @brief Lifetime total energy produced by all connected inverters.
 * @unit Watt-hour [Wh]
 *
 * @details
 * Indicates the total lifetime energy yield of all connected
 * inverters in the system.
 */
constexpr Register SITE_ENERGY_TOTAL(509, 4, RegType::UINT64);

} // namespace F

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
 * @brief SunSpec inverter model identifier (integer + scale factor).
 * @returns
 *  - 101: Single-phase
 *  - 102: Split-phase
 *  - 103: Three-phase
 */
constexpr Register ID(40069, 1, RegType::UINT16);

/**
 * @brief Length of inverter model block.
 * @returns Always returns 50.
 */
constexpr Register L(40070, 1, RegType::UINT16);

/**
 * @brief AC total current.
 * @unit Amperes [A]
 */
constexpr Register A(40071, 1, RegType::UINT16);

/**
 * @brief AC current in phase A.
 * @unit Amperes [A]
 */
constexpr Register APHA(40072, 1, RegType::UINT16);

/**
 * @brief AC current in phase B.
 * @unit Amperes [A]
 */
constexpr Register APHB(40073, 1, RegType::UINT16);

/**
 * @brief AC current in phase C.
 * @unit Amperes [A]
 */
constexpr Register APHC(40074, 1, RegType::UINT16);

/** @brief AC current scale factor. */
constexpr Register A_SF(40075, 1, RegType::INT16);

/**
 * @brief AC voltage phase-to-phase AB.
 * @unit Volts [V]
 */
constexpr Register PPVPHAB(40076, 1, RegType::UINT16);

/**
 * @brief AC voltage phase-to-phase BC.
 * @unit Volts [V]
 */
constexpr Register PPVPHBC(40077, 1, RegType::UINT16);

/**
 * @brief AC voltage phase-to-phase CA.
 * @unit Volts [V]
 */
constexpr Register PPVPHCA(40078, 1, RegType::UINT16);

/**
 * @brief AC voltage phase A to neutral.
 * @unit Volts [V]
 */
constexpr Register PHVPHA(40079, 1, RegType::UINT16);

/**
 * @brief AC voltage phase B to neutral.
 * @unit Volts [V]
 */
constexpr Register PHVPHB(40080, 1, RegType::UINT16);

/**
 * @brief AC voltage phase C to neutral.
 * @unit Volts [V]
 */
constexpr Register PHVPHC(40081, 1, RegType::UINT16);

/** @brief AC voltage scale factor. */
constexpr Register V_SF(40082, 1, RegType::INT16);

/**
 * @brief AC active power.
 * @unit Watts [W]
 */
constexpr Register W(40083, 1, RegType::INT16);

/** @brief AC active power scale factor. */
constexpr Register W_SF(40084, 1, RegType::INT16);

/**
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(40085, 1, RegType::UINT16);

/** @brief AC frequency scale factor. */
constexpr Register FREQ_SF(40086, 1, RegType::INT16);

/**
 * @brief Apparent power.
 * @unit Volt-amperes [VA]
 */
constexpr Register VA(40087, 1, RegType::INT16);

/** @brief Apparent power scale factor. */
constexpr Register VA_SF(40088, 1, RegType::INT16);

/**
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(40089, 1, RegType::INT16);

/** @brief Reactive power scale factor. */
constexpr Register VAR_SF(40090, 1, RegType::INT16);

/**
 * @brief Power factor.
 * @unit Percent [%]
 */
constexpr Register PF(40091, 1, RegType::INT16);

/** @brief Power factor scale factor. */
constexpr Register PF_SF(40092, 1, RegType::INT16);

/**
 * @brief AC lifetime energy production.
 * @unit Watt-hours [Wh]
 */
constexpr Register WH(40093, 2, RegType::UINT32);

/** @brief AC lifetime energy scale factor. */
constexpr Register WH_SF(40095, 1, RegType::INT16);

/**
 * @brief DC current.
 * @unit Amperes [A]
 */
constexpr Register DCA(40096, 1, RegType::UINT16);

/** @brief DC current scale factor. */
constexpr Register DCA_SF(40097, 1, RegType::INT16);

/**
 * @brief DC voltage.
 * @unit Volts [V]
 */
constexpr Register DCV(40098, 1, RegType::UINT16);

/** @brief DC voltage scale factor. */
constexpr Register DCV_SF(40099, 1, RegType::INT16);

/**
 * @brief DC power.
 * @unit Watts [W]
 */
constexpr Register DCW(40100, 1, RegType::INT16);

/** @brief DC power scale factor. */
constexpr Register DCW_SF(40101, 1, RegType::INT16);

/** @brief Inverter operating state. */
constexpr Register ST(40107, 1, RegType::UINT16);

/** @brief Vendor-defined operating state. */
constexpr Register STVND(40108, 1, RegType::UINT16);

/** @brief Event flags bits 0–31. */
constexpr Register EVT1(40109, 2, RegType::UINT32);

/** @brief Event flags bits 32–63. */
constexpr Register EVT2(40111, 2, RegType::UINT32);

/** @brief Vendor-defined event flags bits 0–31. */
constexpr Register EVTVND1(40113, 2, RegType::UINT32);

/** @brief Vendor-defined event flags bits 32–63. */
constexpr Register EVTVND2(40115, 2, RegType::UINT32);

/** @brief Vendor-defined event flags bits 64–95. */
constexpr Register EVTVND3(40117, 2, RegType::UINT32);

/** @brief Vendor-defined event flags bits 96–127. */
constexpr Register EVTVND4(40119, 2, RegType::UINT32);

} // namespace I10X

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
 * @brief SunSpec inverter model identifier (float).
 * @details Identifies the type of inverter register map. The value indicates
 * whether the device is single-, split-, or three-phase.
 * @returns
 *  - 111: Single-phase
 *  - 112: Split-phase
 *  - 113: Three-phase
 */
constexpr Register ID(40069, 1, RegType::UINT16);

/**
 * @brief Length of inverter model block.
 * @returns Always returns 60.
 */
constexpr Register L(40070, 1, RegType::UINT16);

/**
 * @brief AC total current.
 * @unit Ampere [A]
 */
constexpr Register A(40071, 2, RegType::FLOAT);

/**
 * @brief AC current in phase A.
 * @unit Ampere [A]
 */
constexpr Register APHA(40073, 2, RegType::FLOAT);

/**
 * @brief AC current in phase B.
 * @unit Ampere [A]
 */
constexpr Register APHB(40075, 2, RegType::FLOAT);

/**
 * @brief AC current in phase C.
 * @unit Ampere [A]
 */
constexpr Register APHC(40077, 2, RegType::FLOAT);

/**
 * @brief AC voltage phase-to-phase AB.
 * @unit Volt [V]
 */
constexpr Register PPVPHAB(40079, 2, RegType::FLOAT);

/**
 * @brief AC voltage phase-to-phase BC.
 * @unit Volt [V]
 */
constexpr Register PPVPHBC(40081, 2, RegType::FLOAT);

/**
 * @brief AC voltage phase-to-phase CA.
 * @unit Volt [V]
 */
constexpr Register PPVPHCA(40083, 2, RegType::FLOAT);

/**
 * @brief AC voltage phase-A to neutral.
 * @unit Volt [V]
 */
constexpr Register PHVPHA(40085, 2, RegType::FLOAT);

/**
 * @brief AC voltage phase-B to neutral.
 * @unit Volt [V]
 */
constexpr Register PHVPHB(40087, 2, RegType::FLOAT);

/**
 * @brief AC voltage phase-C to neutral.
 * @unit Volt [V]
 */
constexpr Register PHVPHC(40089, 2, RegType::FLOAT);

/**
 * @brief AC total power.
 * @unit Watt [W]
 */
constexpr Register W(40091, 2, RegType::FLOAT);

/**
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(40093, 2, RegType::FLOAT);

/**
 * @brief Apparent power.
 * @unit Volt-ampere [VA]
 */
constexpr Register VA(40095, 2, RegType::FLOAT);

/**
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(40097, 2, RegType::FLOAT);

/**
 * @brief Power factor.
 * @unit Percent [%]
 */
constexpr Register PF(40099, 2, RegType::FLOAT);

/**
 * @brief Lifetime energy production.
 * @unit Watt-hour [Wh]
 */
constexpr Register WH(40101, 2, RegType::FLOAT);

/**
 * @brief DC current.
 * @unit Ampere [A]
 */
constexpr Register DCA(40103, 2, RegType::FLOAT);

/**
 * @brief DC voltage.
 * @unit Volt [V]
 */
constexpr Register DCV(40105, 2, RegType::FLOAT);

/**
 * @brief DC power.
 * @unit Watt [W]
 */
constexpr Register DCW(40107, 2, RegType::FLOAT);

/** @brief Inverter operating state. */
constexpr Register ST(40117, 1, RegType::UINT16);

/** @brief Vendor-defined operating state. */
constexpr Register STVND(40118, 1, RegType::UINT16);

/** @brief Event flags (bits 0–31). */
constexpr Register EVT1(40119, 2, RegType::UINT32);

/** @brief Event flags (bits 32–63). */
constexpr Register EVT2(40121, 2, RegType::UINT32);

/** @brief Vendor-defined event flags (bits 0–31). */
constexpr Register EVTVND1(40123, 2, RegType::UINT32);

/** @brief Vendor-defined event flags (bits 32–63). */
constexpr Register EVTVND2(40125, 2, RegType::UINT32);

/** @brief Vendor-defined event flags (bits 64–95). */
constexpr Register EVTVND3(40127, 2, RegType::UINT32);

/** @brief Vendor-defined event flags (bits 96–127). */
constexpr Register EVTVND4(40129, 2, RegType::UINT32);

} // namespace I11X

/**
 * @namespace I120
 * @brief SunSpec Nameplate Model (ID 120) register definitions.
 *
 * @details
 * Describes the static electrical capabilities of the inverter such as rated
 * power, current, voltage, and supported quadrants.
 *
 * This namespace defines a `FLOAT_OFFSET` constant. Float register addresses
 * are obtained by adding this offset to the corresponding integer register
 * address (`ADDR`). This adjustment accounts for the Fronius representation
 * where float-defined registers are stored as integers with scale factors.
 */
namespace I120 {

/** @brief Total number of registers in the nameplate model. */
constexpr uint16_t SIZE = 26;

/**
 * @brief Offset to convert integer+scale factor register addresses to float
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the float
 * register address corresponding to an integer+scale register, add this
 * offset to the integer+scale register address (`ADDR`).
 */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Uniquely identifies this as a SunSpec Nameplate Model.
 * @return 120
 */
constexpr Register ID(40121, 1, RegType::UINT16);

/**
 * @brief Length of Nameplate Model block.
 * @return Always 26
 */
constexpr Register L(40122, 1, RegType::UINT16);

/** @brief Type of DER device. Default value is 4 to indicate a PV device. */
constexpr Register DERTYP(40123, 1, RegType::UINT16);

/**
 * @brief Continuous power output capability of the inverter.
 * @return Power output [W]
 */
constexpr Register WRTG(40124, 1, RegType::UINT16);

/** @brief Continuous power output scale factor. */
constexpr Register WRTG_SF(40125, 1, RegType::INT16);

/**
 * @brief Continuous Volt-Ampere capability of the inverter.
 * @return Apparent power [VA]
 */
constexpr Register VARTG(40126, 1, RegType::UINT16);

/** @brief Continuous Volt-Ampere capability scale factor. */
constexpr Register VARTG_SF(40127, 1, RegType::INT16);

/**
 * @brief Continuous VAR capability of the inverter in quadrant 1.
 * @return Reactive power Q1 [VAr]
 */
constexpr Register VARRTGQ1(40128, 1, RegType::INT16);

/**
 * @brief Continuous VAR capability of the inverter in quadrant 4.
 * @return Reactive power Q4 [VAr]
 */
constexpr Register VARRTGQ4(40131, 1, RegType::INT16);

/** @brief Continuous VAR capability scale factor. */
constexpr Register VARRTG_SF(40132, 1, RegType::INT16);

/**
 * @brief Maximum RMS AC current level capability of the inverter.
 * @return AC current [A]
 */
constexpr Register ARTG(40133, 1, RegType::UINT16);

/** @brief Maximum RMS AC current level scale factor. */
constexpr Register ARTG_SF(40134, 1, RegType::INT16);

/**
 * @brief Minimum power factor capability of the inverter in quadrant 1.
 * @return Power factor Q1 [cos(phi)]
 */
constexpr Register PFRTGQ1(40135, 1, RegType::INT16);

/**
 * @brief Minimum power factor capability of the inverter in quadrant 4.
 * @return Power factor Q4 [cos(phi)]
 */
constexpr Register PFRTGQ4(40138, 1, RegType::INT16);

/** @brief Minimum power factor capability scale factor. */
constexpr Register PFRTG_SF(40139, 1, RegType::INT16);

/**
 * @brief Nominal energy rating of storage device.
 * @return Energy rating [Wh]
 */
constexpr Register WHRTG(40140, 1, RegType::UINT16);

/** @brief Nominal energy rating scale factor. */
constexpr Register WHRTG_SF(40141, 1, RegType::INT16);

/**
 * @brief Maximum rate of energy transfer into the storage device.
 * @return Transfer power [W]
 */
constexpr Register MAXCHARTE(40144, 1, RegType::UINT16);

/** @brief Charge rate scale factor. */
constexpr Register MAXCHARTE_SF(40145, 1, RegType::INT16);

/**
 * @brief Maximum rate of energy transfer out of the storage device.
 * @return Transfer power [W]
 */
constexpr Register MAXDISCHARTE(40146, 1, RegType::UINT16);

/** @brief Discharge rate scale factor. */
constexpr Register MAXDISCHARTE_SF(40147, 1, RegType::INT16);

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
 * A `FlOAT_OFFSET` is defined in this namespace. Float register addresses are
 * calculated by adding this offset to the integer model addresses (`ADDR`) to
 * match the Fronius integer + scale factor representation.
 */
namespace I122 {

/** @brief Total number of registers in the extended model. */
constexpr uint16_t SIZE = 44;

/**
 * @brief Offset to convert integer+scale factor register addresses to float
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the float
 * register address corresponding to an integer+scale register, add this
 * offset to the integer+scale register address (`ADDR`).
 */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief
 * Uniquely identifies this as a SunSpec Extended (Measurements_Status) Model.
 * @return 122
 */
constexpr Register ID(40181, 1, RegType::UINT16);

/**
 * @brief
 * Length of Extended Model block.
 * @return Always 44
 */
constexpr Register L(40182, 1, RegType::UINT16);

/**
 * @brief
 * PV inverter present/available status.
 * Bit 0: Connected
 * Bit 1: Available
 * Bit 2: Operating
 * Bit 3: Test
 */
constexpr Register PVCONN(40183, 1, RegType::UINT16);

/**
 * @brief
 * Storage inverter present/available status.
 * Bit 0: Connected
 * Bit 1: Available
 * Bit 2: Operating
 * Bit 3: Test
 */
constexpr Register STORCONN(40184, 1, RegType::UINT16);

/**
 * @brief
 * ECP connection status.
 * 0: Disconnected
 * 1: Connected
 */
constexpr Register ECPCONN(40185, 1, RegType::UINT16);

/** @brief AC lifetime active (real) energy output [Wh]. */
constexpr Register ACTWH(40186, 4, RegType::UINT64);

/**
 * @brief
 * Bit Mask indicating which inverter controls are currently active.
 * Bit 0: FixedW
 * Bit 1: FixedVAR
 * Bit 2: FixedPF
 */
constexpr Register STACTCTL(40216, 2, RegType::UINT32);

/**
 * @brief
 * Source of time synchronization
 * @return RTC
 */
constexpr Register TMSSRC(40218, 4, RegType::STRING);

/** @brief Timestamp seconds since 01-01-2000 00:00 UTC */
constexpr Register TMS(40222, 2, RegType::UINT32);

} // namespace I122

/**
 * @namespace I123
 * @brief SunSpec Immediate Controls Model (ID 123) register definitions.
 *
 * @details
 * This namespace provides definitions for all registers of the SunSpec
 * Immediate Controls model, including writable control points for inverter
 * commands and setpoint updates, such as power-limit or reactive-power
 * settings.
 */
namespace I123 {

/** @brief Total number of registers in the Immediate Controls model. */
constexpr uint16_t SIZE = 24;

/**
 * @brief Offset to convert integer+scale factor register addresses to float
 * addresses.
 *
 * @details
 * In Fronius SunSpec devices, some float registers are implemented using
 * integer registers with associated scale factors. To obtain the corresponding
 * float register address from an integer+scale factor register address
 * (`ADDR`), add this offset.
 */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief
 * Uniquely identifies this as a SunSpec Immediate Controls model.
 * @return 123
 */
constexpr Register ID(40227, 1, RegType::UINT16);

/**
 * @brief
 * Length of the SunSpec Immediate Controls model.
 * @return 24
 */
constexpr Register L(40228, 1, RegType::UINT16);

/**
 * @brief
 * Time window for connect/disconnect.
 * @unit s
 */
constexpr Register CONN_WINTMS(40229, 1, RegType::UINT16);

/**
 * @brief
 * Timeout period for connect/disconnect.
 * @unit s
 */
constexpr Register CONN_RVRTTMS(40230, 1, RegType::UINT16);

/**
 * @brief
 * Connection control.
 * 0: Disconnected
 * 1: Connected
 */
constexpr Register CONN(40231, 1, RegType::UINT16);

/**
 * @brief
 * Set power output to specified level.
 * @unit % WMax
 */
constexpr Register WMAXLIMPCT(40232, 1, RegType::UINT16);

/**
 * @brief
 * Time window for power limit change.
 * @return 0-300
 * @unit seconds [s]
 */
constexpr Register WMAXLIMPCT_WINTMS(40233, 1, RegType::UINT16);

/**
 * @brief
 * Timeout period for power limit.
 * @return 0-28800
 * @unit seconds [s]
 */
constexpr Register WMAXLIMPCT_RVRTTMS(40234, 1, RegType::UINT16);

/**
 * @brief
 * Ramp time for moving from current setpoint to new setpoint.
 * @return 0-65534 (0xFFFF has the same effect as 0x0000)
 * @unit seconds [s]
 */
constexpr Register WMAXLIMPCT_RMPTMS(40235, 1, RegType::UINT16);

/**
 * @brief
 * Throttle enable/disable control.
 * 0: Disabled
 * 1: Enabled
 */
constexpr Register WMAXLIM_ENA(40236, 1, RegType::UINT16);

/**
 * @brief
 * Set power factor to specific value.
 * @return 0.8 to 1.0 and -0.8 to -0.999 [cos phi]
 */
constexpr Register OUTPFSET(40237, 1, RegType::INT16);

/**
 * @brief
 * Time window for power factor change.
 * @return 0-300
 * @unit seconds [s]
 */
constexpr Register OUTPFSET_WINTMS(40238, 1, RegType::UINT16);

/**
 * @brief
 * Timeout period for power factor.
 * @return 0-28800
 * @unit seconds [s]
 */
constexpr Register OUTPFSET_RVRTTMS(40239, 1, RegType::UINT16);

/**
 * @brief
 * Ramp time for moving from current setpoint to new setpoint.
 * @return 0-65534 (0xFFFF has the same effect as 0x0000)
 * @unit seconds [s]
 */
constexpr Register OUTPFSET_RMPTMS(40240, 1, RegType::UINT16);

/**
 * @brief
 * Fixed power factor enable/disable control.
 * 0: Disabled
 * 1: Enabled
 */
constexpr Register OUTPFSET_ENA(40241, 1, RegType::UINT16);

/**
 * @brief
 * Reactive power in percent of I_VArMax.
 */
constexpr Register VARMAXPCT(40243, 1, RegType::INT16);

/**
 * @brief
 * Time window for VAR limit change.
 * @return 0-300
 * @unit seconds [s]
 */
constexpr Register VARPCT_WINTMS(40245, 1, RegType::UINT16);

/**
 * @brief
 * Timeout period for VAR limit.
 * @return 0-28800
 * @unit seconds [s]
 */
constexpr Register VARPCT_RVRTTMS(40246, 1, RegType::UINT16);

/**
 * @brief
 * Ramp time for moving from current setpoint to new setpoint.
 * @return 0-65534 (0xFFFF has the same effect as 0x0000)
 * @unit seconds [s]
 */
constexpr Register VARPCT_RMPTMS(40247, 1, RegType::UINT16);

/**
 * @brief
 * VAR limit mode.
 * 2: VAR limit as a % of VArMax
 */
constexpr Register VARPCT_MOD(40248, 1, RegType::UINT16);

/**
 * @brief
 * Fixed VAR enable/disable control.
 * 0: Disabled
 * 1: Enabled
 */
constexpr Register VARPCT_ENA(40249, 1, RegType::UINT16);

/** @brief Scale factor for power output percent. */
constexpr Register WMAXLIMPCT_SF(40250, 1, RegType::INT16);

/** @brief Scale factor for power factor. */
constexpr Register OUTPFSET_SF(40251, 1, RegType::INT16);

/** @brief Scale factor for reactive power. */
constexpr Register VARPCT_SF(40252, 1, RegType::INT16);

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

/** @brief Total number of registers in the storage model (float) */
constexpr uint16_t SIZE = 26;

/**
 * @brief Offset to convert integer+scale factor register addresses to float
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the float
 * register address corresponding to an integer+scale register, add this
 * offset to the integer+scale register address (`ADDR`).
 */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief
 * Model identifier
 * @return 124
 */
constexpr Register ID(40303, 1, RegType::UINT16);

/**
 * @brief
 * Length of Basic Storage Controls block
 * @return 24
 */
constexpr Register L(40304, 1, RegType::UINT16);

/**
 * @brief
 * Setpoint for maximum charge
 * @unit Watt [W]
 * @note Multiply by InWRte and OutWRte to define charge/discharge limits.
 */
constexpr Register WCHAMAX(40305, 1, RegType::UINT16);

/**
 * @brief
 * Setpoint for maximum charging rate
 * @unit Percent of WChaMax per second [% WChaMax/s]
 */
constexpr Register WCHAGRA(40306, 1, RegType::UINT16);

/**
 * @brief
 * Setpoint for maximum discharging rate
 * @unit Percent of WChaMax per second [% WChaMax/s]
 */
constexpr Register WDISCHAGRA(40307, 1, RegType::UINT16);

/**
 * @brief
 * Activate hold/discharge/charge storage control mode
 * @note Bitfield: bit 0 = CHARGE, bit 1 = DISCHARGE
 */
constexpr Register STORCTL_MOD(40308, 1, RegType::UINT16);

/**
 * @brief
 * Setpoint for minimum storage reserve
 * @unit Percent of nominal maximum storage [%]
 */
constexpr Register MINRSVPCT(40310, 1, RegType::UINT16);

/**
 * @brief
 * Available energy as a percent of capacity rating
 * @unit Percent [%]
 */
constexpr Register CHASTATE(40311, 1, RegType::UINT16);

/**
 * @brief
 * Charge status of the storage device
 * @note Enumerated values: 1=OFF, 2=EMPTY, 3=DISCHARGING, 4=CHARGING, 5=FULL,
 * 6=HOLDING, 7=TESTING
 */
constexpr Register CHAST(40314, 1, RegType::UINT16);

/**
 * @brief
 * Percent of maximum discharge rate
 * @unit Percent of WChaMax [%]
 * @note Valid range -100.00% to +100.00%; uses scale factor InOutWRte_SF.
 */
constexpr Register OUTWRTE(40315, 1, RegType::INT16);

/**
 * @brief
 * Percent of maximum charge rate
 * @unit Percent of WChaMax [%]
 * @note Valid range -100.00% to +100.00%; uses scale factor InOutWRte_SF.
 */
constexpr Register INWRTE(40316, 1, RegType::INT16);

/**
 * @brief
 * Setpoint to enable or disable charging from grid
 * @note Enumerated values: 0=PV (grid charging disabled), 1=GRID (grid charging
 * enabled)
 */
constexpr Register CHAGRISET(40320, 1, RegType::UINT16);

/** @brief Scale factor for maximum charge */
constexpr Register WCHAMAX_SF(40321, 1, RegType::INT16);

/** @brief Scale factor for maximum charge and discharge rate */
constexpr Register WCHADISCHAGRA_SF(40322, 1, RegType::INT16);

/** @brief Scale factor for minimum reserve percentage */
constexpr Register MINRSVPCT_SF(40324, 1, RegType::INT16);

/** @brief Scale factor for available energy percent */
constexpr Register CHASTATE_SF(40325, 1, RegType::INT16);

/** @brief Scale factor for percent charge/discharge rate */
constexpr Register INOUTWRTE_SF(40328, 1, RegType::INT16);

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
 * @brief Offset to convert integer+scale factor register addresses to float
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the float
 * register address corresponding to an integer+scale register, add this
 * offset to the integer+scale register address (`ADDR`).
 */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief
 * Uniquely identifies this as a SunSpec Multiple MPPT Inverter
 * Extension Model.
 * @return 160
 */
constexpr Register ID(40253, 1, RegType::UINT16);

/**
 * @brief
 * Length of Multiple MPPT Inverter Extension Model.
 * @return 48
 */
constexpr Register L(40254, 1, RegType::UINT16);

/** @brief DC current scale factor */
constexpr Register DCA_SF(40255, 1, RegType::INT16);

/** @brief DC voltage scale factor */
constexpr Register DCV_SF(40256, 1, RegType::INT16);

/** @brief DC power scale factor */
constexpr Register DCW_SF(40257, 1, RegType::INT16);

/** @brief DC energy scale factor
 * @note Not supported for Fronius Hybrid inverters
 */
constexpr Register DCWH_SF(40258, 1, RegType::INT16);

/** @brief Global events register. */
constexpr Register EVT(40259, 2, RegType::UINT32);

/**
 * @brief
 * Number of DC modules.
 * @return 2
 */
constexpr Register N(40261, 1, RegType::UINT16);

/**
 * @brief
 * Input ID of first DC input.
 * @return 1
 */
constexpr Register ID_1(40263, 1, RegType::UINT16);

/**
 * @brief
 * Input ID string of first DC input.
 * @return "String 1"
 */
constexpr Register IDSTR_1(40264, 8, RegType::STRING);

/**
 * @brief
 * DC current of first input.
 * @unit Ampere [A]
 */
constexpr Register DCA_1(40272, 1, RegType::UINT16);

/**
 * @brief
 * DC voltage of first input.
 * @unit Volt [V]
 */
constexpr Register DCV_1(40273, 1, RegType::UINT16);

/**
 * @brief
 * DC power of first input.
 * @unit Watt [W]
 */
constexpr Register DCW_1(40274, 1, RegType::UINT16);

/**
 * @brief
 * DC lifetime energy of first input.
 * @note Not supported for Fronius Hybrid inverters
 * @unit Watt-hour [Wh]
 */
constexpr Register DCWH_1(40275, 2, RegType::UINT32);

/** @brief Timestamp of first DC input since 01-Jan-2000 00:00 UTC. */
constexpr Register TMS_1(40277, 2, RegType::UINT32);

/**
 * @brief
 * Temperature of first DC input.
 * @unit Celsius [°C]
 */
constexpr Register TMP_1(40279, 1, RegType::INT16);

/** @brief Operating state of first DC input. */
constexpr Register DCST_1(40280, 1, RegType::UINT16);

/** @brief Module events of first DC input. */
constexpr Register DCEVT_1(40281, 2, RegType::UINT32);

/**
 * @brief
 * Input ID of second DC input.
 * @return 2
 */
constexpr Register ID_2(40283, 1, RegType::UINT16);

/**
 * @brief
 * Input ID string of second DC input.
 * @return "String 2" or "not supported"
 */
constexpr Register IDSTR_2(40284, 8, RegType::STRING);

/**
 * @brief
 * DC current of second input.
 * @unit Ampere [A]
 * @note Not supported if only one DC input
 */
constexpr Register DCA_2(40292, 1, RegType::UINT16);

/**
 * @brief
 * DC voltage of second input.
 * @unit Volt [V]
 * @note Not supported if only one DC input
 */
constexpr Register DCV_2(40293, 1, RegType::UINT16);

/**
 * @brief
 * DC power of second input.
 * @unit Watt [W]
 * @note Not supported if only one DC input
 */
constexpr Register DCW_2(40294, 1, RegType::UINT16);

/**
 * @brief
 * DC lifetime energy of second input.
 * @note Not supported for Fronius Hybrid inverters
 * @unit Watt-hour [Wh]
 */
constexpr Register DCWH_2(40295, 2, RegType::UINT16);

/**
 * @brief
 * Timestamp of second DC input since 01-Jan-2000 00:00 UTC.
 * @note Not supported if only one DC input
 */
constexpr Register TMS_2(40297, 2, RegType::UINT32);

/**
 * @brief
 * Temperature of second DC input.
 * @unit Celsius [°C]
 * @note Not supported if only one DC input
 */
constexpr Register TMP_2(40299, 1, RegType::INT16);

/**
 * @brief
 * Operating state of second DC input.
 * @note Not supported if only one DC input
 */
constexpr Register DCST_2(40300, 1, RegType::UINT16);

/**
 * @brief
 * Module events of second DC input.
 * @note Not supported if only one DC input
 */
constexpr Register DCEVT_2(40301, 2, RegType::UINT32);

} // namespace I160

/**
 * @namespace I_END
 * @brief SunSpec end-of-block registers.
 *
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
 * @brief Offset to convert integer+scale factor register addresses to float
 * addresses.
 *
 * @details
 * In Fronius devices, certain SunSpec float registers are implemented as
 * integer registers with associated scale factors. To obtain the float
 * register address corresponding to an integer+scale register, add this
 * offset to the integer+scale register address (`ADDR`).
 */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Offset for storage-enabled (hybrid) inverter register maps.
 *
 * @details
 * Some Fronius inverters, specifically hybrid models that include a
 * Basic Storage Control (BSC) register map, have their END register
 * block shifted by a fixed number of registers compared to
 * non-hybrid models.
 *
 * This constant defines that address offset. For example:
 * - On **non-hybrid** inverters (no storage map), the END::ID register starts
 * at address **40303**.
 * - On **hybrid** inverters (with storage map), the END::ID register starts at
 * address **40329**.
 *
 * Thus, the address difference between hybrid and non-hybrid models is 26
 * registers.
 */
constexpr uint16_t STORAGE_OFFSET = 26;

/**
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @return Always returns 0xFFFF.
 *
 * @note For hybrid inverters that include a Basic Storage Control (BSC)
 * register map, the end register address is offset by 26 registers. Take this
 * into account when iterating over SunSpec blocks or calculating the next
 * model's start address.
 */
constexpr Register ID(40303, 1, RegType::UINT16);

/**
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @return Always returns 0.
 */
constexpr Register L(40304, 1, RegType::UINT16);

} // namespace I_END

#endif /* INVERTER_REGISTERS_H_ */
