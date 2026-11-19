#include "DebouncedButton.h"

#include <Arduino.h>

DebouncedButton::DebouncedButton(uint8_t pin)
    : pin_(pin), lastLevel_(true), lastChangeMs_(0), pressPending_(false) {}

Result DebouncedButton::begin() {
  pinMode(pin_, INPUT_PULLUP);
  lastLevel_ = digitalRead(pin_) == HIGH;
  lastChangeMs_ = millis();
  return Result::Ok;
}

void DebouncedButton::update() {
  const bool level = digitalRead(pin_) == HIGH;
  const uint32_t now = millis();
  if (level != lastLevel_ && now - lastChangeMs_ > kDebounceMs) {
    lastChangeMs_ = now;
    lastLevel_ = level;
    if (!level) {  // LOW = pressed
      pressPending_ = true;
    }
  }
}

bool DebouncedButton::wasPressed() {
  const bool pressed = pressPending_;
  pressPending_ = false;
  return pressed;
}
