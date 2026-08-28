#include <unity.h>
#include "intervalpolicy.h"

// Mirrors EepromManager::readUpdateSensorDataInterval's bounds check
// (value > 0 && value <= 1000000), extracted as IntervalPolicy::isValid
// so it is testable without EEPROM/board access.

void test_value_within_bounds_is_valid()
{
  TEST_ASSERT_TRUE(IntervalPolicy::isValid(5000, 0, 1000000));
}

void test_zero_is_invalid()
{
  TEST_ASSERT_FALSE(IntervalPolicy::isValid(0, 0, 1000000));
}

void test_negative_is_invalid()
{
  TEST_ASSERT_FALSE(IntervalPolicy::isValid(-1, 0, 1000000));
}

void test_value_at_max_is_valid()
{
  TEST_ASSERT_TRUE(IntervalPolicy::isValid(1000000, 0, 1000000));
}

void test_value_above_max_is_invalid()
{
  TEST_ASSERT_FALSE(IntervalPolicy::isValid(1000001, 0, 1000000));
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(test_value_within_bounds_is_valid);
  RUN_TEST(test_zero_is_invalid);
  RUN_TEST(test_negative_is_invalid);
  RUN_TEST(test_value_at_max_is_valid);
  RUN_TEST(test_value_above_max_is_invalid);
  return UNITY_END();
}
