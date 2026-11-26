#include "IRoller.h"
#include <Arduino.h>

namespace {

constexpr uint8_t kUnusedPin = 255;

struct RollerConfig {
  uint8_t motorPin;          // PWM or digital pin driving the motor
  uint8_t motorDirPin;       // Direction pin (255 if unidirectional)
  uint8_t sensorPin;         // IR break-beam / optical exit sensor pin
  bool sensorActiveLow;      // True if sensor reads LOW when a bill is present
  uint8_t motorSpeed;        // PWM duty cycle (0-255), 255 = full ON
  uint32_t emptyTimeoutMs;   // Max wait for bill leading edge to hit sensor
  uint32_t jamTimeoutMs;     // Max time bill can block sensor before Jam
  uint32_t clearDelayMs;     // Extra run time after trailing edge clears sensor
};

class HardwareRoller final : public IRoller {
 public:
  explicit HardwareRoller(const RollerConfig& config) : cfg_(config) {}

  Result begin() override {
    pinMode(cfg_.motorPin, OUTPUT);
    if (cfg_.motorDirPin != kUnusedPin) {
      pinMode(cfg_.motorDirPin, OUTPUT);
      digitalWrite(cfg_.motorDirPin, HIGH);
    }

    pinMode(cfg_.sensorPin, cfg_.sensorActiveLow ? INPUT_PULLUP : INPUT);

    setMotor(false);
    state_ = State::Idle;
    lastResult_ = Result::Ok;
    return Result::Ok;
  }

  Result startFeedOne() override {
    if (state_ == State::Uninitialized) {
      return Result::NotReady;
    }
    if (isBusy()) {
      return Result::Busy;
    }
    if (isBillDetected()) {
      lastResult_ = Result::Jam;
      return Result::Jam;
    }

    stateStartTimeMs_ = millis();
    state_ = State::FeedingWaitForLeadingEdge;
    setMotor(true);
    return Result::Ok;
  }

  void update() override {
    if (!isBusy()) {
      return;
    }

    const uint32_t now = millis();
    const uint32_t elapsed = now - stateStartTimeMs_;
    const bool billPresent = isBillDetected();

    switch (state_) {
      case State::FeedingWaitForLeadingEdge:
        if (billPresent) {
          state_ = State::FeedingWaitForTrailingEdge;
          stateStartTimeMs_ = now;
        } else if (elapsed >= cfg_.emptyTimeoutMs) {
          finish(Result::Empty);
        }
        break;

      case State::FeedingWaitForTrailingEdge:
        if (!billPresent) {
          if (cfg_.clearDelayMs == 0) {
            finish(Result::Ok);
          } else {
            state_ = State::FeedingClearing;
            stateStartTimeMs_ = now;
          }
        } else if (elapsed >= cfg_.jamTimeoutMs) {
          finish(Result::Jam);
        }
        break;

      case State::FeedingClearing:
        if (billPresent) {
          finish(Result::Jam);  // Double-feed detected
        } else if (elapsed >= cfg_.clearDelayMs) {
          finish(Result::Ok);
        }
        break;

      case State::Uninitialized:
      case State::Idle:
        break;
    }
  }

  void stop() override {
    setMotor(false);
    if (state_ != State::Uninitialized) {
      state_ = State::Idle;
    }
  }

  bool isBusy() const override {
    return state_ == State::FeedingWaitForLeadingEdge ||
           state_ == State::FeedingWaitForTrailingEdge ||
           state_ == State::FeedingClearing;
  }

  Result lastResult() const override {
    return lastResult_;
  }

 private:
  enum class State : uint8_t {
    Uninitialized,
    Idle,
    FeedingWaitForLeadingEdge,
    FeedingWaitForTrailingEdge,
    FeedingClearing
  };

  void setMotor(bool on) {
    if (!on) {
      analogWrite(cfg_.motorPin, 0);
      digitalWrite(cfg_.motorPin, LOW);
      return;
    }

    if (cfg_.motorSpeed >= 255) {
      digitalWrite(cfg_.motorPin, HIGH);
    } else {
      analogWrite(cfg_.motorPin, cfg_.motorSpeed);
    }
  }

  bool isBillDetected() const {
    const int raw = digitalRead(cfg_.sensorPin);
    return cfg_.sensorActiveLow ? (raw == LOW) : (raw == HIGH);
  }

  void finish(Result outcome) {
    setMotor(false);
    lastResult_ = outcome;
    state_ = State::Idle;
  }

  RollerConfig cfg_;
  State state_ = State::Uninitialized;
  Result lastResult_ = Result::NotReady;
  uint32_t stateStartTimeMs_ = 0;
};

// Configure pins and timings for both rollers here
HardwareRoller inputRollerImpl({
    .motorPin = 5,
    .motorDirPin = 4,
    .sensorPin = 2,
    .sensorActiveLow = true,
    .motorSpeed = 200,
    .emptyTimeoutMs = 800,
    .jamTimeoutMs = 1500,
    .clearDelayMs = 100
});

HardwareRoller outputRollerImpl({
    .motorPin = 6,
    .motorDirPin = 7,
    .sensorPin = 3,
    .sensorActiveLow = true,
    .motorSpeed = 220,
    .emptyTimeoutMs = 800,
    .jamTimeoutMs = 1500,
    .clearDelayMs = 120
});

}  // namespace

IRoller& inputRoller = inputRollerImpl;
IRoller& outputRoller = outputRollerImpl;