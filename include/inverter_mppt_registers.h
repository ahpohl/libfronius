/**
 * @file inverter_mppt_registers.h
 * @brief Defines SunSpec Multiple MPPT Extension Model registers for Fronius
 * inverters.
 *
 * @details
 * This header defines symbolic Modbus register mappings for the SunSpec
 * Multiple MPPT (Maximum Power Point Tracking) Extension Model, which
 * supplements the core inverter models (101–103 and 111–113) by providing
 * detailed DC input data for each string.
 *
 * The SunSpec MPPT model (ID 160) allows reading current, voltage, power,
 * temperature, energy, and event status for up to two independent DC inputs.
 *
 * - The **I160::INT** namespace contains the **integer-based model** with scale
 *   factors, corresponding to the SunSpec "Integer + Scale Factor" variant.
 *
 * - The **I160::FLOAT** namespace contains the **floating-point model**, which
 *   uses 32-bit floating-point registers for measured quantities.
 *
 * @note
 * Some registers are not supported by all Fronius inverter types — particularly
 * Hybrid inverters — or when only a single DC input is available.
 */

#ifndef INVERTER_MPPT_REGISTERS_H_
#define INVERTER_MPPT_REGISTERS_H_

#include <cstdint>

/**
 * @namespace I160
 * @brief SunSpec Multiple MPPT Extension Model (ID 160).
 *
 * This namespace groups all registers of the SunSpec Multiple MPPT Extension
 * Model, which provides per-input DC data for inverters with multiple MPP
 * trackers. Each sub-namespace (INT and FLOAT) defines the register addresses
 * and block layout for a specific data type representation.
 */
namespace I160 {

/**
 * @namespace I160::FLOAT
 * @brief SunSpec Multiple MPPT Extension Model (Float) registers.
 *
 * Contains the register definitions for the SunSpec MPPT Extension Model (ID
 * 160) as implemented by Fronius for its floating-point inverter models
 * (111–113).
 *
 * According to the SunSpec specification, all measured quantities in this model
 * should be represented as 32-bit IEEE floating-point values. However, Fronius
 * uses a hybrid implementation:
 * - The register layout and numbering correspond to the official *float* model.
 * - The actual data encoding matches the *integer + scale factor*
 * representation.
 *
 * This means that although the register addresses are identical to the float
 * model, the values must still be interpreted as integers and scaled using the
 * respective scale factors to obtain correct physical units.
 *
 * @note This inconsistency is specific to Fronius and deviates from the
 * standard SunSpec definition. Always verify values by comparing against known
 * measurements.
 */

namespace FLOAT {

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

} // namespace FLOAT

/**
 * @namespace I160::INT
 * @brief SunSpec Multiple MPPT Extension Model (Integer + Scale Factor)
 * registers.
 *
 * Contains the integer-based register definitions for the SunSpec MPPT
 * Extension Model (ID 160). Values must be scaled using their associated scale
 * factor registers to obtain the physical quantities.
 *
 * @note Used in inverter models 101–103.
 */
namespace INT {

/** @brief Total number of registers in the multiple MPPT model (float). */
constexpr uint16_t SIZE = 48;

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

} // namespace INT

} // namespace I160

#endif /* INVERTER_MPPT_REGISTERS_H_ */
