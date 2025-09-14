#ifndef METER_H_
#define METER_H_

#include "fronius.h"
#include <expected>
#include <modbus_error.h>

class Meter : public Fronius {
public:
  Meter();
  virtual ~Meter();

  /** Get the meter type

   Uniquely identifies this as a SunSpec Meter Modbus Map

   Float register model:
   211: single phase
   212: split phase
   213: three phase

   Integer and scale factor model:
   201: single phase
   202: split phase
   203: three phase

   @returns pair, meter ID and length of register map
   */
  std::expected<std::pair<int, int>, ModbusError> getMeterType(void);

  /** Return the current register model in use */
  bool isFloatRegisters(void) const;

  /** Get the complete Fronius Meter Register Map from device */
  std::expected<void, ModbusError> getMeterRegisters(void);

  /** AC total current [A] */
  double getAcCurrent(void);

  /** AC phase current

   param res current [A]
   param ph phase A, B or C (default A)
   */
  double getAcCurrentPhase(const char &ph = 'A');

  /** AC voltage phase-to-neutral

   param res voltage [V]
   param ph phase A, B or C (default A)
   */
  double getAcVoltage(const char &ph = 'A');

  /** AC phase-to-phase voltage

   param res voltage [V]
   param ph_pair phase AB, BC or CA
   */
  double getAcVoltagePhaseToPhase(const std::string &ph_pair);

  /** AC power value [W] */
  double getAcPower(void);

  /** AC frequency value [Hz] */
  double getAcFrequency(void);

  /** Apparent power [VA] */
  double getAcPowerApparent(void);

  /** Reactive power [VAr] */
  double getAcPowerReactive(void);

  /** Power factor [%] */
  double getAcPowerFactor(void);

  /** AC energy total exported [kWh] */
  double getAcEnergyExport(void);

  /** AC energy total imported [kWh] */
  double getAcEnergyImport(void);

private:
  /** Current register map model in use */
  bool useFloatRegisters_;

  /** Read meter event flags */
  bool readEventFlags_(uint32_t &flag1, uint32_t &flag2);
};

#endif /* METER_H_ */
