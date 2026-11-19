#include "MatrixKeypad.h"

namespace {

// Key layout printed on the Elegoo membrane keypad.
char keymap[MatrixKeypad::kRows][MatrixKeypad::kCols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

}  // namespace

MatrixKeypad::MatrixKeypad(const uint8_t (&rowPins)[kRows],
                           const uint8_t (&colPins)[kCols])
    : rowPins_{rowPins[0], rowPins[1], rowPins[2], rowPins[3]},
      colPins_{colPins[0], colPins[1], colPins[2], colPins[3]},
      keypad_(makeKeymap(keymap), rowPins_, colPins_, kRows, kCols) {}

Result MatrixKeypad::begin() { return Result::Ok; }

// The Keypad library scans and debounces inside getKey().
void MatrixKeypad::update() {}

char MatrixKeypad::getKey() {
  const char key = keypad_.getKey();
  return key == NO_KEY ? kNoKey : key;
}
