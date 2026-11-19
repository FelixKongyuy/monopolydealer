#include "PlayerInterface.h"

#include <stdio.h>

namespace {

// One LCD line plus the terminating '\0'.
constexpr uint8_t kLineSize = kDisplayCols + 1;

}  // namespace

PlayerInterface::PlayerInterface(IKeypad& keypad, IDisplay& display,
                                 IButton& passGo, IBank& bank)
    : keypad_(keypad), display_(display), passGo_(passGo), bank_(bank) {}

void PlayerInterface::begin(uint32_t nowMs) {
  nowMs_ = nowMs;
  showMessage(" MONOPOLY BANK", "  Starting...", kSplashMs, Screen::Menu);
}

void PlayerInterface::update(uint32_t nowMs) {
  nowMs_ = nowMs;

  // Read both inputs every time so presses made on other screens are
  // dropped instead of firing later.
  const char key = keypad_.getKey();
  const bool passGoPressed = passGo_.wasPressed();

  switch (screen_) {
    case Screen::Menu:
      if (key != kNoKey) handleMenuKey(key);
      break;
    case Screen::Typing:
      if (key != kNoKey) handleTypingKey(key);
      break;
    case Screen::Confirm:
      if (key != kNoKey) handleConfirmKey(key);
      break;
    case Screen::Busy:
      if (!bank_.isBusy()) finishJob();
      break;
    case Screen::Message:
      if (nowMs_ - messageStartMs_ >= messageDurationMs_) {
        if (afterMessage_ == Screen::Typing) {
          showTyping();
        } else {
          showMenu();
        }
      }
      break;
  }

  // Pass GO only works from the main menu so it can't interrupt anything.
  if (passGoPressed && screen_ == Screen::Menu) {
    startPayOut(kPassGoAmount, "PASS GO!", "Collect $200", Job::PassGo);
  }
}

// ---------- Key handling ----------

void PlayerInterface::handleMenuKey(char key) {
  if (key == 'A') {
    mode_ = Mode::Withdraw;
    showTyping();
  } else if (key == 'B') {
    startCounting(Job::Deposit);
  } else if (key == 'C') {
    mode_ = Mode::Pay;
    showTyping();
  }
}

void PlayerInterface::handleTypingKey(char key) {
  if (key >= '0' && key <= '9') {
    if (digitCount_ == 0 && key == '0') return;  // no leading zeros
    if (digitCount_ < kMaxDigits) {
      amount_ = amount_ * 10 + (key - '0');
      ++digitCount_;
    }
    showTyping();
  } else if (key == '*') {  // delete last digit
    if (digitCount_ > 0) {
      amount_ /= 10;
      --digitCount_;
    }
    showTyping();
  } else if (key == '#') {
    if (amount_ > 0) {
      showConfirm();
    } else {
      showMessage("Enter an amount", "", kHintMs, Screen::Typing);
    }
  } else if (key == 'D') {
    showMenu();
  }
}

void PlayerInterface::handleConfirmKey(char key) {
  if (key == '#') {
    if (mode_ == Mode::Withdraw) {
      if (!bank_.canPayOut(amount_)) {
        showFailure(Result::InsufficientStock);
        return;
      }
      startPayOut(amount_, "Dispensing...", "", Job::Withdraw);
    } else {
      startCounting(Job::PayCounting);
    }
  } else if (key == 'D') {
    showTyping();  // go back and edit
  }
}

// ---------- Screens ----------

void PlayerInterface::showMenu() {
  screen_ = Screen::Menu;
  amount_ = 0;
  digitCount_ = 0;
  showLines("A:Withdraw B:Dep", "C:Pay  GO=$200");
}

void PlayerInterface::showTyping() {
  screen_ = Screen::Typing;
  char line[kLineSize];
  if (digitCount_ == 0) {
    snprintf(line, sizeof(line), "%s $_", modeLabel());
  } else {
    snprintf(line, sizeof(line), "%s $%ld", modeLabel(),
             static_cast<long>(amount_));
  }
  showLines(line, "#=OK *=Del D=Bk");
}

