#ifndef RECONNECTPOLICY_H
#define RECONNECTPOLICY_H

#include <stdint.h>

namespace ReconnectPolicy
{
inline uint32_t computeDelayMs(uint8_t attempt, uint32_t baseDelayMs,
                               uint32_t maxDelayMs, uint32_t jitterMs)
{
  // Exponential backoff with capped exponent:
  // - attempt 1 -> base * 2
  // - attempt 2 -> base * 4
  // ...
  // The shift is limited to keep values bounded and deterministic.
  const uint8_t shiftSteps = (attempt < 5U) ? attempt : 5U;
  uint32_t delayMs = baseDelayMs << shiftSteps;
  // Hard cap ensures reconnect delay never exceeds configured maximum.
  if (delayMs > maxDelayMs)
  {
    delayMs = maxDelayMs;
  }

  // Jitter is added by caller to decorrelate retries across device fleet.
  return delayMs + jitterMs;
}
} // namespace ReconnectPolicy

#endif // RECONNECTPOLICY_H
