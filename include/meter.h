#ifndef METER_H_
#define METER_H_

#include "fronius.h"
#include "modbus_config.h"
#include "modbus_error.h"
#include <expected>
#include <modbus/modbus.h>

class Meter : public Fronius {
public:
  explicit Meter(const ModbusConfig &cfg);
  virtual ~Meter();

  /** Validate the meter registers */
  std::expected<void, ModbusError> validateDevice();

  /** Return the current register model in use */
  bool getUseFloatRegisters(void) const;

  /** Return the number of phases */
  int getPhases(void) const;

  /** Return the meter ID */
  int getId(void) const;

  /** Detect and initialize the meter

   Must be called after the connection has been established.
   Sets the register model from the meter ID

   Float register model:
   211: single phase
   212: split phase
   213: three phase

   Integer and scale factor model:
   201: single phase
   202: split phase
   203: three phase
   */
  std::expected<void, ModbusError> detectAndInitialize(void);

  /** Fetch the complete Fronius Meter Register Map from device */
  std::expected<void, ModbusError> fetchMeterRegisters(void);

  /** AC current [A]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns current [A]
   */
  double getAcCurrent(const Phase ph = Phase::TOTAL) const;

  /** AC voltage [V]

   @param ph phase name [AVERAGE, PHA, PHB or PHC]
   @returns voltage [V]
   */
  double getAcVoltage(const Phase ph = Phase::AVERAGE) const;

  /** AC active power [W]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns voltage [V]
   */
  double getAcPowerActive(const Phase ph = Phase::TOTAL) const;

  /** AC frequency value [Hz] */
  double getAcFrequency(void) const;

  /** AC apparent power [VA]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns power [VA]
   */
  double getAcPowerApparent(const Phase ph = Phase::TOTAL) const;

  /** AC reactive power [VAr]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns power [VAr]
   */
  double getAcPowerReactive(const Phase ph = Phase::TOTAL) const;

  /** Power factor [%]

   @param ph phase name [AVERAGE, PHA, PHB or PHC]
   @returns power factor
  */
  double getAcPowerFactor(const Phase ph = Phase::AVERAGE) const;

  /** AC active energy exported [Wh]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns energy [Wh]
   */
  double getAcEnergyActiveExport(const Phase ph = Phase::TOTAL) const;

  /** AC active energy imported [Wh]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns energy [Wh]
   */
  double getAcEnergyActiveImport(const Phase ph = Phase::TOTAL) const;

private:
  /** Store connection and register validity states */
  bool connectedAndValid_{false};

  /** Current register map model in use */
  bool useFloatRegisters_{false};

  /** Store the meter ID */
  int id_{0};

  /** Read event flags */
  std::expected<void, ModbusError> getEventFlags_(uint32_t &flag1,
                                                  uint32_t &flag2);
};

#endif /* METER_H_ */
