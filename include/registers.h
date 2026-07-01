/**
 * @file registers.h
 * @brief Umbrella header for raw Modbus register access.
 *
 * @details
 * Includes the SunSpec common block and per-device register maps
 * together with the `ModbusUtils` helpers used to pack and unpack raw
 * register values (e.g. when building a virtual Modbus TCP slave
 * mapping by hand).
 *
 * Most consumers using the high-level `Inverter`/`Meter` API do not
 * need this header; include `<fronius/fronius.h>` instead.
 */

#ifndef FRONIUS_REGISTERS_H_
#define FRONIUS_REGISTERS_H_

#include "common_registers.h"
#include "inverter_registers.h"
#include "meter_registers.h"
#include "modbus_utils.h"
#include "register_base.h"

#endif // FRONIUS_REGISTERS_H_
