// DebouncedButton.h - IButton for a push button between a pin and GND.
// Owner: Aung (Pass GO button). Uses the internal pull-up, so no resistor.
//
// Arduino-only: lives in lib/drivers so the laptop tests never compile it.

#pragma once

#include <stdint.h>

#include "IButton.h"

class DebouncedButton final : public IButton {
 public:
  explicit DebouncedButton(uint8_t pin);

  Result begin() override;
  void update() override;
  bool wasPressed() override;

 private:
  static constexpr uint32_t kDebounceMs = 50;

  uint8_t pin_;
  bool lastLevel_;           // true = HIGH = released
  uint32_t lastChangeMs_;
  bool pressPending_;
};
