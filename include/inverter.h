#ifndef INVERTER_H_
#define INVERTER_H_

#include "fronius.h"

class Inverter : public Fronius {
public:
  Inverter();
  virtual ~Inverter();
};

#endif /* INVERTER_H_ */
