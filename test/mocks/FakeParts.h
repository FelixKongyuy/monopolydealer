// FakeParts.h - fake hardware and backend for laptop unit tests.
//
// Each fake implements the real interface, so app code can't tell the
// difference. Tests drive them directly: press a key, finish a bank job,
// read what the screen shows.

#pragma once

#include <stdint.h>

#include "IBank.h"
#include "IButton.h"
#include "IDisplay.h"
#include "IKeypad.h"

// Returns one queued key on the next getKey(), then kNoKey.
class FakeKeypad final : public IKeypad {
 public:
  Result begin() override { return Result::Ok; }
  void update() override {}
  char getKey() override {
    const char key = next_;
    next_ = kNoKey;
    return key;
  }

  void press(char key) { next_ = key; }

 private:
  char next_ = kNoKey;
};

// Remembers the last text written to each row.
class FakeDisplay final : public IDisplay {
 public:
  Result begin() override { return Result::Ok; }
  void clear() override {
    for (uint8_t row = 0; row < kDisplayRows; ++row) lines_[row][0] = '\0';
  }
  Result showLine(uint8_t row, const char* text) override {
    if (row >= kDisplayRows || text == nullptr) return Result::InvalidArgument;
    uint8_t i = 0;
    for (; i < kDisplayCols && text[i] != '\0'; ++i) lines_[row][i] = text[i];
    lines_[row][i] = '\0';
    return Result::Ok;
  }
  void setBacklight(bool) override {}

  const char* line(uint8_t row) const { return lines_[row]; }

 private:
  char lines_[kDisplayRows][kDisplayCols + 1] = {};
};

// wasPressed() is true once after press().
class FakeButton final : public IButton {
 public:
  Result begin() override { return Result::Ok; }
  void update() override {}
  bool wasPressed() override {
    const bool pressed = pressed_;
    pressed_ = false;
    return pressed;
  }

  void press() { pressed_ = true; }

 private:
  bool pressed_ = false;
};

// Jobs stay busy until the test calls finish() or finishCounting().
class FakeBank final : public IBank {
 public:
  bool canPayOut(int32_t amount) const override {
    return amount > 0 && amount <= stock;
  }
  Result startPayOut(int32_t amount) override {
    if (!canPayOut(amount)) return Result::InsufficientStock;
    ++payOutCount;
    lastPayOut = amount;
    busy = true;
    return Result::Ok;
  }
  Result startCounting() override {
    ++countingCount;
    busy = true;
    return Result::Ok;
  }
  bool isBusy() const override { return busy; }
  Result lastResult() const override { return result; }
  int32_t countedTotal() const override { return counted; }

  void finish(Result outcome = Result::Ok) {
    result = outcome;
    busy = false;
  }
  void finishCounting(int32_t total) {
    counted = total;
    finish();
  }

  int32_t stock = 5000;  // largest payout the fake bank allows
  int payOutCount = 0;
  int countingCount = 0;
  int32_t lastPayOut = 0;
  int32_t counted = 0;
  bool busy = false;
  Result result = Result::Ok;
};
