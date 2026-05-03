/**
 * @file inverter_registers.h
 * @brief SunSpec inverter and Fronius-specific register definitions.
 *
 * @details
 * Symbolic register addresses for the SunSpec inverter models and the
 * Fronius proprietary extensions, organised into the following namespaces:
 *
 * - **F**     – Fronius proprietary Modbus extensions.
 * - **I10X**  – SunSpec inverter integer + scale-factor models (101–103).
 * - **I11X**  – SunSpec inverter float models (111–113).
 * - **I120**  – SunSpec Nameplate model.
 * - **I122**  – SunSpec Extended Measurements & Status model.
 * - **I123**  – SunSpec Immediate Controls model.
 * - **I124**  – SunSpec Basic Storage Controls model.
 * - **I160**  – SunSpec Multiple MPPT Extension model.
 * - **I_END** – SunSpec end-of-map marker.
 *
 * Several namespaces define a `FLOAT_OFFSET` constant. In Fronius devices,
 * some SunSpec float-defined registers are implemented as integer
 * registers with associated scale factors. To compute the float-map
 * address, add `FLOAT_OFFSET` to the integer register's `ADDR`.
 */

#ifndef INVERTER_REGISTERS_H_
#define INVERTER_REGISTERS_H_

#include "register_base.h"
#include <cstdint>

/**
 * @namespace F
 * @brief Fronius proprietary Modbus extensions.
 *
 * Vendor registers that complement the SunSpec maps: device management
 * triggers, vendor inverter state, and site-level totals.
 */
namespace F {

/**
 * @brief Delete stored rating data of the current inverter.
 *
 * Writing `0xFFFF` deletes the stored rating data for the addressed
 * inverter.
 */
constexpr Register DELETE_DATA(211, 1, Register::Type::UINT16);

/**
 * @brief Persistently store rating data.
 *
 * Writing `0xFFFF` stores rating data of all inverters connected to the
 * Fronius Datamanager.
 */
constexpr Register STORE_DATA(212, 1, Register::Type::UINT16);

/**
 * @brief Current vendor-specific inverter state code.
 *
 * Refer to the inverter manual for state-code details.
 *
 * @note Not supported for Fronius Hybrid inverters. Behaviour can differ
 *       at night vs. daytime on some models.
 */
constexpr Register ACTIVE_STATE_CODE(213, 1, Register::Type::UINT16);

/**
 * @brief Reset all event flags and the active state code.
 *
 * Writing `0xFFFF` clears the event flag registers and the active state
 * code register.
 */
constexpr Register RESET_ALL_EVENT_FLAGS(214, 1, Register::Type::UINT16);

/**
 * @brief Select SunSpec model encoding for inverter and meter data.
 *
 * After writing the desired value, write `6` to confirm the change.
 *
 * - `1`: floating-point models
 * - `2`: integer + scale-factor models
 */
constexpr Register MODEL_TYPE(215, 1, Register::Type::UINT16);

/**
 * @brief Scope of storage restriction reporting in I124.
 *
 * - `0`: local restrictions (default; set via Modbus)
 * - `1`: global restrictions (system-wide)
 */
constexpr Register STORAGE_RESTRICTIONS_VIEW_MODE(216, 1,
                                                  Register::Type::UINT16);

/**
 * @brief Total instantaneous active power across all connected inverters.
 * @unit Watt [W]
 */
constexpr Register SITE_POWER(499, 2, Register::Type::UINT32);

/**
 * @brief Total energy produced today across all connected inverters
 *        (since midnight).
 * @unit Watt-hour [Wh]
 */
constexpr Register SITE_ENERGY_DAY(501, 4, Register::Type::UINT64);

/**
 * @brief Total energy produced this calendar year across all connected
 *        inverters.
 * @unit Watt-hour [Wh]
 */
constexpr Register SITE_ENERGY_YEAR(505, 4, Register::Type::UINT64);

/**
 * @brief Lifetime total energy produced across all connected inverters.
 * @unit Watt-hour [Wh]
 */
constexpr Register SITE_ENERGY_TOTAL(509, 4, Register::Type::UINT64);

} // namespace F

/**
 * @namespace I10X
 * @brief SunSpec inverter model — integer + scale factor variant.
 *
 * Models 101 (single-phase), 102 (split-phase), and 103 (three-phase).
 * Physical quantities are 16-bit integers paired with scale-factor
 * registers (`*_SF`).
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
constexpr Register ID(40069, 1, Register::Type::UINT16);

/**
 * @brief Length of inverter model block.
 * @returns Always returns 50.
 */
