#include "Ledger.h"

namespace {

bool isValidDenom(Denom d) {
  return static_cast<uint8_t>(d) < static_cast<uint8_t>(Denom::Count);
}

}

Result Ledger::setCounts(const BillCounts& counts) {
  for (uint8_t i = 0; i < kDenomCount; ++i) {
    if (counts.count[i] > kSlotCapacity) {
      return Result::InvalidArgument;
    }
  }

  counts_ = counts;
  return Result::Ok;
}

bool Ledger::hasRoom(Denom d) const {
  if (!isValidDenom(d)) {
    return false;
  }
  return counts_.count[static_cast<uint8_t>(d)] < kSlotCapacity;
}

Result Ledger::recordSorted(Denom d) {
  if (!isValidDenom(d)) {
    return Result::InvalidArgument;
  }

  const uint8_t index = static_cast<uint8_t>(d);
  if (counts_.count[index] >= kSlotCapacity) {
    return Result::SlotFull;
  }

  ++counts_.count[index];
  return Result::Ok;
}

Result Ledger::recordDispensed(Denom d) {
  if (!isValidDenom(d)) {
    return Result::InvalidArgument;
  }

  const uint8_t index = static_cast<uint8_t>(d);
  if (counts_.count[index] == 0) {
    return Result::InvalidArgument;
  }

  --counts_.count[index];
  return Result::Ok;
}

uint8_t Ledger::count(Denom d) const {
  if (!isValidDenom(d)) {
    return 0;
  }
  return counts_.count[static_cast<uint8_t>(d)];
}

const BillCounts& Ledger::counts() const {
  return counts_;
}

int32_t Ledger::total() const {
  return totalValue(counts_);
}