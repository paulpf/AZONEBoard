// systemconfig.h
#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include "config.h"

// Injectable, mutable runtime configuration - as opposed to config.h's
// compile-time constants. Owned by Bootstrap, referenced by Application.
struct SystemConfig
{
  // Sensor read/publish interval. Mutable at runtime (via MQTT) and
  // persisted to EEPROM by Application; initialized from EEPROM (or the
  // default) at startup.
  unsigned long sensorUpdateIntervalMs = SENSOR_UPDATE_INTERVAL_DEFAULT_MS;

  // How often common/device data (IP address, current sensor interval) is
  // (re-)published, independent of the sensor interval.
  unsigned long commonDataIntervalMs = COMMON_DATA_INTERVAL_MS;
};

#endif // SYSTEMCONFIG_H