constexpr Register L(40070, 1, Register::Type::UINT16);

/**
 * @brief AC total current.
 * @unit Amperes [A]
 */
constexpr Register A(40071, 1, Register::Type::UINT16);

/**
 * @brief AC current in phase A.
 * @unit Amperes [A]
 */
constexpr Register APHA(40072, 1, Register::Type::UINT16);

/**
 * @brief AC current in phase B.
 * @unit Amperes [A]
 */
constexpr Register APHB(40073, 1, Register::Type::UINT16);

/**
 * @brief AC current in phase C.
 * @unit Amperes [A]
 */
constexpr Register APHC(40074, 1, Register::Type::UINT16);

/** @brief AC current scale factor. */
constexpr Register A_SF(40075, 1, Register::Type::INT16);

/**
 * @brief AC voltage phase-to-phase AB.
 * @unit Volts [V]
 */
constexpr Register PPVPHAB(40076, 1, Register::Type::UINT16);

/**
 * @brief AC voltage phase-to-phase BC.
 * @unit Volts [V]
 */
constexpr Register PPVPHBC(40077, 1, Register::Type::UINT16);

/**
 * @brief AC voltage phase-to-phase CA.
 * @unit Volts [V]
 */
constexpr Register PPVPHCA(40078, 1, Register::Type::UINT16);

/**
 * @brief AC voltage phase A to neutral.
 * @unit Volts [V]
 */
constexpr Register PHVPHA(40079, 1, Register::Type::UINT16);

/**
 * @brief AC voltage phase B to neutral.
 * @unit Volts [V]
 */
constexpr Register PHVPHB(40080, 1, Register::Type::UINT16);

/**
 * @brief AC voltage phase C to neutral.
 * @unit Volts [V]
 */
constexpr Register PHVPHC(40081, 1, Register::Type::UINT16);

/** @brief AC voltage scale factor. */
constexpr Register V_SF(40082, 1, Register::Type::INT16);

/**
 * @brief AC active power.
 * @unit Watts [W]
 */
constexpr Register W(40083, 1, Register::Type::INT16);

/** @brief AC active power scale factor. */
constexpr Register W_SF(40084, 1, Register::Type::INT16);

/**
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(40085, 1, Register::Type::UINT16);

/** @brief AC frequency scale factor. */
constexpr Register FREQ_SF(40086, 1, Register::Type::INT16);

/**
 * @brief Apparent power.
 * @unit Volt-amperes [VA]
 */
constexpr Register VA(40087, 1, Register::Type::INT16);

/** @brief Apparent power scale factor. */
constexpr Register VA_SF(40088, 1, Register::Type::INT16);

/**
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(40089, 1, Register::Type::INT16);

/** @brief Reactive power scale factor. */
constexpr Register VAR_SF(40090, 1, Register::Type::INT16);

/**
 * @brief Power factor.
 * @unit Percent [%]
 */
constexpr Register PF(40091, 1, Register::Type::INT16);

/** @brief Power factor scale factor. */
constexpr Register PF_SF(40092, 1, Register::Type::INT16);

/**
 * @brief AC lifetime energy production.
 * @unit Watt-hours [Wh]
 */
constexpr Register WH(40093, 2, Register::Type::UINT32);

/** @brief AC lifetime energy scale factor. */
constexpr Register WH_SF(40095, 1, Register::Type::INT16);

/**
 * @brief DC current.
 * @unit Amperes [A]
 */
constexpr Register DCA(40096, 1, Register::Type::UINT16);

/** @brief DC current scale factor. */
constexpr Register DCA_SF(40097, 1, Register::Type::INT16);

/**
 * @brief DC voltage.
 * @unit Volts [V]
 */
constexpr Register DCV(40098, 1, Register::Type::UINT16);

/** @brief DC voltage scale factor. */
constexpr Register DCV_SF(40099, 1, Register::Type::INT16);

/**
 * @brief DC power.
 * @unit Watts [W]
 */
constexpr Register DCW(40100, 1, Register::Type::INT16);

/** @brief DC power scale factor. */
constexpr Register DCW_SF(40101, 1, Register::Type::INT16);

/** @brief Inverter operating state. */
constexpr Register ST(40107, 1, Register::Type::UINT16);

