// Types.h - shared result codes and data types used by every module.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses the type you change.
//
// Plain C++ only: no Arduino.h, so this compiles on the Uno and on a laptop
// for unit tests.

#pragma once

#include <stdint.h>

// Every function that can fail returns one of these instead of failing silently.
enum class Result : uint8_t {
  Ok = 0,
  Busy,               // a previous command is still running
  NotReady,           // begin() or homing has not finished yet
  InvalidArgument,    // bad input, e.g. slot out of range or negative amount
  InsufficientStock,  // the bank does not hold enough money in total
  CannotMakeAmount,   // enough money in total, but no exact combination of bills
  SlotFull,           // a carousel slot is at capacity
  Jam,                // a bill did not move when it should have
  Empty,              // no bill available to feed
  UnknownBill,        // colour reading did not match any denomination
  HardwareFault,      // a sensor or driver is not responding
};

// One raw reading from the colour sensor. Units are the sensor's raw counts.
struct ColorReading {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
  uint16_t clear;  // overall brightness
};
