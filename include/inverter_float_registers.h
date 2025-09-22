#ifndef INVERTER_FLOAT_REGISTERS_H_
#define INVERTER_FLOAT_REGISTERS_H_

#include <cstdint>

constexpr uint16_t I11X_SIZE = 60;

/** Uniquely identifies this as a SunSpec inverter ModBus map (float)
 *
 * @returns 111: single phase
 *          112: split phase
 *          113: three phase
 */
struct I11X_ID {
  static constexpr uint16_t ADDR = 40070 - 1;
  static constexpr uint16_t NB = 1;
};

/** Length of inverter model block.
 *
 * @returns 60
 */
struct I11X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC total current value [A] */
struct I11X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC phase-A current value [A] */
struct I11X_APHA {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC phase-B current value [A] */
struct I11X_APHB {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC phase-C current value [A] */
struct I11X_APHC {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-AB value [V] */
struct I11X_PPVPHAB {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-BC value [V] */
struct I11X_PPVPHBC {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-CA value [V] */
struct I11X_PPVPHCA {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-A-to-neutral value [V] */
struct I11X_PHVPHA {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-B-to-neutral value [V] */
struct I11X_PHVPHB {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-C-to-neutral value [V] */
struct I11X_PHVPHC {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC power value [W] */
struct I11X_W {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC frequency value [Hz] */
struct I11X_HZ {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 2;
};

/** Apparent power [VA] */
struct I11X_VA {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 2;
};

/** Reactive power [VAr] */
struct I11X_VAR {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 2;
};

/** Power factor [%] */
struct I11X_PF {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC lifetime energy production [Wh] */
struct I11X_WH {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 2;
};

/** DC current value [A] */
struct I11X_DCA {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 2;
};

/** DC voltage value [V] */
struct I11X_DCV {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 2;
};

/** DC power value [W] */
struct I11X_DCW {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 2;
};

/** Cabinet temperature [°C] */
struct I11X_TMPCAB {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/** Coolant or heat sink temperature [°C] */
struct I11X_TMPSNK {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/** Transformer temperature [°C] */
struct I11X_TMPTRNS {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/** Other temperature [°C] */
struct I11X_TMPOT {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/** Operating state */
struct I11X_ST {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 1;
};

/** Vendor defined operating state */
struct I11X_STVND {
  static constexpr uint16_t ADDR = 40119 - 1;
  static constexpr uint16_t NB = 1;
};

/** Event flags (bits 0-31) */
struct I11X_EVT1 {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

/** Event flags (bits 32-63) */
struct I11X_EVT2 {
  static constexpr uint16_t ADDR = 40122 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 0-31) */
struct I11X_EVTVND1 {
  static constexpr uint16_t ADDR = 40124 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 32-63) */
struct I11X_EVTVND2 {
  static constexpr uint16_t ADDR = 40126 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 64-95) */
struct I11X_EVTVND3 {
  static constexpr uint16_t ADDR = 40128 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 96-127) */
struct I11X_EVTVND4 {
  static constexpr uint16_t ADDR = 40130 - 1;
  static constexpr uint16_t NB = 2;
};

/** End block identifier

 @returns 0xFFFF
 */
struct E11X_ID {
  static constexpr uint16_t ADDR = 40340 - 1;
  static constexpr uint16_t NB = 1;
};

/** End block length

 @returns 0
 */
struct E11X_L {
  static constexpr uint16_t ADDR = 40341 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* INVERTER_FLOAT_REGISTERS_H_ */
