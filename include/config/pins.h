// pins.h - every pin number in the project. Nothing else may hard-code a pin.
// Owner: Zen.
//
// PROVISIONAL: the player interface pins below come from Minh's prototype
// using the Elegoo kit's parallel LCD1602 and membrane keypad.
//
// Pin budget warning: this layout uses 15 of the Uno's pins (D2-D12, A0-A3),
// leaving only D13 plus A4/A5 (I2C) free. The carousel stepper (4 pins),
// home switch (1) and two roller motors (4-6) will not fit. The planned fix
// is moving the LCD onto an I2C backpack and the keypad onto a PCF8574
// expander, which frees 14 pins. Only main.cpp and the drivers change when
// that happens; the UI code talks to IDisplay/IKeypad and stays the same.

#pragma once

#include <Arduino.h>
#include <stdint.h>

// ---------- LCD1602, parallel 4-bit mode ----------
// LCD pins 1 VSS->GND, 2 VDD->5V, 3 V0->10k pot wiper, 5 RW->GND,
// 15 A->5V through 220 ohm, 16 K->GND. Data pins D0-D3 unused.
constexpr uint8_t kPinLcdRs = 12;
constexpr uint8_t kPinLcdEnable = 11;
constexpr uint8_t kPinLcdD4 = A0;
constexpr uint8_t kPinLcdD5 = A1;
constexpr uint8_t kPinLcdD6 = A2;
constexpr uint8_t kPinLcdD7 = A3;

// ---------- 4x4 membrane keypad ----------
// Keypad ribbon, left to right: D9 D8 D7 D6 (rows) D5 D4 D3 D2 (columns).
constexpr uint8_t kKeypadRows = 4;
constexpr uint8_t kKeypadCols = 4;
constexpr uint8_t kPinKeypadRows[kKeypadRows] = {9, 8, 7, 6};
constexpr uint8_t kPinKeypadCols[kKeypadCols] = {5, 4, 3, 2};

// ---------- Pass GO button ----------
// One leg to this pin, the other to GND. Uses the internal pull-up.
constexpr uint8_t kPinPassGo = 10;
