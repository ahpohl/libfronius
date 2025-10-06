/**
 * @file inverter_intsf_registers.h
 * @brief Defines SunSpec Inverter (Integer + Scale Factor) Model registers
 * (I10X) for Fronius devices.
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

/** @brief Total number of registers in the inverter model (integer + scale
 * factor). */
constexpr uint16_t I10X_SIZE = 50;

/**
 * @struct I10X_ID
 * @brief SunSpec inverter model identifier (integer + scale factor).
 *
 * @return
 *  - 101: Single-phase
 *  - 102: Split-phase
 *  - 103: Three-phase
 */
struct I10X_ID {
  static constexpr uint16_t ADDR = 40070 - 1; ///< Register start address.
  static constexpr uint16_t NB = 1;           ///< Number of registers.
};

/**
 * @struct I10X_L
 * @brief Length of inverter model block.
 *
 * @return Always returns 50.
 */
struct I10X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_A
 * @brief AC total current.
 * @unit Amperes [A]
 */
struct I10X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_APHA
 * @brief AC current in phase A.
 * @unit Amperes [A]
 */
struct I10X_APHA {
  static constexpr uint16_t ADDR = 40073 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_APHB
 * @brief AC current in phase B.
 * @unit Amperes [A]
 */
struct I10X_APHB {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_APHC
 * @brief AC current in phase C.
 * @unit Amperes [A]
 */
struct I10X_APHC {
  static constexpr uint16_t ADDR = 40075 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_A_SF
 * @brief AC current scale factor.
 */
struct I10X_A_SF {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PPVPHAB
 * @brief AC voltage phase-to-phase AB.
 * @unit Volts [V]
 */
struct I10X_PPVPHAB {
  static constexpr uint16_t ADDR = 40077 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PPVPHBC
 * @brief AC voltage phase-to-phase BC.
 * @unit Volts [V]
 */
struct I10X_PPVPHBC {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PPVPHCA
 * @brief AC voltage phase-to-phase CA.
 * @unit Volts [V]
 */
struct I10X_PPVPHCA {
  static constexpr uint16_t ADDR = 40079 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PHVPHA
 * @brief AC voltage phase A to neutral.
 * @unit Volts [V]
 */
struct I10X_PHVPHA {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PHVPHB
 * @brief AC voltage phase B to neutral.
 * @unit Volts [V]
 */
struct I10X_PHVPHB {
  static constexpr uint16_t ADDR = 40081 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PHVPHC
 * @brief AC voltage phase C to neutral.
 * @unit Volts [V]
 */
struct I10X_PHVPHC {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_V_SF
 * @brief AC voltage scale factor.
 */
struct I10X_V_SF {
  static constexpr uint16_t ADDR = 40083 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_W
 * @brief AC active power.
 * @unit Watts [W]
 */
struct I10X_W {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_W_SF
 * @brief AC active power scale factor.
 */
struct I10X_W_SF {
  static constexpr uint16_t ADDR = 40085 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_HZ
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
struct I10X_HZ {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_HZ_SF
 * @brief AC frequency scale factor.
 */
struct I10X_HZ_SF {
  static constexpr uint16_t ADDR = 40087 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_VA
 * @brief Apparent power.
 * @unit Volt-amperes [VA]
 */
struct I10X_VA {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_VA_SF
 * @brief Apparent power scale factor.
 */
struct I10X_VA_SF {
  static constexpr uint16_t ADDR = 40089 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_VAR
 * @brief Reactive power.
 * @unit Volt-ampere reactive [VAr]
 */
struct I10X_VAR {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_VAR_SF
 * @brief Reactive power scale factor.
 */
struct I10X_VAR_SF {
  static constexpr uint16_t ADDR = 40091 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PF
 * @brief Power factor.
 * @unit Percent [%]
 */
struct I10X_PF {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_PF_SF
 * @brief Power factor scale factor.
 */
struct I10X_PF_SF {
  static constexpr uint16_t ADDR = 40093 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_WH
 * @brief AC lifetime energy production.
 * @unit Watt-hours [Wh]
 */
struct I10X_WH {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I10X_WH_SF
 * @brief AC lifetime energy scale factor.
 */
struct I10X_WH_SF {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 1;
};

// DC measurements and scale factors

/**
 * @struct I10X_DCA
 * @brief DC current.
 * @unit Amperes [A]
 */
struct I10X_DCA {
  static constexpr uint16_t ADDR = 40097 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_DCA_SF
 * @brief DC current scale factor.
 */
struct I10X_DCA_SF {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_DCV
 * @brief DC voltage.
 * @unit Volts [V]
 */
struct I10X_DCV {
  static constexpr uint16_t ADDR = 40099 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_DCV_SF
 * @brief DC voltage scale factor.
 */
struct I10X_DCV_SF {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_DCW
 * @brief DC power.
 * @unit Watts [W]
 */
struct I10X_DCW {
  static constexpr uint16_t ADDR = 40101 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_DCW_SF
 * @brief DC power scale factor.
 */
struct I10X_DCW_SF {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 1;
};

// Temperatures and scale factor

/**
 * @struct I10X_TMPCAB
 * @brief Cabinet temperature.
 * @unit Celsius [°C]
 */
struct I10X_TMPCAB {
  static constexpr uint16_t ADDR = 40103 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_TMPSNK
 * @brief Heat sink/coolant temperature.
 * @unit Celsius [°C]
 */
struct I10X_TMPSNK {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_TMPTRNS
 * @brief Transformer temperature.
 * @unit Celsius [°C]
 */
struct I10X_TMPTRNS {
  static constexpr uint16_t ADDR = 40105 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_TMPOT
 * @brief Other temperature.
 * @unit Celsius [°C]
 */
struct I10X_TMPOT {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_TMP_SF
 * @brief Temperature scale factor.
 */
struct I10X_TMP_SF {
  static constexpr uint16_t ADDR = 40107 - 1;
  static constexpr uint16_t NB = 1;
};

// Operating state and events

/**
 * @struct I10X_ST
 * @brief Inverter operating state.
 */
struct I10X_ST {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_STVND
 * @brief Vendor-defined operating state.
 */
struct I10X_STVND {
  static constexpr uint16_t ADDR = 40109 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I10X_EVT1
 * @brief Event flags bits 0–31.
 */
struct I10X_EVT1 {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I10X_EVT2
 * @brief Event flags bits 32–63.
 */
struct I10X_EVT2 {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I10X_EVTVND1
 * @brief Vendor-defined event flags bits 0–31.
 */
struct I10X_EVTVND1 {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I10X_EVTVND2
 * @brief Vendor-defined event flags bits 32–63.
 */
struct I10X_EVTVND2 {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I10X_EVTVND3
 * @brief Vendor-defined event flags bits 64–95.
 */
struct I10X_EVTVND3 {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I10X_EVTVND4
 * @brief Vendor-defined event flags bits 96–127.
 */
struct I10X_EVTVND4 {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct E10X_ID
 * @brief End-of-block identifier.
 *
 * @return Always 0xFFFF.
 */
struct E10X_ID {
  static constexpr uint16_t ADDR = 40330 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct E10X_L
 * @brief End-of-block length.
 *
 * @return Always 0.
 */
struct E10X_L {
  static constexpr uint16_t ADDR = 40331 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* INVERTER_INTSF_REGISTERS_H_ */
