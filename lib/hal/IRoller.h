// IRoller.h - moves exactly one bill per command.
// Owner: Milo. Planned implementation: DcRoller.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Two instances exist:
//  - input roller:  bin -> past the colour sensor -> carousel slot
//  - output roller: carousel slot -> dispensing tray
//
// Contract:
//  - begin() once in setup(). update() every loop(); never blocks.
//  - startFeedOne() starts moving one bill and returns at once. Returns Busy
//    if a feed is already running.
//  - When isBusy() turns false, lastResult() is Ok, Jam (the bill did not
//    clear in time) or Empty (nothing to feed, if the hardware can tell).
//  - stop() cuts the motor immediately.

#pragma once

#include "Types.h"

class IRoller {
 public:
  virtual Result begin() = 0;
  virtual void update() = 0;

  virtual Result startFeedOne() = 0;
  virtual void stop() = 0;

  virtual bool isBusy() const = 0;
  virtual Result lastResult() const = 0;

 protected:
  ~IRoller() = default;
};
