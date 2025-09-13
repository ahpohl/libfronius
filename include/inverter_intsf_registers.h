#ifndef INVERTER_INTSF_REGISTERS_H_
#define INVERTER_INTSF_REGISTERS_H_

#include <cstdint>

constexpr uint16_t I10X_SIZE = 50;

/** Uniquely identifies this as a SunSpec inverter ModBus map.
 *
 * @returns 101: single phase
 *          102: split phase
 *          103: three phase
 */
struct I10X_ID {
  static constexpr uint16_t ADDR = 40070 - 1;
  static constexpr uint16_t NB = 1;
};

/** Length of inverter model block.
 *
 * @returns 50
 */
struct I10X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC total current value [A] */
struct I10X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC phase-A current value [A] */
struct I10X_APHA {
  static constexpr uint16_t ADDR = 40073 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC phase-B current value [A] */
struct I10X_APHB {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC phase-C current value [A] */
struct I10X_APHC {
  static constexpr uint16_t ADDR = 40075 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC current scale factor */
struct I10X_A_SF {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-AB value [V] */
struct I10X_PPVPH_AB {
  static constexpr uint16_t ADDR = 40077 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-BC value [V] */
struct I10X_PPVPH_BC {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-CA value [V] */
struct I10X_PPVPH_CA {
  static constexpr uint16_t ADDR = 40079 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-A-to-neutral value [V] */
struct I10X_PHVPH_A {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-B-to-neutral value [V] */
struct I10X_PHVPH_B {
  static constexpr uint16_t ADDR = 40081 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-C-to-neutral value [V] */
struct I10X_PHVPH_C {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage scale factor */
struct I10X_V_SF {
  static constexpr uint16_t ADDR = 40083 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC power value [W] */
struct I10X_W {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC power scale factor */
struct I10X_W_SF {
  static constexpr uint16_t ADDR = 40085 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC frequency value [Hz] */
struct I10X_HZ {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC frequency scale factor */
struct I10X_HZ_SF {
  static constexpr uint16_t ADDR = 40087 - 1;
  static constexpr uint16_t NB = 1;
};

/** Apparent power [VA] */
struct I10X_VA {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 1;
};

/** Apparent power scale factor */
struct I10X_VA_SF {
  static constexpr uint16_t ADDR = 40089 - 1;
  static constexpr uint16_t NB = 1;
};

/** Reactive power [VAr] */
struct I10X_VAR {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 1;
};

/** Reactive power scale factor */
struct I10X_VAR_SF {
  static constexpr uint16_t ADDR = 40091 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor [%] */
struct I10X_PF {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor scale factor */
struct I10X_PF_SF {
  static constexpr uint16_t ADDR = 40093 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC lifetime energy production [Wh] */
struct I10X_WH {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC lifetime energy production scale factor */
struct I10X_WH_SF {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 1;
};

/** DC current value [A] */
struct I10X_DCA {
  static constexpr uint16_t ADDR = 40097 - 1;
  static constexpr uint16_t NB = 1;
};

/** DC current scale factor */
struct I10X_DCA_SF {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 1;
};

/** DC voltage value [V] */
struct I10X_DCV {
  static constexpr uint16_t ADDR = 40099 - 1;
  static constexpr uint16_t NB = 1;
};

/** DC voltage scale factor */
struct I10X_DCV_SF {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 1;
};

/** DC power value [W] */
struct I10X_DCW {
  static constexpr uint16_t ADDR = 40101 - 1;
  static constexpr uint16_t NB = 1;
};

/** DC power value scale factor */
struct I10X_DCW_SF {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 1;
};

/** Cabinet temperature [°C] */
struct I10X_TMP_CAB {
  static constexpr uint16_t ADDR = 40103 - 1;
  static constexpr uint16_t NB = 1;
};

/** Coolant or heat sink temperature [°C] */
struct I10X_TMP_SNK {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 1;
};

/** Transformer temperature [°C] */
struct I10X_TMP_TRNS {
  static constexpr uint16_t ADDR = 40105 - 1;
  static constexpr uint16_t NB = 1;
};

/** Other temperature [°C] */
struct I10X_TMP_OT {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 1;
};

/** Temperature scale factor */
struct I10X_TMP_SF {
  static constexpr uint16_t ADDR = 40107 - 1;
  static constexpr uint16_t NB = 1;
};

/** Operating state */
struct I10X_ST {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 1;
};

/** Vendor defined operating state */
struct I10X_ST_VND {
  static constexpr uint16_t ADDR = 40109 - 1;
  static constexpr uint16_t NB = 1;
};

/** Event flags (bits 0-31) */
struct I10X_EVT1 {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/** Event flags (bits 32-63) */
struct I10X_EVT2 {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 0-31) */
struct I10X_EVT_VND1 {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 32-63) */
struct I10X_EVT_VND2 {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 64-95) */
struct I10X_EVT_VND3 {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 2;
};

/** Vendor defined event flags (bits 96-127) */
struct I10X_EVT_VND4 {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

#endif /* INVERTER_INTSF_REGISTERS_H_ */
