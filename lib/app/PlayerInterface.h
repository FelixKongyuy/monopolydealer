// PlayerInterface.h - the menus players use on the LCD and keypad.
// Owner: Minh. Based on Minh's player_interface sketch.
//
// Plain C++ only (no Arduino.h): it talks to the hardware through IKeypad,
// IDisplay and IButton and to the backend through IBank, so it runs in the
// laptop unit tests with fake parts.
//
// Keys:
//   A = withdraw   B = deposit   C = pay and get change
//   0-9 = type amount   * = delete digit   # = enter / yes   D = back / cancel
//   Pass GO button = collect $200 (main menu only)
//
// Nothing blocks. Messages that used to delay() now stay on screen until
// their time is up, and bank jobs run while update() keeps being called.

#pragma once

#include <stdint.h>

#include "IBank.h"
#include "IButton.h"
#include "IDisplay.h"
#include "IKeypad.h"

class PlayerInterface {
 public:
  enum class Screen : uint8_t {
    Menu,     // choose withdraw / deposit / pay
    Typing,   // entering an amount
    Confirm,  // "Withdraw $264?"
    Busy,     // waiting for the bank to finish a job
    Message,  // a timed message, then the next screen
  };

  static constexpr uint8_t kMaxDigits = 4;  // up to $9999
  static constexpr int32_t kPassGoAmount = 200;

  static constexpr uint32_t kSplashMs = 1500;
  static constexpr uint32_t kHintMs = 1000;
  static constexpr uint32_t kNoticeMs = 2000;
  static constexpr uint32_t kDoneMs = 2500;
  static constexpr uint32_t kErrorMs = 3000;

  PlayerInterface(IKeypad& keypad, IDisplay& display, IButton& passGo,
                  IBank& bank);

  // Call once in setup(), after every part's begin().
  void begin(uint32_t nowMs);

  // Call every loop() with millis(), after the parts' update().
  void update(uint32_t nowMs);

  Screen screen() const { return screen_; }
  int32_t amount() const { return amount_; }

 private:
  enum class Mode : uint8_t { Withdraw, Pay };

  enum class Job : uint8_t {
    None,
    Withdraw,      // paying out a withdrawal
    Deposit,       // counting a deposit
    PayCounting,   // counting what the player handed over
    PayReturning,  // player was short, giving their bills back
    PayChange,     // paying out change
    PassGo,        // paying out $200
  };

  void handleMenuKey(char key);
  void handleTypingKey(char key);
  void handleConfirmKey(char key);

  void showMenu();
  void showTyping();
  void showConfirm();
  void showLines(const char* line1, const char* line2);
  void showMessage(const char* line1, const char* line2, uint32_t durationMs,
                   Screen next);
  void showBusy(const char* line1, const char* line2, Job job);
  void showFailure(Result result);

  void startPayOut(int32_t amount, const char* line1, const char* line2,
                   Job job);
  void startCounting(Job job);
  void finishJob();

  const char* modeLabel() const;

  IKeypad& keypad_;
  IDisplay& display_;
  IButton& passGo_;
  IBank& bank_;

  Screen screen_ = Screen::Menu;
  Mode mode_ = Mode::Withdraw;
  Job job_ = Job::None;

  int32_t amount_ = 0;     // amount being typed
  uint8_t digitCount_ = 0;
  int32_t paidOut_ = 0;    // amount of the payout in progress

  uint32_t nowMs_ = 0;
  uint32_t messageStartMs_ = 0;
  uint32_t messageDurationMs_ = 0;
  Screen afterMessage_ = Screen::Menu;
};
