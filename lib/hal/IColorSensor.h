// IColorSensor.h - reads the colour of the bill under the sensor.
// Owner: Osy. Planned implementation: Apds9960Sensor.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Contract:
//  - begin() once in setup(). Returns HardwareFault if the sensor does not
//    answer on I2C.
//  - update() every loop(). Never blocks.
//  - requestReading() turns the fixed LED on and starts one colour
//    measurement. Returns Busy if one is already running.
//  - readingReady() becomes true when the measurement is done; lastReading()
//    then returns it. Turning raw readings into a Denom is BillClassifier's
//    job, not this class's.
//  - billPresent() is true while a bill sits over the sensor (proximity).

#pragma once

#include "Types.h"

class IColorSensor {
 public:
  virtual Result begin() = 0;
  virtual void update() = 0;

  virtual Result requestReading() = 0;
  virtual bool readingReady() const = 0;
  virtual ColorReading lastReading() const = 0;

  virtual bool billPresent() const = 0;

 protected:
  ~IColorSensor() = default;
};
