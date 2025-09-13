#ifndef METER_H_
#define METER_H_

#include "fronius.h"
#include <expected>
#include <modbus_error.h>

class Meter : public Fronius {
public:
  Meter();
  virtual ~Meter();

  /** Test if the smart meter has SunSpec registers */
  std::expected<bool, ModbusError> isSunSpecMeter(void);

  /** Get the register model */
  bool getFloatRegisterModel(void) const;

  /** Read the complete Fronius Meter Register Map */
  std::expected<void, ModbusError> readMeterRegisters(void);

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
