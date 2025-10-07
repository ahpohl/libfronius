/**
 * @file inverter_intsf_registers.h
 * @brief Defines SunSpec Inverter (Integer + Scale Factor) Model registers
 * (I10X) for Fronius devices
 *
 * @details
 * This header provides symbolic register definitions for the SunSpec Modbus
 * inverter model that uses integer registers with associated scale factors.
 * Each physical quantity has a corresponding scale factor register (e.g., A_SF,
 * V_SF) to obtain the correct value.
 *
 * Supported inverter model IDs:
 *  - **101** – Single-phase inverter
 *  - **102** – Split-phase inverter
 *  - **103** – Three-phase inverter
 *
 * @note Many registers require applying the associated scale factor to
 * calculate the actual physical value.
 */

#ifndef INVERTER_INTSF_REGISTERS_H_
#define INVERTER_INTSF_REGISTERS_H_

#include <cstdint>

/**
 * @namespace I10X_INT
 * @brief SunSpec Inverter (Integer + Scale Factor) Model registers (I10X)
 * for Fronius devices.
 */
namespace I10X_INT {

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
 * @struct HZ
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
struct HZ {
  static constexpr uint16_t ADDR = 40085;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct HZ_SF
 * @brief AC frequency scale factor.
 */
struct HZ_SF {
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

} // namespace I10X_INT

/**
 * @namespace I160_INT
 * @brief SunSpec Multi-MPPT Inverter (Integer) Model registers.
 */
namespace I160_INT {

/**
 * @struct ID
 * @brief Uniquely identifies this as a SunSpec Multiple MPPT Inverter Extension
 * Model.
 * @returns 160
 */
struct ID {
  static constexpr uint16_t ADDR = 40253;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct L
 * @brief Length of Multiple MPPT Inverter Extension Model.
 * @returns 48
 */
struct L {
  static constexpr uint16_t ADDR = 40254;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCA_SF
 * @brief DC current scale factor
 */
struct DCA_SF {
  static constexpr uint16_t ADDR = 40255;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCV_SF
 * @brief DC voltage scale factor
 */
struct DCV_SF {
  static constexpr uint16_t ADDR = 40256;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCW_SF
 * @brief DC power scale factor
 */
struct DCW_SF {
  static constexpr uint16_t ADDR = 40257;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct DCWH_SF
 * @brief DC energy scale factor
 * @note Not supported for Fronius Hybrid inverters
 */
struct DCWH_SF {
  static constexpr uint16_t ADDR = 40258;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct EVT
 * @brief Global Events
 */
struct EVT {
  static constexpr uint16_t ADDR = 40259;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct N
 * @brief Number of modules
 * @returns 2
 */
struct N {
  static constexpr uint16_t ADDR = 40261;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct TMS_PER
 * @brief Timestamp Period
 * @note Not supported
 */
struct TMS_PER {
  static constexpr uint16_t ADDR = 40262;
  static constexpr uint16_t NB = 1;
};

// --- Input 1 ---

/**
 * @struct INPUT1_ID
 * @brief Input ID
 * @returns 1
 */
struct INPUT1_ID {
  static constexpr uint16_t ADDR = 40263;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCA
 * @brief DC current
 * @unit Amperes [A]
 */
struct INPUT1_DCA {
  static constexpr uint16_t ADDR = 40272;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCV
 * @brief DC voltage
 * @unit Volts [V]
 */
struct INPUT1_DCV {
  static constexpr uint16_t ADDR = 40273;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCW
 * @brief DC power
 * @unit Watts [W]
 */
struct INPUT1_DCW {
  static constexpr uint16_t ADDR = 40274;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCWH
 * @brief DC lifetime energy
 * @unit Watt-hours [Wh]
 * @note Not supported for Fronius Hybrid inverters
 */
struct INPUT1_DCWH {
  static constexpr uint16_t ADDR = 40275;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT1_TMS
 * @brief Timestamp since 01-Jan-2000 00:00 UTC
 */
struct INPUT1_TMS {
  static constexpr uint16_t ADDR = 40277;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT1_TMP
 * @brief Temperature
 * @unit Celsius [°C]
 */
struct INPUT1_TMP {
  static constexpr uint16_t ADDR = 40279;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCST
 * @brief Operating state
 */
struct INPUT1_DCST {
  static constexpr uint16_t ADDR = 40280;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT1_DCEVT
 * @brief Module events
 */
struct INPUT1_DCEVT {
  static constexpr uint16_t ADDR = 40281;
  static constexpr uint16_t NB = 2;
};

// --- Input 2 ---

/**
 * @struct INPUT2_ID
 * @brief Input ID
 * @returns 2
 */
struct INPUT2_ID {
  static constexpr uint16_t ADDR = 40283;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCA
 * @brief DC current
 * @unit Amperes [A]
 * @note Not supported if only one DC input
 */
struct INPUT2_DCA {
  static constexpr uint16_t ADDR = 40292;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCV
 * @brief DC voltage
 * @unit Volts [V]
 * @note Not supported if only one DC input
 */
struct INPUT2_DCV {
  static constexpr uint16_t ADDR = 40293;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCW
 * @brief DC power
 * @unit Watts [W]
 * @note Not supported if only one DC input
 */
struct INPUT2_DCW {
  static constexpr uint16_t ADDR = 40294;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCWH
 * @brief DC lifetime energy
 * @unit Watt-hours [Wh]
 * @note Not supported for Fronius Hybrid inverters
 */
struct INPUT2_DCWH {
  static constexpr uint16_t ADDR = 40295;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT2_TMS
 * @brief Timestamp since 01-Jan-2000 00:00 UTC
 * @note Not supported if only one DC input
 */
struct INPUT2_TMS {
  static constexpr uint16_t ADDR = 40297;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct INPUT2_TMP
 * @brief Temperature
 * @unit Celsius [°C]
 * @note Not supported if only one DC input
 */
struct INPUT2_TMP {
  static constexpr uint16_t ADDR = 40299;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCST
 * @brief Operating state
 * @note Not supported if only one DC input
 */
struct INPUT2_DCST {
  static constexpr uint16_t ADDR = 40300;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct INPUT2_DCEVT
 * @brief Module events
 * @note Not supported if only one DC input
 */
struct INPUT2_DCEVT {
  static constexpr uint16_t ADDR = 40301;
  static constexpr uint16_t NB = 2;
};

} // namespace I160_INT

/**
 * @namespace END_INT
 * @brief End-of-block registers for SunSpec inverter models (I10X).
 *
 * This namespace contains the symbolic definitions for the end-of-block
 * registers that mark the end of the SunSpec inverter model block.
 */
namespace END_INT {

/**
 * @struct ID
 * @brief End-of-block identifier.
 * @details This register indicates the end of the SunSpec inverter model block.
 * @returns Always returns 0xFFFF.
 */
struct ID {
  static constexpr uint16_t ADDR = 40329; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

/**
 * @struct L
 * @brief End-of-block length field.
 * @details This register contains the length of the end-of-block segment.
 * @returns Always returns 0.
 */
struct L {
  static constexpr uint16_t ADDR = 40330; ///< Register start address.
  static constexpr uint16_t NB = 1;       ///< Number of registers.
};

} // namespace END_INT

#endif /* INVERTER_INTSF_REGISTERS_H_ */