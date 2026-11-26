// ICarousel.h - turns the carousel so a slot lines up with a roller.
// Owner: Milo. Planned implementation: StepperCarousel
// (28BYJ-48 stepper + ULN2003 driver + home switch).
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Contract:
//  - Slots are numbered 0 to kSlotCount - 1. Slot n holds Denom n (Money.h).
//  - begin() once in setup(). update() every loop(); never blocks.
//  - startHoming() turns until the home switch triggers and calls that
//    position slot 0. It must finish before moveToSlot() is allowed.
//  - moveToSlot() starts a move and returns at once. Poll isBusy().
//    Station says which roller the slot must line up with; if both rollers
//    sit at the same angle, the implementation ignores it.
//  - Commands return Busy while a move runs, NotReady before homing
//    finishes, InvalidArgument for a bad slot.
//  - lastResult() is Ok after a successful move, or the error that stopped
//    it (e.g. HardwareFault if homing never found the switch).
//  - stop() halts the motor immediately and clears isHomed().

#pragma once

#include <stdint.h>

#include "Money.h"
#include "Types.h"

enum class Station : uint8_t {
  Input,   // where the input roller drops a sorted bill in
  Output,  // where the output roller pulls a bill out
};

class ICarousel {
 public:
  virtual Result begin() = 0;
  virtual void update() = 0;

  virtual Result startHoming() = 0;
  virtual Result moveToSlot(uint8_t slot, Station station) = 0;
  virtual void stop() = 0;

  virtual bool isBusy() const = 0;
  virtual bool isHomed() const = 0;
  virtual uint8_t currentSlot() const = 0;
  virtual Result lastResult() const = 0;

 protected:
  ~ICarousel() = default;
};
