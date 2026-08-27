#ifndef INTERVALPOLICY_H
#define INTERVALPOLICY_H

// Pure, hardware-free validation for persisted/interactively-set intervals
// (e.g. the EEPROM-backed sensor update interval). Extracted so it can be
// unit-tested natively without EEPROM/board access - see
// EepromManager::readUpdateSensorDataInterval for the call site.
namespace IntervalPolicy
{
inline bool isValid(long value, long minExclusive, long maxInclusive)
{
  return value > minExclusive && value <= maxInclusive;
}
} // namespace IntervalPolicy

#endif // INTERVALPOLICY_H
