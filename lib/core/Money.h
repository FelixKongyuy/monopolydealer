// Money.h - Monopoly denominations and bill counts.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// Rules:
//  - Money is always a whole number of dollars in an int32_t. Never float.
//    (int on the Uno is only 16 bits and overflows past 32,767.)
//  - Carousel slot n holds denomination n: slot 0 = $1 ... slot 6 = $500.
//    The CAD team labels the physical slots in the same order.

#pragma once

#include <stdint.h>

enum class Denom : uint8_t {
  D1 = 0,
  D5,
  D10,
  D20,
  D50,
  D100,
  D500,
  Count,           // number of real denominations; not a bill
  Unknown = 0xFF,  // the classifier could not tell; never stored in a slot
};

constexpr uint8_t kDenomCount = static_cast<uint8_t>(Denom::Count);
constexpr uint8_t kSlotCount = kDenomCount;

// Bills one carousel slot can hold (storage design: 45 per slot, 315 total).
constexpr uint8_t kSlotCapacity = 45;

// Value of each denomination in dollars, lowest to highest, indexed by Denom.
constexpr uint16_t kDenomValue[kDenomCount] = {1, 5, 10, 20, 50, 100, 500};

constexpr uint8_t indexOf(Denom d) { return static_cast<uint8_t>(d); }

// Only call with a real denomination (not Count or Unknown).
constexpr uint16_t valueOf(Denom d) { return kDenomValue[indexOf(d)]; }

constexpr bool isValid(Denom d) { return indexOf(d) < kDenomCount; }

constexpr Denom denomAt(uint8_t index) { return static_cast<Denom>(index); }

// How many bills of each denomination, indexed by Denom. Used for the bank's
// stock, a change plan, or the bills a player inserted.
struct BillCounts {
  uint8_t count[kDenomCount];
};

// All-zero BillCounts.
inline BillCounts emptyCounts() {
  BillCounts c = {};
  return c;
}

// Total dollar value of a set of bills.
inline int32_t totalValue(const BillCounts& c) {
  int32_t sum = 0;
  for (uint8_t i = 0; i < kDenomCount; ++i) {
    sum += static_cast<int32_t>(c.count[i]) * kDenomValue[i];
  }
  return sum;
}
