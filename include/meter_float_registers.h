#ifndef METER_FLOAT_REGISTERS_H_
#define METER_FLOAT_REGISTERS_H_

#include <cstdint>

constexpr uint16_t M21X_SIZE = 124;

/** Uniquely identifies this as a SunSpec Meter Modbus Map (float)
 *
 * 211: single phase
 * 212: split phase
 * 213: three phase
 */
struct M21X_ID {
  static constexpr uint16_t ADDR = 40070 - 1;
  static constexpr uint16_t NB = 1;
};

/** Length of meter model block
 *
 * @returns 124
 */
struct M21X_L {
  static constexpr uint16_t ADDR = 40071 - 1;
  static constexpr uint16_t NB = 1;
};

/** AC total current [A] */
struct M21X_A {
  static constexpr uint16_t ADDR = 40072 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC phase-A current [A] */
struct M21X_APHA {
  static constexpr uint16_t ADDR = 40074 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC phase-B current [A] */
struct M21X_APHB {
  static constexpr uint16_t ADDR = 40076 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC phase-C current [A] */
struct M21X_APHC {
  static constexpr uint16_t ADDR = 40078 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage average phase-to-neutral [V] */
struct M21X_PHV {
  static constexpr uint16_t ADDR = 40080 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-A-to-neutral [V] */
struct M21X_PHVPHA {
  static constexpr uint16_t ADDR = 40082 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-B-to-neutral [V] */
struct M21X_PHVPHB {
  static constexpr uint16_t ADDR = 40084 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-C-to-neutral [V] */
struct M21X_PHVPHC {
  static constexpr uint16_t ADDR = 40086 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage average phase-to-phase [V] */
struct M21X_PPV {
  static constexpr uint16_t ADDR = 40088 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-AB [V] */
struct M21X_PPVPHAB {
  static constexpr uint16_t ADDR = 40090 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-BC [V] */
struct M21X_PPVPHBC {
  static constexpr uint16_t ADDR = 40092 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC voltage phase-CA [V] */
struct M21X_PPVPHCA {
  static constexpr uint16_t ADDR = 40094 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC frequency [Hz] */
struct M21X_HZ {
  static constexpr uint16_t ADDR = 40096 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC power total [W] */
struct M21X_W {
  static constexpr uint16_t ADDR = 40098 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC power phase-A [W] */
struct M21X_WPHA {
  static constexpr uint16_t ADDR = 40100 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC power phase-B [W] */
struct M21X_WPHB {
  static constexpr uint16_t ADDR = 40102 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC power phase-C [W] */
struct M21X_WPHC {
  static constexpr uint16_t ADDR = 40104 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC apparent power total [VA] */
struct M21X_VA {
  static constexpr uint16_t ADDR = 40106 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC apparent power phase-A [VA] */
struct M21X_VAPHA {
  static constexpr uint16_t ADDR = 40108 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC apparent power phase-B [VA] */
struct M21X_VAPHB {
  static constexpr uint16_t ADDR = 40110 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC apparent power phase-C [VA] */
struct M21X_VAPHC {
  static constexpr uint16_t ADDR = 40112 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC reactive power total [VAr] */
struct M21X_VAR {
  static constexpr uint16_t ADDR = 40114 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC reactive power phase-A [VAr] */
struct M21X_VARPHA {
  static constexpr uint16_t ADDR = 40116 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC reactive power phase-B [VAr] */
struct M21X_VARPHB {
  static constexpr uint16_t ADDR = 40118 - 1;
  static constexpr uint16_t NB = 2;
};

/** AC reactive power phase-C [VAr] */
struct M21X_VARPHC {
  static constexpr uint16_t ADDR = 40120 - 1;
  static constexpr uint16_t NB = 2;
};

/** Power factor [%] */
struct M21X_PF {
  static constexpr uint16_t ADDR = 40122 - 1;
  static constexpr uint16_t NB = 2;
};

/** Power factor phase-A [%] */
struct M21X_PFPHA {
  static constexpr uint16_t ADDR = 40124 - 1;
  static constexpr uint16_t NB = 2;
};

/** Power factor phase-B [%] */
struct M21X_PFPHB {
  static constexpr uint16_t ADDR = 40126 - 1;
  static constexpr uint16_t NB = 2;
};

/** Power factor phase-C [%] */
struct M21X_PFPHC {
  static constexpr uint16_t ADDR = 40128 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported [Wh] */
struct M21X_TOTWH_EXP {
  static constexpr uint16_t ADDR = 40130 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported phase-A [Wh] */
struct M21X_TOTWH_EXPPHA {
  static constexpr uint16_t ADDR = 40132 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported phase-B [Wh] */
struct M21X_TOTWH_EXPPHB {
  static constexpr uint16_t ADDR = 40134 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy exported phase-C [Wh] */
struct M21X_TOTWH_EXPPHC {
  static constexpr uint16_t ADDR = 40136 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported [Wh] */
struct M21X_TOTWH_IMP {
  static constexpr uint16_t ADDR = 40138 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported phase-A [Wh] */
struct M21X_TOTWH_IMPPHA {
  static constexpr uint16_t ADDR = 40140 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported phase-B [Wh] */
struct M21X_TOTWH_IMPPHB {
  static constexpr uint16_t ADDR = 40142 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total energy imported phase-C [Wh] */
struct M21X_TOTWH_IMPPHC {
  static constexpr uint16_t ADDR = 40144 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy exported [VAh] */
struct M21X_TOTVAH_EXP {
  static constexpr uint16_t ADDR = 40146 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy exported phase-A [VAh] */
struct M21X_TOTVAH_EXPPHA {
  static constexpr uint16_t ADDR = 40148 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy exported phase-B [VAh] */
struct M21X_TOTVAH_EXPPHB {
  static constexpr uint16_t ADDR = 40150 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy exported phase-C [VAh] */
struct M21X_TOTVAH_EXPPHC {
  static constexpr uint16_t ADDR = 40152 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy imported [VAh] */
struct M21X_TOTVAH_IMP {
  static constexpr uint16_t ADDR = 40154 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy imported phase-A [VAh] */
struct M21X_TOTVAH_IMPPHA {
  static constexpr uint16_t ADDR = 40156 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy imported phase-B [VAh] */
struct M21X_TOTVAH_IMPPHB {
  static constexpr uint16_t ADDR = 40158 - 1;
  static constexpr uint16_t NB = 2;
};

/** Total apparent energy imported phase-C [VAh] */
struct M21X_TOTVAH_IMPPHC {
  static constexpr uint16_t ADDR = 40160 - 1;
  static constexpr uint16_t NB = 2;
};

/** Events */
struct M21X_EVT {
  static constexpr uint16_t ADDR = 40194 - 1;
  static constexpr uint16_t NB = 2;
};

/** End block identifier

 @returns 0xFFFF
 */
struct E21X_ID {
  static constexpr uint16_t ADDR = 40196 - 1;
  static constexpr uint16_t NB = 1;
};

/** End block length

 @returns 0
 */
struct E21X_L {
  static constexpr uint16_t ADDR = 40197 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* METER_FLOAT_REGISTERS_H_ */
