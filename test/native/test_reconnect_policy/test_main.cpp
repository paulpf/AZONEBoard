#include <unity.h>
#include "reconnectpolicy.h"

void test_backoff_increases_with_attempts_without_jitter()
{
  const uint32_t base = 1000;
  const uint32_t maxDelay = 30000;

  const uint32_t d1 = ReconnectPolicy::computeDelayMs(1, base, maxDelay, 0);
  const uint32_t d2 = ReconnectPolicy::computeDelayMs(2, base, maxDelay, 0);
  const uint32_t d3 = ReconnectPolicy::computeDelayMs(3, base, maxDelay, 0);

  TEST_ASSERT_TRUE(d2 > d1);
  TEST_ASSERT_TRUE(d3 > d2);
}

void test_backoff_caps_at_max_delay()
{
  const uint32_t base = 1000;
  const uint32_t maxDelay = 30000;
  const uint32_t jitter = 0;

  const uint32_t delay = ReconnectPolicy::computeDelayMs(20, base, maxDelay, jitter);
  TEST_ASSERT_EQUAL_UINT32(maxDelay, delay);
}

void test_jitter_is_added()
{
  const uint32_t base = 1000;
  const uint32_t maxDelay = 30000;
  const uint32_t jitter = 123;

  const uint32_t delay = ReconnectPolicy::computeDelayMs(1, base, maxDelay, jitter);
  TEST_ASSERT_EQUAL_UINT32(2123, delay);
}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(test_backoff_increases_with_attempts_without_jitter);
  RUN_TEST(test_backoff_caps_at_max_delay);
  RUN_TEST(test_jitter_is_added);
  return UNITY_END();
}
