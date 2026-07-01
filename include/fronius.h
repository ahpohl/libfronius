/**
 * @file fronius.h
 * @brief Umbrella header for the libfronius public object API.
 *
 * @details
 * Includes everything needed to configure a shared Modbus bus, attach
 * `Inverter`/`Meter` devices to it, and read their high-level API.
 *
 * Consumers who need raw SunSpec/proprietary register access instead
 * (e.g. to build a virtual Modbus TCP slave mapping) should include
 * `<fronius/registers.h>` in addition to, or instead of, this header.
 */

#ifndef FRONIUS_H_
#define FRONIUS_H_

#include "fronius_bus.h"
#include "fronius_device.h"
#include "fronius_types.h"
#include "inverter.h"
#include "meter.h"
#include "modbus_config.h"
#include "modbus_error.h"

#endif // FRONIUS_H_
