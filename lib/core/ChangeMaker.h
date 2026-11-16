// ChangeMaker.h - works out which bills to pay out.
// Owner: Aung. Implementation goes in ChangeMaker.cpp.
//
// FROZEN v1 (2026-11-15). Change only through a pull request reviewed by
// Felix and by the owner of every module that uses it.
//
// makeChange(amount, stock, out):
//  - Ok: `out` holds the FEWEST bills that add up to exactly `amount`, and
//    never more of a denomination than `stock` has. amount == 0 gives Ok
//    with no bills.
//  - InvalidArgument: amount < 0.
//  - InsufficientStock: totalValue(stock) < amount.
//  - CannotMakeAmount: enough money in total, but no exact combination
//    (e.g. owe $3 while holding only $5 bills).
//  - On any error, `out` is all zeros.
//
// Implementation rules:
//  - Plain greedy (largest bill first) is NOT enough once stock runs low.
//    Example: owe $60 holding one $50, three $20, no $10 or $5.
//    Greedy takes the $50 and gets stuck; the answer is 3 x $20.
//  - No heap, no big lookup tables (the Uno has 2 KB of RAM). A depth-first
//    search over the 7 denominations with pruning fits easily.

#pragma once

#include <stdint.h>

#include "Money.h"
#include "Types.h"

Result makeChange(int32_t amount, const BillCounts& stock, BillCounts& out);
