#ifndef METER_H_
#define METER_H_

#include "fronius.h"

class Meter : public Fronius {
public:
  Meter();
  virtual ~Meter();
};

#endif /* METER_H_ */
