// main.cpp - builds every part and runs them. No logic lives here.
// Owner: Zen.

#include <Arduino.h>

#include "DebouncedButton.h"
#include "LcdDisplay.h"
#include "MatrixKeypad.h"
#include "PlayerInterface.h"
#include "SimulatedBank.h"
#include "config/pins.h"

// ---------- Hardware ----------
LcdDisplay display(kPinLcdRs, kPinLcdEnable, kPinLcdD4, kPinLcdD5, kPinLcdD6,
                   kPinLcdD7);
MatrixKeypad keypad(kPinKeypadRows, kPinKeypadCols);
DebouncedButton passGoButton(kPinPassGo);

// ---------- Backend ----------
// Stand-in until Aung's real IBank exists.
SimulatedBank bank;

// ---------- App ----------
PlayerInterface ui(keypad, display, passGoButton, bank);

void setup() {
  display.begin();
  keypad.begin();
  passGoButton.begin();
  ui.begin(millis());
}

void loop() {
  const uint32_t now = millis();

  keypad.update();
  passGoButton.update();
  bank.update(now);

  ui.update(now);
}
