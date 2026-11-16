// Unit tests for Money.h. Run on a laptop with:  pio test -e native
// This file is also the template for Aung's ChangeMaker and Ledger tests.

#include <unity.h>

#include "Money.h"

void setUp() {}
void tearDown() {}

void test_seven_denominations_lowest_to_highest() {
  TEST_ASSERT_EQUAL_UINT8(7, kDenomCount);
  for (uint8_t i = 1; i < kDenomCount; ++i) {
    TEST_ASSERT_TRUE(kDenomValue[i - 1] < kDenomValue[i]);
  }
}

void test_value_of_each_denomination() {
  TEST_ASSERT_EQUAL_UINT16(1, valueOf(Denom::D1));
  TEST_ASSERT_EQUAL_UINT16(5, valueOf(Denom::D5));
  TEST_ASSERT_EQUAL_UINT16(10, valueOf(Denom::D10));
  TEST_ASSERT_EQUAL_UINT16(20, valueOf(Denom::D20));
  TEST_ASSERT_EQUAL_UINT16(50, valueOf(Denom::D50));
  TEST_ASSERT_EQUAL_UINT16(100, valueOf(Denom::D100));
  TEST_ASSERT_EQUAL_UINT16(500, valueOf(Denom::D500));
}

void test_unknown_is_not_valid() {
  TEST_ASSERT_FALSE(isValid(Denom::Unknown));
  TEST_ASSERT_FALSE(isValid(Denom::Count));
  TEST_ASSERT_TRUE(isValid(Denom::D500));
}

void test_total_value_of_full_carousel() {
  BillCounts full = emptyCounts();
  for (uint8_t i = 0; i < kDenomCount; ++i) {
    full.count[i] = kSlotCapacity;
  }
  // 45 x (1 + 5 + 10 + 20 + 50 + 100 + 500) = 45 x 686
  TEST_ASSERT_EQUAL_INT32(30870, totalValue(full));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_seven_denominations_lowest_to_highest);
  RUN_TEST(test_value_of_each_denomination);
  RUN_TEST(test_unknown_is_not_valid);
  RUN_TEST(test_total_value_of_full_carousel);
  return UNITY_END();
}
