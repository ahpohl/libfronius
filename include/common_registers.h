#ifndef COMMON_REGISTERS_H_
#define COMMON_REGISTERS_H_

#include <cstdint>

/** Length of Common Model block */
constexpr int CommonMapSize = 65;

/** Uniquely identifies this as a SunSpec ModBus map.

 @returns 0x53756e53 ('SunS')
 */
struct C001_SID {
  uint32_t res = 0;
  const uint16_t reg = 40001 - 1;
  const uint16_t nb = 2;
};
inline C001_SID C001_SID;

/** Uniquely identifies this as a SunSpec Common Model block

 @returns 1
 */
struct C001_ID {
  uint16_t res = 0;
  const uint16_t reg = 40003 - 1;
  const uint16_t nb = 1;
};
inline C001_ID C001_ID;

/** Length of Common Model block.

 @returns 65
 */
struct C001_L {
  uint16_t res = 0;
  const uint16_t reg = 40004 - 1;
  const uint16_t nb = 1;
};
inline C001_L C001_L;

/** Manufacturer

 @returns: Fronius
 */
struct C001_Mn {
  char str[32] = {0};
  const uint16_t reg = 40005 - 1;
  const uint16_t nb = 16;
};
inline C001_Mn C001_Mn;

/** Device model

 @returns i.e. IG+150V [3p]
 */
struct C001_Md {
  char str[32] = {0};
  const uint16_t reg = 40021 - 1;
  const uint16_t nb = 16;
};
inline C001_Md C001_Md;

/** SW version of installed option

 @returns i.e. Firmware version of Datamanager
 */
struct C001_Opt {
  char str[16] = {0};
  const uint16_t reg = 40037 - 1;
  const uint16_t nb = 8;
};
inline C001_Opt C001_Opt;

/** SW version of main device

 @returns i.e. Firmware version of inverter, meter, battery etc.
 */
struct C001_Vr {
  char str[16] = {0};
  const uint16_t reg = 40045 - 1;
  const uint16_t nb = 8;
};
inline C001_Vr C001_Vr;

/** Serial number of the inverter

 Please note, that the inverter serial number is not supported on all devices
 and also depends on the internal inverter controller‘s production date and its
 software version.

 If the inverter serial number is not supported, then two fallbacks are
 implemented that are still unique but may not match the inverter serial number
 printed on the nameplate:

 1: Serial of inverter controller (PMC) if supported
 2: Unique ID (UID) of inverter controller

 For SYMOHYBRID inverters the inverter serial number is not supported and this
 register always contains one of the two implemented fallbacks.

 Please note further that due to startup timing issues or synchronization faults
 this field may change its value during operation from one option to another,
 e.g. at boot time only UID is available and after some time device information
 is synchronized then this field changes from UID to PMC or inverter serial
 number.
 */
struct C001_SN {
  char str[32] = {0};
  const uint16_t reg = 40053 - 1;
  const uint16_t nb = 16;
};
inline C001_SN C001_SN;

/** Read ModBus device address

 @returns 1-247
 */
struct C001_DA {
  uint16_t res = 0;
  const uint16_t reg = 40069 - 1;
  const uint16_t nb = 1;
};
inline C001_DA C001_DA;

#endif /* COMMON_REGISTERS_H_ */
