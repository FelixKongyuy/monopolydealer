// LcdDisplay.h - IDisplay for a parallel LCD1602 (Elegoo kit) in 4-bit mode.
// Owner: Minh.
//
// Arduino-only: lives in lib/drivers so the laptop tests never compile it.

#pragma once

#include <LiquidCrystal.h>
#include <stdint.h>

#include "IDisplay.h"

class LcdDisplay final : public IDisplay {
 public:
  LcdDisplay(uint8_t rs, uint8_t enable, uint8_t d4, uint8_t d5, uint8_t d6,
             uint8_t d7);

  Result begin() override;
  void clear() override;
  Result showLine(uint8_t row, const char* text) override;

  // The kit LCD's backlight is wired straight to 5V, so this does nothing.
  void setBacklight(bool on) override;

 private:
  LiquidCrystal lcd_;
};
