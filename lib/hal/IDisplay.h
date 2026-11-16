// IDisplay.h - the 16x2 I2C character LCD.
// Owner: Minh.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Contract:
//  - begin() once in setup(). Returns HardwareFault if the LCD does not
//    answer on I2C.
//  - showLine() writes text on row 0 or 1, padded with spaces or cut to
//    kDisplayCols characters, so old text never lingers. InvalidArgument for
//    a bad row.
//  - Text is a plain C string in RAM. Build it in a char[kDisplayCols + 1]
//    buffer (e.g. with snprintf). No String objects.
//  - Only call showLine() when the text changes; LCD writes over I2C are slow.

#pragma once

#include <stdint.h>

#include "Types.h"

constexpr uint8_t kDisplayCols = 16;
constexpr uint8_t kDisplayRows = 2;

class IDisplay {
 public:
  virtual Result begin() = 0;

  virtual void clear() = 0;
  virtual Result showLine(uint8_t row, const char* text) = 0;
  virtual void setBacklight(bool on) = 0;

 protected:
  ~IDisplay() = default;
};
