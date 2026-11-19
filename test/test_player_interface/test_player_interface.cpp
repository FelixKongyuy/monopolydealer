// Unit tests for PlayerInterface. Run on a laptop with:  pio test -e native
// Each test builds the UI with fake parts, presses keys, and checks the screen
// and what was asked of the bank.

#include <unity.h>

#include "../mocks/FakeParts.h"
#include "PlayerInterface.h"

namespace {

using Screen = PlayerInterface::Screen;

// One UI wired to fresh fakes, already past the start-up message.
struct Rig {
  FakeKeypad keypad;
  FakeDisplay display;
  FakeButton button;
  FakeBank bank;
  PlayerInterface ui{keypad, display, button, bank};
  uint32_t now = 0;

  Rig() {
    ui.begin(now);
    tick(PlayerInterface::kSplashMs);
  }

  void tick(uint32_t ms) {
    now += ms;
    ui.update(now);
  }

  void press(char key) {
    keypad.press(key);
    tick(10);
  }

  void type(const char* keys) {
    for (const char* k = keys; *k != '\0'; ++k) press(*k);
  }

  const char* line(uint8_t row) const { return display.line(row); }
};

}  // namespace

void setUp() {}
void tearDown() {}

void test_starts_on_menu_after_splash() {
  Rig r;
  TEST_ASSERT_TRUE(r.ui.screen() == Screen::Menu);
  TEST_ASSERT_EQUAL_STRING("A:Withdraw B:Dep", r.line(0));
  TEST_ASSERT_EQUAL_STRING("C:Pay  GO=$200", r.line(1));
}

void test_typing_ignores_leading_zero_caps_digits_and_deletes() {
  Rig r;
  r.press('A');
  TEST_ASSERT_EQUAL_STRING("Withdraw $_", r.line(0));

  r.type("0123");
  TEST_ASSERT_EQUAL_STRING("Withdraw $123", r.line(0));

  r.type("45");  // fifth digit is ignored
  TEST_ASSERT_EQUAL_STRING("Withdraw $1234", r.line(0));

  r.press('*');
  TEST_ASSERT_EQUAL_STRING("Withdraw $123", r.line(0));
}

void test_enter_without_amount_shows_hint_then_returns() {
  Rig r;
  r.type("A#");
  TEST_ASSERT_EQUAL_STRING("Enter an amount", r.line(0));

  r.tick(PlayerInterface::kHintMs);
  TEST_ASSERT_EQUAL_STRING("Withdraw $_", r.line(0));
}

void test_back_from_confirm_keeps_amount() {
  Rig r;
  r.type("A75#");
  TEST_ASSERT_EQUAL_STRING("Withdraw $75?", r.line(0));

  r.press('D');
  TEST_ASSERT_EQUAL_STRING("Withdraw $75", r.line(0));
}

void test_withdraw_pays_out_then_returns_to_menu() {
  Rig r;
  r.type("A264#");
  TEST_ASSERT_EQUAL_STRING("Withdraw $264?", r.line(0));

  r.press('#');
  TEST_ASSERT_EQUAL_INT(1, r.bank.payOutCount);
  TEST_ASSERT_EQUAL_INT32(264, r.bank.lastPayOut);
  TEST_ASSERT_TRUE(r.ui.screen() == Screen::Busy);

  r.bank.finish();
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("Paid out $264", r.line(0));
  TEST_ASSERT_EQUAL_STRING("Take your cash!", r.line(1));

  r.tick(PlayerInterface::kDoneMs);
  TEST_ASSERT_TRUE(r.ui.screen() == Screen::Menu);
}

void test_withdraw_more_than_bank_holds_is_refused() {
  Rig r;
  r.bank.stock = 100;
  r.type("A264##");
  TEST_ASSERT_EQUAL_STRING("Not enough bills", r.line(0));
  TEST_ASSERT_EQUAL_INT(0, r.bank.payOutCount);
}

void test_jam_shows_error() {
  Rig r;
  r.type("A50##");
  r.bank.finish(Result::Jam);
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("ERROR: bill jam", r.line(0));
}

