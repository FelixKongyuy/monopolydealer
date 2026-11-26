#include "ChangeMaker.h"

namespace {

void searchChange(int8_t index, int32_t remaining, uint16_t billsUsed,
                  const BillCounts& stock, BillCounts& current,
                  BillCounts& bestPlan, uint16_t& bestBills) {
  if (remaining == 0) {
    if (billsUsed < bestBills) {
      bestBills = billsUsed;
      bestPlan = current;
    }
    return;
  }

  if (index < 0 || billsUsed >= bestBills) {
    return;
  }

  const int32_t value = kDenomValue[index];
  int32_t maxTake = remaining / value;
  if (maxTake > stock.count[index]) {
    maxTake = stock.count[index];
  }

  for (int32_t take = maxTake; take >= 0; --take) {
    if (billsUsed + static_cast<uint16_t>(take) >= bestBills) {
      break;
    }

    current.count[index] = static_cast<uint8_t>(take);
    searchChange(index - 1, remaining - take * value,
                 billsUsed + static_cast<uint16_t>(take),
                 stock, current, bestPlan, bestBills);
  }

  current.count[index] = 0;
}

}

Result makeChange(int32_t amount, const BillCounts& stock, BillCounts& out) {
  out = emptyCounts();

  if (amount < 0) {
    return Result::InvalidArgument;
  }
  if (amount == 0) {
    return Result::Ok;
  }
  if (totalValue(stock) < amount) {
    return Result::InsufficientStock;
  }

  BillCounts current = emptyCounts();
  BillCounts bestPlan = emptyCounts();
  uint16_t bestBills = UINT16_MAX;

  searchChange(static_cast<int8_t>(kDenomCount - 1), amount, 0,
               stock, current, bestPlan, bestBills);

  if (bestBills == UINT16_MAX) {
    return Result::CannotMakeAmount;
  }

  out = bestPlan;
  return Result::Ok;