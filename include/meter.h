#ifndef METER_H_
#define METER_H_

#include "fronius.h"
#include <expected>
#include <modbus_error.h>

class Meter : public Fronius {
public:
  Meter();
  virtual ~Meter();

  /** Return the meter initialization state  */
  bool isInitialized() const noexcept { return isInitialized_; }

  /** Get the meter ID and set the register map model

   Must be called after the connection has been established.

   Float register model:
   211: single phase
   212: split phase
   213: three phase

   Integer and scale factor model:
   201: single phase
   202: split phase
   203: three phase

   @returns meter ID
   */
  std::expected<int, ModbusError> detectAndInitializeMeter(void);

  /** Return the current register model in use */
  bool getUseFloatRegisters(void) const;

  /** Return the number of phases */
  int getPhases(void) const;

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

  /** AC active energy exported [kWh]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns energy [kWh]
   */
  double getAcEnergyActiveExport(const Phase ph = Phase::TOTAL) const;

  /** AC active energy imported [kWh]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns energy [kWh]
   */
  double getAcEnergyActiveImport(const Phase ph = Phase::TOTAL) const;

private:
  /** Guard that detectAndInitializeMeter() has been called */
  void checkInitialized() const;

  /** Initialization state*/
  bool isInitialized_{false};

  /** Current register map model in use */
  bool useFloatRegisters_{false};

  /** Store the number of phases */
  int phases_{0};

  /** Read meter event flags */
  std::expected<void, ModbusError> getEventFlags_(uint32_t &flag1,
                                                  uint32_t &flag2);
};

#endif /* METER_H_ */