void test_deposit_shows_counted_total() {
  Rig r;
  r.press('B');
  TEST_ASSERT_EQUAL_INT(1, r.bank.countingCount);
  TEST_ASSERT_EQUAL_STRING("Insert bills", r.line(0));

  r.bank.finishCounting(350);
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("Deposited $350", r.line(0));
}

// The key test case from the deck: pay $500 for $264 rent, get $236 back.
void test_pay_500_for_264_gives_236_change() {
  Rig r;
  r.type("C264#");
  TEST_ASSERT_EQUAL_STRING("Pay $264?", r.line(0));

  r.press('#');
  TEST_ASSERT_EQUAL_INT(1, r.bank.countingCount);

  r.bank.finishCounting(500);
  r.tick(10);
  TEST_ASSERT_EQUAL_INT32(236, r.bank.lastPayOut);
  TEST_ASSERT_EQUAL_STRING("Change: $236", r.line(0));

  r.bank.finish();
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("Take your cash!", r.line(1));
}

void test_pay_exact_gives_no_change() {
  Rig r;
  r.type("C264##");
  r.bank.finishCounting(264);
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("Paid exactly!", r.line(0));
  TEST_ASSERT_EQUAL_INT(0, r.bank.payOutCount);
}

void test_pay_short_returns_the_bills() {
  Rig r;
  r.type("C264##");
  r.bank.finishCounting(200);
  r.tick(10);
  TEST_ASSERT_EQUAL_INT32(200, r.bank.lastPayOut);
  TEST_ASSERT_EQUAL_STRING("Short by $64", r.line(0));
  TEST_ASSERT_EQUAL_STRING("Returning bills", r.line(1));

  r.bank.finish();
  r.tick(10);
  TEST_ASSERT_TRUE(r.ui.screen() == Screen::Menu);
}

void test_pay_with_no_bills_counted() {
  Rig r;
  r.type("C264##");
  r.bank.finishCounting(0);
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("No bills counted", r.line(0));
  TEST_ASSERT_EQUAL_INT(0, r.bank.payOutCount);
}

void test_pass_go_from_menu_pays_200() {
  Rig r;
  r.button.press();
  r.tick(10);
  TEST_ASSERT_EQUAL_INT32(200, r.bank.lastPayOut);
  TEST_ASSERT_EQUAL_STRING("PASS GO!", r.line(0));

  r.bank.finish();
  r.tick(10);
  TEST_ASSERT_EQUAL_STRING("Take your $200", r.line(1));
}

void test_pass_go_ignored_while_typing() {
  Rig r;
  r.press('A');
  r.button.press();
  r.tick(10);
  TEST_ASSERT_EQUAL_INT(0, r.bank.payOutCount);
  TEST_ASSERT_TRUE(r.ui.screen() == Screen::Typing);

  // The press was dropped, so going back to the menu doesn't pay out late.
  r.press('D');
  r.tick(10);
  TEST_ASSERT_EQUAL_INT(0, r.bank.payOutCount);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_starts_on_menu_after_splash);
  RUN_TEST(test_typing_ignores_leading_zero_caps_digits_and_deletes);
  RUN_TEST(test_enter_without_amount_shows_hint_then_returns);
  RUN_TEST(test_back_from_confirm_keeps_amount);
  RUN_TEST(test_withdraw_pays_out_then_returns_to_menu);
  RUN_TEST(test_withdraw_more_than_bank_holds_is_refused);
  RUN_TEST(test_jam_shows_error);
  RUN_TEST(test_deposit_shows_counted_total);
  RUN_TEST(test_pay_500_for_264_gives_236_change);
  RUN_TEST(test_pay_exact_gives_no_change);
  RUN_TEST(test_pay_short_returns_the_bills);
  RUN_TEST(test_pay_with_no_bills_counted);
  RUN_TEST(test_pass_go_from_menu_pays_200);
  RUN_TEST(test_pass_go_ignored_while_typing);
  return UNITY_END();
}
