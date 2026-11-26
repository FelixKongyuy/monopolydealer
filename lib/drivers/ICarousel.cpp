// ICarousel.cpp - StepperCarousel implementation for 28BYJ-48 + ULN2003 + home switch.
#include <Arduino.h>
#include "ICarousel.h"


namespace {

// --- MATCH THESE 5 NAMES TO YOUR pins.h ---
constexpr uint8_t kPinIn1 = CAROUSEL_IN1;
constexpr uint8_t kPinIn2 = CAROUSEL_IN2;
constexpr uint8_t kPinIn3 = CAROUSEL_IN3;
constexpr uint8_t kPinIn4 = CAROUSEL_IN4;
constexpr uint8_t kPinHome = CAROUSEL_HOME_PIN;

// --- HARDWARE TUNING ---
constexpr bool kHomeActiveLow = true;          // true = switch connects pin to GND
constexpr int32_t kStepsPerRev = 4096;         // 4096 half-steps per revolution
constexpr int32_t kOutputStationOffset = 0;    // 0 if Input & Output rollers share angle
constexpr uint32_t kStepIntervalUs = 2500;     // 2.5 ms per half-step (~10 RPM)
constexpr int32_t kAccelRampSteps = 32;        // Smooth ramp steps at start/end

constexpr uint8_t kHalfStepTable[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
};

class StepperCarousel final : public ICarousel {
 public:
  Result begin() override {
    pinMode(kPinIn1, OUTPUT);
    pinMode(kPinIn2, OUTPUT);
    pinMode(kPinIn3, OUTPUT);
    pinMode(kPinIn4, OUTPUT);
    pinMode(kPinHome, kHomeActiveLow ? INPUT_PULLUP : INPUT);

    releaseCoils();

    state_ = State::Unhomed;
    homed_ = false;
    currentSlot_ = 0;
    currentStep_ = 0;
    stepsRemaining_ = 0;
    lastResult_ = Result::Ok;
    return Result::Ok;
  }

  void update() override {
    if (state_ == State::Uninitialized || state_ == State::Unhomed ||
        state_ == State::Idle) {
      return;
    }

    const uint32_t now = micros();
    if (static_cast<uint32_t>(now - lastStepMicros_) < currentIntervalUs()) {
      return;
    }
    lastStepMicros_ = now;

    // 1. Back off if already pressing the home switch when homing starts
    if (state_ == State::HomingClearSwitch) {
      if (!isHomeTriggered()) {
        homingSteps_ = 0;
        state_ = State::HomingSeekSwitch;
        return;
      }

      stepMotor(-1);
      ++homingSteps_;

      if (homingSteps_ > kStepsPerRev / 4) {
        releaseCoils();
        homed_ = false;
        state_ = State::Unhomed;
        lastResult_ = Result::HardwareFault;
      }
      return;
    }

    // 2. Rotate forward until home switch triggers
    if (state_ == State::HomingSeekSwitch) {
      if (isHomeTriggered()) {
        releaseCoils();
        currentStep_ = 0;
        currentSlot_ = 0;
        stepsRemaining_ = 0;
        homed_ = true;
        state_ = State::Idle;
        lastResult_ = Result::Ok;
        return;
      }

      stepMotor(+1);
      ++homingSteps_;

      if (homingSteps_ > (kStepsPerRev * 5) / 4) {
        releaseCoils();
        homed_ = false;
        state_ = State::Unhomed;
        lastResult_ = Result::HardwareFault;
      }
      return;
    }

    // 3. Move to target slot
    if (state_ == State::Moving) {
      if (stepsRemaining_ == 0) {
        releaseCoils();
        currentSlot_ = targetSlot_;
        state_ = State::Idle;
        lastResult_ = Result::Ok;
        return;
      }

      const int8_t dir = (stepsRemaining_ > 0) ? +1 : -1;
      stepMotor(dir);
      stepsRemaining_ -= dir;

      currentStep_ = (currentStep_ + dir) % kStepsPerRev;
      if (currentStep_ < 0) {
        currentStep_ += kStepsPerRev;
      }

      if (stepsRemaining_ == 0) {
        releaseCoils();
        currentSlot_ = targetSlot_;
        state_ = State::Idle;
        lastResult_ = Result::Ok;
      }
    }
  }

  Result startHoming() override {
    if (state_ == State::Uninitialized) {
      return Result::NotReady;
    }
    if (isBusy()) {
      return Result::Busy;
    }

    homed_ = false;
    homingSteps_ = 0;
    lastStepMicros_ = micros();
    lastResult_ = Result::Ok;
    state_ = isHomeTriggered() ? State::HomingClearSwitch : State::HomingSeekSwitch;
    return Result::Ok;
  }

