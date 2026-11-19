// IBank.h - what the player interface needs from the backend.
// Owners: Aung (implements it), Minh (uses it).
//
// DRAFT v0 (2026-11-19). Not frozen yet: Aung should review and adjust
// before the real backend is written. Replaces the three placeholder
// functions in Minh's original sketch (backendDispense, backendCountDeposit,
// backendCanDispense).
//
// Contract:
//  - Plain C++ only (no Arduino.h), so the UI can be tested on a laptop.
//  - Nothing blocks. A start...() call begins a job and returns at once.
//    Poll isBusy(); when it turns false, lastResult() says how it went.
//  - canPayOut(amount) is true if the bank holds bills that add up to
//    exactly `amount` right now.
//  - startPayOut(amount) pays out exactly `amount` using the fewest bills.
//    Returns InsufficientStock / CannotMakeAmount straight away if it can't.
//  - startCounting() takes in the player's bills one by one, sorts them and
//    adds up their value. countedTotal() is that value once the job ends.
//  - Money is whole dollars in int32_t.

#pragma once

#include <stdint.h>

#include "Types.h"

class IBank {
 public:
  virtual bool canPayOut(int32_t amount) const = 0;

  virtual Result startPayOut(int32_t amount) = 0;
  virtual Result startCounting() = 0;

  virtual bool isBusy() const = 0;
  virtual Result lastResult() const = 0;
  virtual int32_t countedTotal() const = 0;

 protected:
  ~IBank() = default;
};
