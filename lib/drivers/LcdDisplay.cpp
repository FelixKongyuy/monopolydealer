#include "LcdDisplay.h"

LcdDisplay::LcdDisplay(uint8_t rs, uint8_t enable, uint8_t d4, uint8_t d5,
                       uint8_t d6, uint8_t d7)
    : lcd_(rs, enable, d4, d5, d6, d7) {}

Result LcdDisplay::begin() {
  // A parallel LCD can't report whether it is connected, so this always
  // succeeds. If the screen lights up but shows only boxes, turn the
  // contrast potentiometer.
  lcd_.begin(kDisplayCols, kDisplayRows);
  lcd_.clear();
  return Result::Ok;
}

void LcdDisplay::clear() { lcd_.clear(); }

Result LcdDisplay::showLine(uint8_t row, const char* text) {
  if (row >= kDisplayRows || text == nullptr) {
    return Result::InvalidArgument;
  }

  // Pad with spaces (or cut) to the full width so old characters are erased.
  char line[kDisplayCols + 1];
  uint8_t i = 0;
  for (; i < kDisplayCols && text[i] != '\0'; ++i) {
    line[i] = text[i];
  }
  for (; i < kDisplayCols; ++i) {
    line[i] = ' ';
  }
  line[kDisplayCols] = '\0';

  lcd_.setCursor(0, row);
  lcd_.print(line);
  return Result::Ok;
}

void LcdDisplay::setBacklight(bool) {}
