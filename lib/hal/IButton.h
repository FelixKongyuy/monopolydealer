// IButton.h - a debounced push button (the Pass GO button).
// Owner: Aung.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Contract:
//  - begin() once in setup(). update() every loop(); never blocks.
//  - wasPressed() returns true exactly once per press, then false until the
//    button is released and pressed again.

#pragma once

#include "Types.h"

class IButton {
 public:
  virtual Result begin() = 0;
  virtual void update() = 0;

  virtual bool wasPressed() = 0;

 protected:
  ~IButton() = default;
};