  Result moveToSlot(uint8_t slot, Station station) override {
    if (state_ == State::Uninitialized || !homed_) {
      return Result::NotReady;
    }
    if (isBusy()) {
      return Result::Busy;
    }
    if (slot >= kSlotCount) {
      return Result::InvalidArgument;
    }

    const int32_t target = targetStepFor(slot, station);

    // Shortest path around the 360-degree carousel
    int32_t delta = target - currentStep_;
    const int32_t halfRev = kStepsPerRev / 2;
    if (delta > halfRev) {
      delta -= kStepsPerRev;
    } else if (delta < -halfRev) {
      delta += kStepsPerRev;
    }

    targetSlot_ = slot;

    if (delta == 0) {
      currentSlot_ = slot;
      lastResult_ = Result::Ok;
      return Result::Ok;
    }

    stepsRemaining_ = delta;
    moveTotalSteps_ = (delta >= 0) ? delta : -delta;
    lastStepMicros_ = micros();
    lastResult_ = Result::Ok;
    state_ = State::Moving;
    return Result::Ok;
  }

  void stop() override {
    releaseCoils();
    stepsRemaining_ = 0;
    moveTotalSteps_ = 0;
    homed_ = false;
    if (state_ != State::Uninitialized) {
      state_ = State::Unhomed;
    }
  }

  bool isBusy() const override {
    return state_ == State::HomingClearSwitch ||
           state_ == State::HomingSeekSwitch ||
           state_ == State::Moving;
  }

  bool isHomed() const override { return homed_; }
  uint8_t currentSlot() const override { return currentSlot_; }
  Result lastResult() const override { return lastResult_; }

 private:
  enum class State : uint8_t {
    Uninitialized,
    Unhomed,
    HomingClearSwitch,
    HomingSeekSwitch,
    Idle,
    Moving,
  };

  State state_ = State::Uninitialized;
  bool homed_ = false;
  uint8_t currentSlot_ = 0;
  uint8_t targetSlot_ = 0;
  Result lastResult_ = Result::NotReady;

  int32_t currentStep_ = 0;
  int32_t stepsRemaining_ = 0;
  int32_t moveTotalSteps_ = 0;
  int32_t homingSteps_ = 0;
  uint8_t phaseIndex_ = 0;
  uint32_t lastStepMicros_ = 0;

  static bool isHomeTriggered() {
    const int raw = digitalRead(kPinHome);
    return kHomeActiveLow ? (raw == LOW) : (raw == HIGH);
  }

  static int32_t targetStepFor(uint8_t slot, Station station) {
    int32_t step =
        (static_cast<int32_t>(slot) * kStepsPerRev + (kSlotCount / 2)) / kSlotCount;
    if (station == Station::Output) {
      step += kOutputStationOffset;
    }
    step %= kStepsPerRev;
    if (step < 0) {
      step += kStepsPerRev;
    }
    return step;
  }

  uint32_t currentIntervalUs() const {
    if (state_ == State::HomingClearSwitch || state_ == State::HomingSeekSwitch) {
      return (kStepIntervalUs * 3) / 2;
    }
    const int32_t rem = (stepsRemaining_ >= 0) ? stepsRemaining_ : -stepsRemaining_;
    const int32_t done = moveTotalSteps_ - rem;
    const int32_t edge = (done < rem) ? done : rem;
    if (edge >= kAccelRampSteps) {
      return kStepIntervalUs;
    }
    return kStepIntervalUs +
           (kStepIntervalUs * static_cast<uint32_t>(kAccelRampSteps - edge)) /
               static_cast<uint32_t>(kAccelRampSteps);
  }

  void stepMotor(int8_t dir) {
    phaseIndex_ = (dir > 0) ? ((phaseIndex_ + 1) & 0x07)
                            : ((phaseIndex_ + 7) & 0x07);
    digitalWrite(kPinIn1, kHalfStepTable[phaseIndex_][0]);
    digitalWrite(kPinIn2, kHalfStepTable[phaseIndex_][1]);
    digitalWrite(kPinIn3, kHalfStepTable[phaseIndex_][2]);
    digitalWrite(kPinIn4, kHalfStepTable[phaseIndex_][3]);
  }

  static void releaseCoils() {
    digitalWrite(kPinIn1, LOW);
    digitalWrite(kPinIn2, LOW);
    digitalWrite(kPinIn3, LOW);
    digitalWrite(kPinIn4, LOW);
  }
};

}  // namespace

// Factory accessor so main.cpp can grab the ICarousel instance without another .h file
ICarousel& getCarousel() {
  static StepperCarousel instance;
  return instance;
}