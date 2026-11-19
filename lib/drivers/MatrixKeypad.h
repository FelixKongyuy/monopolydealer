// MatrixKeypad.h - IKeypad for a 4x4 membrane keypad wired to 8 Uno pins.
// Owner: Minh. Uses the "Keypad" library by Mark Stanley and Alexander Brevig.
//
// Arduino-only: lives in lib/drivers so the laptop tests never compile it.

#pragma once

#include <Keypad.h>
#include <stdint.h>

#include "IKeypad.h"

class MatrixKeypad final : public IKeypad {
 public:
  static constexpr uint8_t kRows = 4;
  static constexpr uint8_t kCols = 4;

  // Pin arrays are copied, so the caller's arrays don't need to outlive this.
  MatrixKeypad(const uint8_t (&rowPins)[kRows], const uint8_t (&colPins)[kCols]);

  Result begin() override;
  void update() override;
  char getKey() override;

 private:
  // Declared before keypad_ because keypad_ keeps pointers to them.
  uint8_t rowPins_[kRows];
  uint8_t colPins_[kCols];
  Keypad keypad_;
};
