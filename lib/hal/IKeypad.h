// IKeypad.h - the 4x4 keypad, read through the PCF8574 I2C expander.
// Owner: Minh.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Contract:
//  - begin() once in setup(). update() every loop(); never blocks.
//  - getKey() returns each key press exactly once, already debounced, or
//    kNoKey when there is no new press.
//  - Keys: '0'-'9', 'A'-'D', '*', '#'.
//    UI convention: '#' = enter, '*' = clear / back.

#pragma once

#include "Types.h"

constexpr char kNoKey = '\0';

class IKeypad {
 public:
  virtual Result begin() = 0;
  virtual void update() = 0;

  virtual char getKey() = 0;

 protected:
  ~IKeypad() = default;
};
