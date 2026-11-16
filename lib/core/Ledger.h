// Ledger.h - how many bills of each denomination are in the carousel.
// Owner: Aung. Implementation goes in Ledger.cpp.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// The ledger must always match what is physically in the slots. Record a
// bill only after the hardware reports it actually moved.
//
// Contract:
//  - A new Ledger starts with every count at zero.
//  - setCounts() loads the starting stock at the beginning of a game.
//    InvalidArgument if any count exceeds kSlotCapacity.
//  - hasRoom(d) is true if slot d can take one more bill. Check it before
//    sorting a bill into that slot.
//  - recordSorted(d): a bill landed in slot d. SlotFull if already at
//    capacity, InvalidArgument for Unknown/Count.
//  - recordDispensed(d): a bill left slot d. InvalidArgument if the count
//    is already zero or d is not a real denomination.
//  - total() is the dollar value of everything in the carousel.

#pragma once

#include <stdint.h>

#include "Money.h"
#include "Types.h"

class Ledger {
 public:
  Ledger();

  Result setCounts(const BillCounts& counts);

  bool hasRoom(Denom d) const;
  Result recordSorted(Denom d);
  Result recordDispensed(Denom d);

  uint8_t count(Denom d) const;
  const BillCounts& counts() const;
  int32_t total() const;

 private:
  BillCounts counts_;
};
