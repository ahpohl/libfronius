#ifndef INVERTER_H_
#define INVERTER_H_

#include "fronius.h"

class Inverter : public Fronius {
public:
  Inverter(const ModbusConfig &cfg);
  virtual ~Inverter();

  /** Return the meter initialization state  */
  bool isInitialized() const noexcept { return isInitialized_; }

  /** Return the current register model in use */
  bool getUseFloatRegisters(void) const;

  /** Return the number of phases */
  int getPhases(void) const;

  /** Get the inverter ID and set the register map model

   Must be called after the connection has been established.

   Float register model:
   111: single phase
   112: split phase
   113: three phase

   Integer and scale factor model:
   101: single phase
   102: split phase
   103: three phase

   @returns inverter ID
   */
  std::expected<int, ModbusError> detectAndInitialize(void);

  /** Fetch the complete Fronius Inverter Register Map from device */
  std::expected<void, ModbusError> fetchInverterRegisters(void);

  /** AC current [A]

   @param ph phase name [TOTAL, PHA, PHB or PHC]
   @returns current [A]
   */
  double getAcCurrent(const Phase ph = Phase::TOTAL) const;

  /** AC voltage [V]

   @param ph phase name [PHA, PHB, PHC, PHAB, PHBC or PHCA]
   @returns voltage [V]
   */
  double getAcVoltage(const Phase ph = Phase::PHA) const;

  /** AC active power [W] */
  double getAcPowerActive(void) const;

  /** AC apparent power [VA] */
  double getAcPowerApparent(void) const;

  /** AC reactive power [VAr] */
  double getAcPowerReactive(void) const;

  /** AC frequency value [Hz] */
  double getAcFrequency(void) const;

  /** Power factor [%] */
  double getAcPowerFactor(void) const;

  /** AC lifetime energy production [Wh] */
  double getAcEnergy(void) const;

  /** DC current [A] */
  double getDcCurrent(void) const;

  /** DC voltage [V] */
  double getDcVoltage(void) const;

  /** DC power [W] */
  double getDcPower(void) const;

private:
  /** Guard that detectAndInitialize() has been called */
  void checkInitialized() const;

  /** Initialization state*/
  bool isInitialized_{false};

  /** Current register map model in use */
  bool useFloatRegisters_{false};

  /** Store the number of phases */
  int phases_{0};

  /** Read event flags */
  std::expected<void, ModbusError> getEventFlags_(uint32_t &flag1,
                                                  uint32_t &flag2);
};

#endif /* INVERTER_H_ */
