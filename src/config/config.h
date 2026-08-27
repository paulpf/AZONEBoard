// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Serial configuration
#define SERIAL_BAUD_RATE 115200

// Trace level for logging (see infrastructure/trace.h)
#define TRACE_LEVEL TraceLevel::INFO

// Device identity: WifiManager appends the MAC address (colons stripped)
// to this prefix, e.g. "AZ-ONEBoard/A1B2C3D4E5F6". Used as WiFi hostname,
// OTA hostname and MQTT topic/client-id prefix.
#define DEVICE_NAME_PREFIX "AZ-ONEBoard/"

// WiFi configuration
constexpr int WIFI_CONNECTION_TIMEOUT = 10000; // ms, per single connect attempt
constexpr uint8_t WIFI_MAX_RECONNECT_ATTEMPTS = 10;
constexpr uint32_t WIFI_RECONNECT_BASE_DELAY_MS = 1000;
constexpr uint32_t WIFI_RECONNECT_MAX_DELAY_MS = 30000;
constexpr uint32_t WIFI_RECONNECT_JITTER_MS = 500;
// Keep device alive for diagnostics instead of forced reboot once the
// reconnect budget (WIFI_MAX_RECONNECT_ATTEMPTS) is exhausted.
#define WIFI_RESTART_ON_RECONNECT_FAILURE false

// OTA configuration
#define ENABLE_OTA true
constexpr uint16_t OTA_PORT = 8266;
// Security default: fail closed if no OTA password is configured.
#define OTA_ALLOW_INSECURE_NO_PASSWORD false

// EEPROM layout (sensor update interval, see EepromManager)
constexpr int EEPROM_SIZE = 512;
constexpr long EEPROM_SENSOR_INTERVAL_MIN_MS = 0;
constexpr long EEPROM_SENSOR_INTERVAL_MAX_MS = 1000000;

#endif // CONFIG_H
