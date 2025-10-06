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

/** @brief Total number of registers in the inverter model (float). */
constexpr uint16_t I11X_SIZE = 60;

/**
 * @struct I11X_ID
 * @brief SunSpec inverter model identifier (float).
 *
 * @details Identifies the type of inverter register map. The value indicates
 * whether the device is single-, split-, or three-phase.
 *
 * @return
 *  - 111: Single-phase
 *  - 112: Split-phase
 *  - 113: Three-phase
 */
struct I11X_ID {
  static constexpr uint16_t ADDR = 40070 - 1; ///< Register start address.
  static constexpr uint16_t NB = 1;           ///< Number of registers.
};

/**
 * @struct I11X_L
 * @brief Length of inverter model block.
 *
 * @return Always returns 60.
 */
struct I11X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I11X_A
 * @brief AC total current.
 * @unit Amperes [A]
 */
struct I11X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_APHA
 * @brief AC current in phase A.
 * @unit Amperes [A]
 */
struct I11X_APHA {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_APHB
 * @brief AC current in phase B.
 * @unit Amperes [A]
 */
struct I11X_APHB {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_APHC
 * @brief AC current in phase C.
 * @unit Amperes [A]
 */
struct I11X_APHC {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PHVPHA
 * @brief AC voltage phase-A to neutral.
 * @unit Volts [V]
 */
struct I11X_PHVPHA {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PHVPHB
 * @brief AC voltage phase-B to neutral.
 * @unit Volts [V]
 */
struct I11X_PHVPHB {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PHVPHC
 * @brief AC voltage phase-C to neutral.
 * @unit Volts [V]
 */
struct I11X_PHVPHC {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PPVPHAB
 * @brief AC voltage phase-to-phase AB.
 * @unit Volts [V]
 */
struct I11X_PPVPHAB {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PPVPHBC
 * @brief AC voltage phase-to-phase BC.
 * @unit Volts [V]
 */
struct I11X_PPVPHBC {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PPVPHCA
 * @brief AC voltage phase-to-phase CA.
 * @unit Volts [V]
 */
struct I11X_PPVPHCA {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_W
 * @brief AC total power.
 * @unit Watts [W]
 */
struct I11X_W {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_HZ
 * @brief AC frequency.
 * @unit Hertz [Hz]
 */
struct I11X_HZ {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_VA
 * @brief Apparent power.
 * @unit Volt-amperes [VA]
 */
struct I11X_VA {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_VAR
 * @brief Reactive power.
 * @unit Volt-amperes reactive [VAr]
 */
struct I11X_VAR {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_PF
 * @brief Power factor.
 * @unit Percent [%]
 */
struct I11X_PF {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_WH
 * @brief Lifetime energy production.
 * @unit Watt-hours [Wh]
 */
struct I11X_WH {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_DCA
 * @brief DC current.
 * @unit Amperes [A]
 */
struct I11X_DCA {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_DCV
 * @brief DC voltage.
 * @unit Volts [V]
 */
struct I11X_DCV {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_DCW
 * @brief DC power.
 * @unit Watts [W]
 */
struct I11X_DCW {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_TMPCAB
 * @brief Cabinet temperature.
 * @unit Celsius [°C]
 */
struct I11X_TMPCAB {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_TMPSNK
 * @brief Heat sink or coolant temperature.
 * @unit Celsius [°C]
 */
struct I11X_TMPSNK {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_TMPTRNS
 * @brief Transformer temperature.
 * @unit Celsius [°C]
 */
struct I11X_TMPTRNS {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_TMPOT
 * @brief Other temperature (unspecified).
 * @unit Celsius [°C]
 */
struct I11X_TMPOT {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_ST
 * @brief Inverter operating state.
 */
struct I11X_ST {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I11X_STVND
 * @brief Vendor-defined operating state.
 */
struct I11X_STVND {
  static constexpr uint16_t ADDR = 40119 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct I11X_EVT1
 * @brief Event flags (bits 0–31).
 */
struct I11X_EVT1 {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_EVT2
 * @brief Event flags (bits 32–63).
 */
struct I11X_EVT2 {
  static constexpr uint16_t ADDR = 40122 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_EVTVND1
 * @brief Vendor-defined event flags (bits 0–31).
 */
struct I11X_EVTVND1 {
  static constexpr uint16_t ADDR = 40124 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_EVTVND2
 * @brief Vendor-defined event flags (bits 32–63).
 */
struct I11X_EVTVND2 {
  static constexpr uint16_t ADDR = 40126 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_EVTVND3
 * @brief Vendor-defined event flags (bits 64–95).
 */
struct I11X_EVTVND3 {
  static constexpr uint16_t ADDR = 40128 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct I11X_EVTVND4
 * @brief Vendor-defined event flags (bits 96–127).
 */
struct I11X_EVTVND4 {
  static constexpr uint16_t ADDR = 40130 - 1;
  static constexpr uint16_t NB = 2;
};

/**
 * @struct E11X_ID
 * @brief End-of-block identifier.
 *
 * @return Always returns 0xFFFF.
 */
struct E11X_ID {
  static constexpr uint16_t ADDR = 40340 - 1;
  static constexpr uint16_t NB = 1;
};

/**
 * @struct E11X_L
 * @brief End-of-block length field.
 *
 * @return Always returns 0.
 */
struct E11X_L {
  static constexpr uint16_t ADDR = 40341 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* INVERTER_FLOAT_REGISTERS_H_ */
