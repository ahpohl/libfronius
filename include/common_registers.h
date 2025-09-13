#ifndef COMMON_REGISTERS_H_
#define COMMON_REGISTERS_H_

#include <cstdint>

/** Length of Common Model block */
constexpr uint16_t C001_SIZE = 65;

/** Uniquely identifies this as a SunSpec ModBus map.
 *
 * @returns 0x53756e53 ('SunS')
 */
struct C001_SID {
  static constexpr uint16_t ADDR = 40001 - 1;
  static constexpr uint16_t NB = 2;
};

/** Uniquely identifies this as a SunSpec Common Model block
 *
 * @returns 1
 */
struct C001_ID {
  static constexpr uint16_t ADDR = 40003 - 1;
  static constexpr uint16_t NB = 1;
};

/** Length of Common Model block.
 *
 * @returns 65
 */
struct C001_L {
  static constexpr uint16_t ADDR = 40004 - 1;
  static constexpr uint16_t NB = 1;
};

/** Manufacturer
 *
 * @returns: Fronius
 */
struct C001_MN {
  static constexpr uint16_t ADDR = 40005 - 1;
  static constexpr uint16_t NB = 16;
};

/** Device model
 *
 * @returns i.e. IG+150V [3p]
 */
struct C001_MD {
  static constexpr uint16_t ADDR = 40021 - 1;
  static constexpr uint16_t NB = 16;
};

/** SW version of installed option
 *
 * @returns i.e. Firmware version of Datamanager
 */
struct C001_OPT {
  static constexpr uint16_t ADDR = 40037 - 1;
  static constexpr uint16_t NB = 8;
};

/** SW version of main device
 *
 * @returns i.e. Firmware version of inverter, meter, battery etc.
 */
struct C001_VR {
  static constexpr uint16_t ADDR = 40045 - 1;
  static constexpr uint16_t NB = 8;
};

/** Serial number of the inverter
 *
 * Please note, that the inverter serial number is not supported on all devices
 * and also depends on the internal inverter controller‘s production date and
 * its software version.
 *
 * If the inverter serial number is not supported, then two fallbacks are
 * implemented that are still unique but may not match the inverter serial
 * number printed on the nameplate:
 *
 * 1: Serial of inverter controller (PMC) if supported
 * 2: Unique ID (UID) of inverter controller
 *
 * For SYMOHYBRID inverters the inverter serial number is not supported and this
 * addrister always contains one of the two implemented fallbacks.
 *
 * Please note further that due to startup timing issues or synchronization
 * faults this field may change its value during operation from one option to
 * another, e.g. at boot time only UID is available and after some time device
 * information is synchronized then this field changes from UID to PMC or
 * inverter serial number.
 */
struct C001_SN {
  static constexpr uint16_t ADDR = 40053 - 1;
  static constexpr uint16_t NB = 16;
};

/** Read ModBus device address
 *
 * @returns 1-247
 */
struct C001_DA {
  static constexpr uint16_t ADDR = 40069 - 1;
  static constexpr uint16_t NB = 1;
};

#endif /* COMMON_REGISTERS_H_ */
