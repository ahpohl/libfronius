#ifndef METER_INTSF_REGISTERS_H_
#define METER_INTSF_REGISTERS_H_

#include <cstdint>

constexpr uint16_t M20X_SIZE = 105;

/** Uniquely identifies this as a SunSpec Meter Modbus Map
 *
 * 201: single phase
 * 202: split phase
 * 203: three phase
 */
struct M20X_ID {
  static constexpr uint16_t ADDR = 40070 - 1;
  static constexpr uint16_t NB = 1;
};

/** Length of meter model block
 *
 * @returns 105
 */
struct M20X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC total current [A] */
struct M20X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC phase-A current [A] */
struct M20X_APHA {
  static constexpr uint16_t ADDR = 40073 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC phase-B current [A] */
struct M20X_APHB {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC phase-C current [A] */
struct M20X_APHC {
  static constexpr uint16_t ADDR = 40075 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC current scale factor */
struct M20X_A_SF {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage average phase-to-neutral [V] */
struct M20X_PHV {
  static constexpr uint16_t ADDR = 40077 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-A-to-neutral [V] */
struct M20X_PHVPHA {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-B-to-neutral [V] */
struct M20X_PHVPHB {
  static constexpr uint16_t ADDR = 40079 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-C-to-neutral [V] */
struct M20X_PHVPHC {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage average phase-to-phase [V] */
struct M20X_PPV {
  static constexpr uint16_t ADDR = 40081 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-AB [V] */
struct M20X_PPVPHAB {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-BC [V] */
struct M20X_PPVPHBC {
  static constexpr uint16_t ADDR = 40083 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC voltage phase-CA [V] */
struct M20X_PPVPHCA {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 1;
};

/** Voltage scale factor */
struct M20X_V_SF {
  static constexpr uint16_t ADDR = 40085 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC frequency [Hz] */
struct M20X_HZ {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 1;
};

/** Frequency scale factor */
struct M20X_HZ_SF {
  static constexpr uint16_t ADDR = 40087 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC power total [W] */
struct M20X_W {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC power phase-A [W] */
struct M20X_WPHA {
  static constexpr uint16_t ADDR = 40089 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC power phase-B [W] */
struct M20X_WPHB {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC power phase-C [W] */
struct M20X_WPHC {
  static constexpr uint16_t ADDR = 40091 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power scale factor */
struct M20X_W_SF {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC apparent power total [VA] */
struct M20X_VA {
  static constexpr uint16_t ADDR = 40093 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC apparent power phase-A [VA] */
struct M20X_VAPHA {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC apparent power phase-B [VA] */
struct M20X_VAPHB {
  static constexpr uint16_t ADDR = 40095 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC apparent power phase-C [VA] */
struct M20X_VAPHC {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 1;
};

/** Apparent power scale factor */
struct M20X_VA_SF {
  static constexpr uint16_t ADDR = 40097 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC reactive power total [VAr] */
struct M20X_VAR {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC reactive power phase-A [VAr] */
struct M20X_VARPHA {
  static constexpr uint16_t ADDR = 40099 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC reactive power phase-B [VAr] */
struct M20X_VARPHB {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC reactive power phase-C [VAr] */
struct M20X_VARPHC {
  static constexpr uint16_t ADDR = 40101 - 1;
  static constexpr uint16_t NB = 1;
};

/** Reactive power scale factor */
struct M20X_VAR_SF {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor [%] */
struct M20X_PF {
  static constexpr uint16_t ADDR = 40103 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor phase-A [%] */
struct M20X_PFPHA {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor phase-B [%] */
struct M20X_PFPHB {
  static constexpr uint16_t ADDR = 40105 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor phase-C [%] */
struct M20X_PFPHC {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 1;
};

/** Power factor scale factor */
struct M20X_PF_SF {
  static constexpr uint16_t ADDR = 40107 - 1;
  static constexpr uint16_t NB = 1;
};

/** Total energy exported [Wh] */
struct M20X_TOTWH_EXP {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported phase-A [Wh] */
struct M20X_TOTWH_EXPPHA {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported phase-B [Wh] */
struct M20X_TOTWH_EXPPHB {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported phase-C [Wh] */
struct M20X_TOTWH_EXPPHC {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported [Wh] */
struct M20X_TOTWH_IMP {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported phase-A [Wh] */
struct M20X_TOTWH_IMPPHA {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported phase-B [Wh] */
struct M20X_TOTWH_IMPPHB {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported phase-C [Wh] */
struct M20X_TOTWH_IMPPHC {
  static constexpr uint16_t ADDR = 40122 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy scale factor [Wh] */
struct M20X_TOTWH_SF {
  static constexpr uint16_t ADDR = 40124 - 1;
  static constexpr uint16_t NB = 1;
};

/** Events */
struct M20X_EVT {
  static constexpr uint16_t ADDR = 40175 - 1;
  static constexpr uint16_t NB = 2;
};

/** End block identifier

 @returns 0xFFFF
 */
struct E20X_ID {
  static constexpr uint16_t ADDR = 40177 - 1;
  static constexpr uint16_t NB = 1;
};

/** End block length

 @returns 0
 */
struct E20X_L {
  static constexpr uint16_t ADDR = 40178 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* METER_INTSF_REGISTERS_H_ */