/** @brief Vendor-defined operating state. */
constexpr Register STVND(40108, 1, Register::Type::UINT16);

/** @brief Event flags bits 0–31. */
constexpr Register EVT1(40109, 2, Register::Type::UINT32);

/** @brief Event flags bits 32–63. */
constexpr Register EVT2(40111, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags bits 0–31. */
constexpr Register EVTVND1(40113, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags bits 32–63. */
constexpr Register EVTVND2(40115, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags bits 64–95. */
constexpr Register EVTVND3(40117, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags bits 96–127. */
constexpr Register EVTVND4(40119, 2, Register::Type::UINT32);

} // namespace I10X

/**
 * @namespace I11X
 * @brief SunSpec inverter model — float variant.
 *
 * Models 111 (single-phase), 112 (split-phase), and 113 (three-phase).
 * Physical quantities are 32-bit IEEE floats; no scale factors needed.
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
constexpr Register ID(40069, 1, Register::Type::UINT16);

/**
 * @brief Length of inverter model block.
 * @returns Always returns 60.
 */
constexpr Register L(40070, 1, Register::Type::UINT16);

/**
 * @brief AC total current.
 * @unit Ampere [A]
 */
constexpr Register A(40071, 2, Register::Type::FLOAT);

/**
 * @brief AC current in phase A.
 * @unit Ampere [A]
 */
constexpr Register APHA(40073, 2, Register::Type::FLOAT);

/**
 * @brief AC current in phase B.
 * @unit Ampere [A]
 */
constexpr Register APHB(40075, 2, Register::Type::FLOAT);

/**
 * @brief AC current in phase C.
 * @unit Ampere [A]
 */
constexpr Register APHC(40077, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase-to-phase AB.
 * @unit Volt [V]
 */
constexpr Register PPVPHAB(40079, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase-to-phase BC.
 * @unit Volt [V]
 */
constexpr Register PPVPHBC(40081, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase-to-phase CA.
 * @unit Volt [V]
 */
constexpr Register PPVPHCA(40083, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase-A to neutral.
 * @unit Volt [V]
 */
constexpr Register PHVPHA(40085, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase-B to neutral.
 * @unit Volt [V]
 */
constexpr Register PHVPHB(40087, 2, Register::Type::FLOAT);

/**
 * @brief AC voltage phase-C to neutral.
 * @unit Volt [V]
 */
constexpr Register PHVPHC(40089, 2, Register::Type::FLOAT);

/**
 * @brief AC total power.
 * @unit Watt [W]
 */
constexpr Register W(40091, 2, Register::Type::FLOAT);

/**
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
constexpr Register FREQ(40093, 2, Register::Type::FLOAT);

/**
 * @brief Apparent power.
 * @unit Volt-ampere [VA]
 */
constexpr Register VA(40095, 2, Register::Type::FLOAT);

/**
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VAR(40097, 2, Register::Type::FLOAT);

/**
 * @brief Power factor.
 * @unit Percent [%]
 */
constexpr Register PF(40099, 2, Register::Type::FLOAT);

/**
 * @brief Lifetime energy production.
 * @unit Watt-hour [Wh]
 */
constexpr Register WH(40101, 2, Register::Type::FLOAT);

/**
 * @brief DC current.
 * @unit Ampere [A]
 */
constexpr Register DCA(40103, 2, Register::Type::FLOAT);

/**
 * @brief DC voltage.
 * @unit Volt [V]
 */
constexpr Register DCV(40105, 2, Register::Type::FLOAT);

/**
 * @brief DC power.
 * @unit Watt [W]
 */
constexpr Register DCW(40107, 2, Register::Type::FLOAT);

/** @brief Inverter operating state. */
constexpr Register ST(40117, 1, Register::Type::UINT16);

/** @brief Vendor-defined operating state. */
constexpr Register STVND(40118, 1, Register::Type::UINT16);

/** @brief Event flags (bits 0–31). */
constexpr Register EVT1(40119, 2, Register::Type::UINT32);

/** @brief Event flags (bits 32–63). */
constexpr Register EVT2(40121, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags (bits 0–31). */
constexpr Register EVTVND1(40123, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags (bits 32–63). */
constexpr Register EVTVND2(40125, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags (bits 64–95). */
constexpr Register EVTVND3(40127, 2, Register::Type::UINT32);

/** @brief Vendor-defined event flags (bits 96–127). */
constexpr Register EVTVND4(40129, 2, Register::Type::UINT32);

} // namespace I11X

/**
 * @namespace I120
 * @brief SunSpec Nameplate Model (ID 120).
 *
 * Static electrical capabilities of the inverter: rated power, current,
 * voltage, and supported quadrants. Defines `FLOAT_OFFSET` — see file
 * overview for usage.
 */
namespace I120 {

/** @brief Total number of registers in the nameplate model. */
constexpr uint16_t SIZE = 26;

/** @brief Offset to derive the float-map address from the integer-map address.
 * See file overview. */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Uniquely identifies this as a SunSpec Nameplate Model.
 * @return 120
 */
constexpr Register ID(40121, 1, Register::Type::UINT16);

/**
 * @brief Length of Nameplate Model block.
 * @return Always 26
 */
constexpr Register L(40122, 1, Register::Type::UINT16);

/** @brief Type of DER device. Default value is 4 to indicate a PV device. */
constexpr Register DERTYP(40123, 1, Register::Type::UINT16);

/**
 * @brief Continuous power output capability of the inverter.
 * @unit Watt [W]
 */
constexpr Register WRTG(40124, 1, Register::Type::UINT16);

/** @brief Continuous power output scale factor. */
constexpr Register WRTG_SF(40125, 1, Register::Type::INT16);

/**
 * @brief Continuous Volt-Ampere capability of the inverter.
 * @unit Volt-ampere [VA]
 */
constexpr Register VARTG(40126, 1, Register::Type::UINT16);

/** @brief Continuous Volt-Ampere capability scale factor. */
constexpr Register VARTG_SF(40127, 1, Register::Type::INT16);

/**
 * @brief Continuous VAR capability of the inverter in quadrant 1.
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARRTGQ1(40128, 1, Register::Type::INT16);

/**
 * @brief Continuous VAR capability of the inverter in quadrant 4.
 * @unit Volt-ampere reactive [VAr]
 */
constexpr Register VARRTGQ4(40131, 1, Register::Type::INT16);

/** @brief Continuous VAR capability scale factor. */
constexpr Register VARRTG_SF(40132, 1, Register::Type::INT16);

/**
 * @brief Maximum RMS AC current level capability of the inverter.
 * @unit Ampere [A]
 */
constexpr Register ARTG(40133, 1, Register::Type::UINT16);

/** @brief Maximum RMS AC current level scale factor. */
constexpr Register ARTG_SF(40134, 1, Register::Type::INT16);

/**
 * @brief Minimum power factor capability of the inverter in quadrant 1.
 * @unit cos(phi)
 */
constexpr Register PFRTGQ1(40135, 1, Register::Type::INT16);

/**
 * @brief Minimum power factor capability of the inverter in quadrant 4.
 * @unit cos(phi)
 */
constexpr Register PFRTGQ4(40138, 1, Register::Type::INT16);

/** @brief Minimum power factor capability scale factor. */
constexpr Register PFRTG_SF(40139, 1, Register::Type::INT16);

/**
 * @brief Nominal energy rating of storage device.
 * @unit Watt-hour [Wh]
 */
constexpr Register WHRTG(40140, 1, Register::Type::UINT16);

/** @brief Nominal energy rating scale factor. */
constexpr Register WHRTG_SF(40141, 1, Register::Type::INT16);

/**
 * @brief Maximum rate of energy transfer into the storage device.
 * @unit Watt [W]
 */
constexpr Register MAXCHARTE(40144, 1, Register::Type::UINT16);

/** @brief Charge rate scale factor. */
constexpr Register MAXCHARTE_SF(40145, 1, Register::Type::INT16);

/**
 * @brief Maximum rate of energy transfer out of the storage device.
 * @unit Watt [W]
 */
constexpr Register MAXDISCHARTE(40146, 1, Register::Type::UINT16);

/** @brief Discharge rate scale factor. */
constexpr Register MAXDISCHARTE_SF(40147, 1, Register::Type::INT16);

} // namespace I120

/**
 * @namespace I122
 * @brief SunSpec Extended Measurements & Status Model (ID 122).
 *
 * Extended measurement and inverter status registers (e.g. cumulative
 * imported/exported energy, control-state bitmasks, time sync source).
 * Defines `FLOAT_OFFSET` — see file overview for usage.
 */
namespace I122 {

/** @brief Total number of registers in the extended model. */
constexpr uint16_t SIZE = 44;

/** @brief Offset to derive the float-map address from the integer-map address.
 * See file overview. */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Model identifier.
 * @return Always 122.
 */
constexpr Register ID(40181, 1, Register::Type::UINT16);

/**
 * @brief Length of the Extended Measurements & Status block.
 * @return Always 44.
 */
constexpr Register L(40182, 1, Register::Type::UINT16);

/**
 * @brief PV inverter present/available status (bitmask).
 *
 * Bit 0: Connected, Bit 1: Available, Bit 2: Operating, Bit 3: Test.
 */
constexpr Register PVCONN(40183, 1, Register::Type::UINT16);

/**
 * @brief Storage inverter present/available status (bitmask).
 *
 * Bit 0: Connected, Bit 1: Available, Bit 2: Operating, Bit 3: Test.
 */
constexpr Register STORCONN(40184, 1, Register::Type::UINT16);

/**
 * @brief ECP connection status.
 *
 * 0: Disconnected, 1: Connected.
 */
constexpr Register ECPCONN(40185, 1, Register::Type::UINT16);

/**
 * @brief AC lifetime active (real) energy output.
 * @unit Watt-hour [Wh]
 */
constexpr Register ACTWH(40186, 4, Register::Type::UINT64);

/**
 * @brief Bitmask of currently active inverter controls.
 *
 * Bit 0: FixedW, Bit 1: FixedVAR, Bit 2: FixedPF.
 */
constexpr Register STACTCTL(40216, 2, Register::Type::UINT32);

/**
 * @brief Source of time synchronisation (e.g. "RTC").
 */
constexpr Register TMSSRC(40218, 4, Register::Type::STRING);

/**
 * @brief Timestamp, seconds since 01-Jan-2000 00:00 UTC.
 */
constexpr Register TMS(40222, 2, Register::Type::UINT32);

} // namespace I122

/**
 * @namespace I123
 * @brief SunSpec Immediate Controls Model (ID 123).
 *
 * Writable control points — connect/disconnect, fixed-power, fixed-PF,
 * fixed-VAR setpoints with their time windows and ramp/revert timers.
 * Defines `FLOAT_OFFSET` — see file overview for usage.
 */
namespace I123 {

/** @brief Total number of registers in the Immediate Controls model. */
constexpr uint16_t SIZE = 24;

/** @brief Offset to derive the float-map address from the integer-map address.
 * See file overview. */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Model identifier.
 * @return Always 123.
 */
constexpr Register ID(40227, 1, Register::Type::UINT16);

/**
 * @brief Length of the Immediate Controls block.
 * @return Always 24.
 */
constexpr Register L(40228, 1, Register::Type::UINT16);

/**
 * @brief Time window for connect/disconnect.
 * @unit Second [s]
 */
constexpr Register CONN_WINTMS(40229, 1, Register::Type::UINT16);

/**
 * @brief Timeout period for connect/disconnect.
 * @unit Second [s]
 */
constexpr Register CONN_RVRTTMS(40230, 1, Register::Type::UINT16);

/**
 * @brief Connection control. 0: Disconnected, 1: Connected.
 */
constexpr Register CONN(40231, 1, Register::Type::UINT16);

/**
 * @brief Set power output to the specified level.
 * @unit Percent of WMax [%]
 */
constexpr Register WMAXLIMPCT(40232, 1, Register::Type::UINT16);

/**
 * @brief Time window for power limit change. Range 0–300.
 * @unit Second [s]
 */
constexpr Register WMAXLIMPCT_WINTMS(40233, 1, Register::Type::UINT16);

/**
 * @brief Timeout period for power limit. Range 0–28800.
 * @unit Second [s]
 */
constexpr Register WMAXLIMPCT_RVRTTMS(40234, 1, Register::Type::UINT16);

/**
 * @brief Ramp time from current to new setpoint. Range 0–65534
 *        (`0xFFFF` is treated as `0x0000`).
 * @unit Second [s]
 */
constexpr Register WMAXLIMPCT_RMPTMS(40235, 1, Register::Type::UINT16);

/**
 * @brief Throttle enable/disable. 0: Disabled, 1: Enabled.
 */
constexpr Register WMAXLIM_ENA(40236, 1, Register::Type::UINT16);

/**
 * @brief Set power factor.
 * @unit cos(phi); valid range 0.8 to 1.0 and -0.8 to -0.999
 */
constexpr Register OUTPFSET(40237, 1, Register::Type::INT16);

/**
 * @brief Time window for power-factor change. Range 0–300.
 * @unit Second [s]
 */
constexpr Register OUTPFSET_WINTMS(40238, 1, Register::Type::UINT16);

/**
 * @brief Timeout period for power factor. Range 0–28800.
 * @unit Second [s]
 */
constexpr Register OUTPFSET_RVRTTMS(40239, 1, Register::Type::UINT16);

/**
 * @brief Ramp time from current to new setpoint. Range 0–65534
 *        (`0xFFFF` is treated as `0x0000`).
 * @unit Second [s]
 */
constexpr Register OUTPFSET_RMPTMS(40240, 1, Register::Type::UINT16);

/**
 * @brief Fixed power-factor enable/disable. 0: Disabled, 1: Enabled.
 */
constexpr Register OUTPFSET_ENA(40241, 1, Register::Type::UINT16);

/**
 * @brief Reactive power as a percent of I_VArMax.
 */
constexpr Register VARMAXPCT(40243, 1, Register::Type::INT16);

/**
 * @brief Time window for VAR limit change. Range 0–300.
 * @unit Second [s]
 */
constexpr Register VARPCT_WINTMS(40245, 1, Register::Type::UINT16);

/**
 * @brief Timeout period for VAR limit. Range 0–28800.
 * @unit Second [s]
 */
constexpr Register VARPCT_RVRTTMS(40246, 1, Register::Type::UINT16);

/**
 * @brief Ramp time from current to new setpoint. Range 0–65534
 *        (`0xFFFF` is treated as `0x0000`).
 * @unit Second [s]
 */
constexpr Register VARPCT_RMPTMS(40247, 1, Register::Type::UINT16);

/**
 * @brief VAR limit mode. 2: VAR limit as a percent of VArMax.
 */
constexpr Register VARPCT_MOD(40248, 1, Register::Type::UINT16);

/**
 * @brief Fixed VAR enable/disable. 0: Disabled, 1: Enabled.
 */
constexpr Register VARPCT_ENA(40249, 1, Register::Type::UINT16);

/** @brief Scale factor for power output percent. */
constexpr Register WMAXLIMPCT_SF(40250, 1, Register::Type::INT16);

/** @brief Scale factor for power factor. */
constexpr Register OUTPFSET_SF(40251, 1, Register::Type::INT16);

/** @brief Scale factor for reactive power. */
constexpr Register VARPCT_SF(40252, 1, Register::Type::INT16);

} // namespace I123

/**
 * @namespace I124
 * @brief SunSpec Basic Storage Controls Model (ID 124).
 *
 * Storage charge/discharge setpoints and state. Although specified as a
 * float model in SunSpec, Fronius devices implement these as integer
 * registers with scale factors. Defines `FLOAT_OFFSET` — see file overview
 * for usage.
 */
namespace I124 {

/** @brief Total number of registers in the storage model (float) */
constexpr uint16_t SIZE = 26;

/** @brief Offset to derive the float-map address from the integer-map address.
 * See file overview. */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Model identifier.
 * @return Always 124.
 */
constexpr Register ID(40303, 1, Register::Type::UINT16);

/**
 * @brief Length of the Basic Storage Controls block.
 * @return Always 24.
 */
constexpr Register L(40304, 1, Register::Type::UINT16);

/**
 * @brief Setpoint for maximum charge.
 * @unit Watt [W]
 * @note Combined with InWRte and OutWRte to define charge/discharge limits.
 */
constexpr Register WCHAMAX(40305, 1, Register::Type::UINT16);

/**
 * @brief Setpoint for the maximum charging rate.
 * @unit Percent of WChaMax per second [% WChaMax/s]
 */
constexpr Register WCHAGRA(40306, 1, Register::Type::UINT16);

/**
 * @brief Setpoint for the maximum discharging rate.
 * @unit Percent of WChaMax per second [% WChaMax/s]
 */
constexpr Register WDISCHAGRA(40307, 1, Register::Type::UINT16);

/**
 * @brief Storage control mode (bitfield).
 *
 * Bit 0: CHARGE, Bit 1: DISCHARGE.
 */
constexpr Register STORCTL_MOD(40308, 1, Register::Type::UINT16);

/**
 * @brief Setpoint for the minimum storage reserve.
 * @unit Percent of nominal maximum storage [%]
 */
constexpr Register MINRSVPCT(40310, 1, Register::Type::UINT16);

/**
 * @brief Available energy as a percent of capacity rating.
 * @unit Percent [%]
 */
constexpr Register CHASTATE(40311, 1, Register::Type::UINT16);

/**
 * @brief Charge status of the storage device.
 *
 * Enumeration: 1=OFF, 2=EMPTY, 3=DISCHARGING, 4=CHARGING, 5=FULL,
 * 6=HOLDING, 7=TESTING.
 */
constexpr Register CHAST(40314, 1, Register::Type::UINT16);

/**
 * @brief Percent of maximum discharge rate. Valid range -100.00 % to
 *        +100.00 %; uses scale factor `INOUTWRTE_SF`.
 * @unit Percent of WChaMax [%]
 */
constexpr Register OUTWRTE(40315, 1, Register::Type::INT16);

/**
 * @brief Percent of maximum charge rate. Valid range -100.00 % to
 *        +100.00 %; uses scale factor `INOUTWRTE_SF`.
 * @unit Percent of WChaMax [%]
 */
constexpr Register INWRTE(40316, 1, Register::Type::INT16);

/**
 * @brief Enable/disable charging from the grid.
 *
 * 0: PV (grid charging disabled), 1: GRID (grid charging enabled).
 */
constexpr Register CHAGRISET(40320, 1, Register::Type::UINT16);

/** @brief Scale factor for maximum charge. */
constexpr Register WCHAMAX_SF(40321, 1, Register::Type::INT16);

/** @brief Scale factor for maximum charge and discharge rate. */
constexpr Register WCHADISCHAGRA_SF(40322, 1, Register::Type::INT16);

/** @brief Scale factor for minimum reserve percent. */
constexpr Register MINRSVPCT_SF(40324, 1, Register::Type::INT16);

/** @brief Scale factor for available-energy percent. */
constexpr Register CHASTATE_SF(40325, 1, Register::Type::INT16);

/** @brief Scale factor for percent charge/discharge rate. */
constexpr Register INOUTWRTE_SF(40328, 1, Register::Type::INT16);

} // namespace I124

/**
 * @namespace I160
 * @brief SunSpec Multiple MPPT Extension Model (ID 160).
 *
 * Per-input DC data for inverters with multiple MPP trackers. Defines
 * `FLOAT_OFFSET` — see file overview for usage.
 */
namespace I160 {

/** @brief Total number of registers in the multiple MPPT model (float). */
constexpr uint16_t SIZE = 48;

/** @brief Offset to derive the float-map address from the integer-map address.
 * See file overview. */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Model identifier.
 * @return Always 160.
 */
constexpr Register ID(40253, 1, Register::Type::UINT16);

/**
 * @brief Length of the Multi-MPPT block.
 * @return Always 48.
 */
constexpr Register L(40254, 1, Register::Type::UINT16);

/** @brief DC current scale factor. */
constexpr Register DCA_SF(40255, 1, Register::Type::INT16);

/** @brief DC voltage scale factor. */
constexpr Register DCV_SF(40256, 1, Register::Type::INT16);

/** @brief DC power scale factor. */
constexpr Register DCW_SF(40257, 1, Register::Type::INT16);

/**
 * @brief DC energy scale factor.
 * @note Not supported on Fronius Hybrid inverters.
 */
constexpr Register DCWH_SF(40258, 1, Register::Type::INT16);

/** @brief Global events register. */
constexpr Register EVT(40259, 2, Register::Type::UINT32);

/**
 * @brief Number of DC modules.
 * @return Always 2.
 */
constexpr Register N(40261, 1, Register::Type::UINT16);

/**
 * @brief Numeric input ID of the first DC input.
 * @return Always 1.
 */
constexpr Register ID_1(40263, 1, Register::Type::UINT16);

/**
 * @brief Input name of the first DC input (e.g. "String 1").
 */
constexpr Register IDSTR_1(40264, 8, Register::Type::STRING);

/**
 * @brief DC current of the first input.
 * @unit Ampere [A]
 */
constexpr Register DCA_1(40272, 1, Register::Type::UINT16);

/**
 * @brief DC voltage of the first input.
 * @unit Volt [V]
 */
constexpr Register DCV_1(40273, 1, Register::Type::UINT16);

/**
 * @brief DC power of the first input.
 * @unit Watt [W]
 */
constexpr Register DCW_1(40274, 1, Register::Type::UINT16);

/**
 * @brief DC lifetime energy of the first input.
 * @unit Watt-hour [Wh]
 * @note Not supported on Fronius Hybrid inverters.
 */
constexpr Register DCWH_1(40275, 2, Register::Type::UINT32);

/** @brief Timestamp of the first DC input, seconds since 01-Jan-2000 UTC. */
constexpr Register TMS_1(40277, 2, Register::Type::UINT32);

/**
 * @brief Temperature of the first DC input.
 * @unit Celsius [°C]
 */
constexpr Register TMP_1(40279, 1, Register::Type::INT16);

/** @brief Operating state of the first DC input. */
constexpr Register DCST_1(40280, 1, Register::Type::UINT16);

/** @brief Module events of the first DC input. */
constexpr Register DCEVT_1(40281, 2, Register::Type::UINT32);

/**
 * @brief Numeric input ID of the second DC input.
 * @return Always 2.
 */
constexpr Register ID_2(40283, 1, Register::Type::UINT16);

/**
 * @brief Input name of the second DC input
 *        (e.g. "String 2" or "not supported").
 */
constexpr Register IDSTR_2(40284, 8, Register::Type::STRING);

/**
 * @brief DC current of the second input.
 * @unit Ampere [A]
 * @note Not supported if only one DC input is present.
 */
constexpr Register DCA_2(40292, 1, Register::Type::UINT16);

/**
 * @brief DC voltage of the second input.
 * @unit Volt [V]
 * @note Not supported if only one DC input is present.
 */
constexpr Register DCV_2(40293, 1, Register::Type::UINT16);

/**
 * @brief DC power of the second input.
 * @unit Watt [W]
 * @note Not supported if only one DC input is present.
 */
constexpr Register DCW_2(40294, 1, Register::Type::UINT16);

/**
 * @brief DC lifetime energy of the second input.
 * @unit Watt-hour [Wh]
 * @note Not supported on Fronius Hybrid inverters.
 */
constexpr Register DCWH_2(40295, 2, Register::Type::UINT32);

/**
 * @brief Timestamp of the second DC input, seconds since 01-Jan-2000 UTC.
 * @note Not supported if only one DC input is present.
 */
constexpr Register TMS_2(40297, 2, Register::Type::UINT32);

/**
 * @brief Temperature of the second DC input.
 * @unit Celsius [°C]
 * @note Not supported if only one DC input is present.
 */
constexpr Register TMP_2(40299, 1, Register::Type::INT16);

/**
 * @brief Operating state of the second DC input.
 * @note Not supported if only one DC input is present.
 */
constexpr Register DCST_2(40300, 1, Register::Type::UINT16);

/**
 * @brief Module events of the second DC input.
 * @note Not supported if only one DC input is present.
 */
constexpr Register DCEVT_2(40301, 2, Register::Type::UINT32);

} // namespace I160

/**
 * @namespace I_END
 * @brief SunSpec end-of-map marker.
 *
 * Two registers (`ID = 0xFFFF`, `L = 0`) that mark the end of the SunSpec
 * register map. Defines `FLOAT_OFFSET` (see file overview) and a
 * `STORAGE_OFFSET` that shifts the end-block address on hybrid inverters
 * which include the BSC (I124) block.
 */
namespace I_END {

/** @brief Offset to derive the float-map address from the integer-map
 *         address. See file overview. */
constexpr uint16_t FLOAT_OFFSET = 10;

/**
 * @brief Address shift applied on storage-enabled (hybrid) inverters.
 *
 * Hybrid inverters with a Basic Storage Control (BSC) block place the
 * end-of-map marker 26 registers later than non-hybrid models. For
 * example, with the integer map: non-hybrid at 40303, hybrid at 40329.
 */
constexpr uint16_t STORAGE_OFFSET = 26;

/**
 * @brief End-of-block identifier.
 *
 * @return Always `0xFFFF`.
 *
 * @note The actual address depends on integer/float encoding (`FLOAT_OFFSET`)
 *       and on hybrid storage presence (`STORAGE_OFFSET`).
 */
constexpr Register ID(40303, 1, Register::Type::UINT16);

/**
 * @brief End-of-block length field.
 *
 * @return Always `0`.
 */
constexpr Register L(40304, 1, Register::Type::UINT16);

} // namespace I_END

#endif /* INVERTER_REGISTERS_H_ */
