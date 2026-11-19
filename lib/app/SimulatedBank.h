// SimulatedBank.h - stand-in backend so the player interface can run on the
// board before the real motors and sensor code exist.
//
// Behaves like the placeholders in Minh's original sketch: every job takes
// one second and succeeds, counting always finds $500, and the bank can pay
// out up to $5000. Replace with Aung's real IBank in main.cpp when ready.

#pragma once

#include <stdint.h>

#include "IBank.h"

class SimulatedBank final : public IBank {
 public:
  static constexpr uint32_t kJobMs = 1000;
  static constexpr int32_t kCountedAmount = 500;
  static constexpr int32_t kPayOutLimit = 5000;

  // Call every loop() with millis().
  void update(uint32_t nowMs) {
    nowMs_ = nowMs;
    if (busy_ && nowMs_ - jobStartMs_ >= kJobMs) {
      busy_ = false;
    }
  }

  bool canPayOut(int32_t amount) const override {
    return amount > 0 && amount <= kPayOutLimit;
  }

  Result startPayOut(int32_t amount) override {
    if (busy_) return Result::Busy;
    if (!canPayOut(amount)) return Result::InsufficientStock;
    startJob();
    return Result::Ok;
  }

  Result startCounting() override {
    if (busy_) return Result::Busy;
    counted_ = kCountedAmount;
    startJob();
    return Result::Ok;
  }

  bool isBusy() const override { return busy_; }
  Result lastResult() const override { return Result::Ok; }
  int32_t countedTotal() const override { return counted_; }

 private:
  void startJob() {
    busy_ = true;
    jobStartMs_ = nowMs_;
  }

  uint32_t nowMs_ = 0;
  uint32_t jobStartMs_ = 0;
  bool busy_ = false;
  int32_t counted_ = 0;
};