void PlayerInterface::showConfirm() {
  screen_ = Screen::Confirm;
  char line[kLineSize];
  snprintf(line, sizeof(line), "%s $%ld?", modeLabel(),
           static_cast<long>(amount_));
  showLines(line, "#=Yes  D=No");
}

void PlayerInterface::showLines(const char* line1, const char* line2) {
  display_.showLine(0, line1);
  display_.showLine(1, line2);
}

void PlayerInterface::showMessage(const char* line1, const char* line2,
                                  uint32_t durationMs, Screen next) {
  screen_ = Screen::Message;
  messageStartMs_ = nowMs_;
  messageDurationMs_ = durationMs;
  afterMessage_ = next;
  showLines(line1, line2);
}

void PlayerInterface::showBusy(const char* line1, const char* line2, Job job) {
  screen_ = Screen::Busy;
  job_ = job;
  showLines(line1, line2);
}

void PlayerInterface::showFailure(Result result) {
  job_ = Job::None;
  if (result == Result::InsufficientStock ||
      result == Result::CannotMakeAmount) {
    showMessage("Not enough bills", "in the bank", kNoticeMs, Screen::Menu);
  } else if (result == Result::Jam) {
    showMessage("ERROR: bill jam", "Call for help", kErrorMs, Screen::Menu);
  } else if (result == Result::UnknownBill) {
    showMessage("Unknown bill", "Call for help", kErrorMs, Screen::Menu);
  } else {
    showMessage("ERROR", "Call for help", kErrorMs, Screen::Menu);
  }
}

// ---------- Bank jobs ----------

void PlayerInterface::startPayOut(int32_t amount, const char* line1,
                                  const char* line2, Job job) {
  paidOut_ = amount;
  showBusy(line1, line2, job);
  const Result result = bank_.startPayOut(amount);
  if (result != Result::Ok) showFailure(result);
}

void PlayerInterface::startCounting(Job job) {
  showBusy("Insert bills", "Counting...", job);
  const Result result = bank_.startCounting();
  if (result != Result::Ok) showFailure(result);
}

void PlayerInterface::finishJob() {
  const Result result = bank_.lastResult();
  if (result != Result::Ok) {
    showFailure(result);
    return;
  }

  char line[kLineSize];
  const Job finished = job_;
  job_ = Job::None;

  switch (finished) {
    case Job::Withdraw:
      snprintf(line, sizeof(line), "Paid out $%ld", static_cast<long>(paidOut_));
      showMessage(line, "Take your cash!", kDoneMs, Screen::Menu);
      break;

    case Job::Deposit:
      snprintf(line, sizeof(line), "Deposited $%ld",
               static_cast<long>(bank_.countedTotal()));
      showMessage(line, "Thank you!", kDoneMs, Screen::Menu);
      break;

    case Job::PayCounting: {
      // Player owes amount_ (e.g. $264 rent) and handed over `paid`.
      const int32_t paid = bank_.countedTotal();
      if (paid <= 0) {
        showMessage("No bills counted", "", kNoticeMs, Screen::Menu);
      } else if (paid < amount_) {
        snprintf(line, sizeof(line), "Short by $%ld",
                 static_cast<long>(amount_ - paid));
        startPayOut(paid, line, "Returning bills", Job::PayReturning);
      } else if (paid == amount_) {
        showMessage("Paid exactly!", "No change", kNoticeMs, Screen::Menu);
      } else {
        snprintf(line, sizeof(line), "Change: $%ld",
                 static_cast<long>(paid - amount_));
        startPayOut(paid - amount_, line, "Dispensing...", Job::PayChange);
      }
      break;
    }

    case Job::PayReturning:
      showMenu();
      break;

    case Job::PayChange:
      snprintf(line, sizeof(line), "Change: $%ld", static_cast<long>(paidOut_));
      showMessage(line, "Take your cash!", kDoneMs, Screen::Menu);
      break;

    case Job::PassGo:
      showMessage("PASS GO!", "Take your $200", kNoticeMs, Screen::Menu);
      break;

    case Job::None:
      showMenu();
      break;
  }
}

const char* PlayerInterface::modeLabel() const {
  return mode_ == Mode::Withdraw ? "Withdraw" : "Pay";
}
