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

// MQTT reconnect timing (see domain/mqttsessionmanager.h) - fixed retry
// interval with a bounded attempt counter that resets instead of giving
// up, since losing MQTT connectivity should never stop the device
// (sensors/OTA/webserver keep working regardless).
constexpr uint32_t MQTT_RETRY_INTERVAL_MS = 5000;
constexpr int MQTT_MAX_RECONNECT_ATTEMPTS = 5;

// MQTT topic suffixes: WifiManager::getDeviceName() (MAC-based, unique per
// board) + suffix = full topic. Kept as suffix-only macros rather than a
// DEVICE_NAME "/x" compile-time concatenation, because the device name is a
// runtime value here (a fleet of boards must not share one MQTT
// client-id/topic namespace) - see Publishers/Mqtt/MqttPublisher.cpp.
#define MQTT_TOPIC_SUFFIX_TEMPERATURE          "/sensor/temperature"
#define MQTT_TOPIC_SUFFIX_HUMIDITY             "/sensor/humidity"
#define MQTT_TOPIC_SUFFIX_TVOC                 "/sensor/tvoc"
#define MQTT_TOPIC_SUFFIX_CO2                  "/sensor/co2"
#define MQTT_TOPIC_SUFFIX_RAW_ETHANOL          "/sensor/rawEthanol"
#define MQTT_TOPIC_SUFFIX_RAW_H2               "/sensor/rawH2"
#define MQTT_TOPIC_SUFFIX_LIGHT_LEVEL           "/sensor/lightLevel"
#define MQTT_TOPIC_SUFFIX_ERRORS               "/sensor/errors"
#define MQTT_TOPIC_SUFFIX_SENSOR_INTERVAL_MS    "/config/sensorUpdateIntervalMs"     // state (retained, device publishes)
#define MQTT_TOPIC_SUFFIX_SENSOR_INTERVAL_SET   "/config/sensorUpdateIntervalMs/set" // command (write here to change it)
#define MQTT_TOPIC_SUFFIX_RSSI                 "/system/rssi"
#define MQTT_TOPIC_SUFFIX_IP                   "/system/ip"
#define MQTT_TOPIC_SUFFIX_HEALTH               "/system/health" // JSON: sensor + system diagnostics
// "/system/status" is the LWT topic (online/offline), built internally by
// MqttManager from the client id - see infrastructure/mqttmanager.cpp.
constexpr uint32_t MQTT_RSSI_INTERVAL_MS = 5000;

// OTA configuration
#define ENABLE_OTA true
constexpr uint16_t OTA_PORT = 8266;
// Security default: fail closed if no OTA password is configured.
#define OTA_ALLOW_INSECURE_NO_PASSWORD false

// EEPROM layout (see EepromManager)
constexpr int EEPROM_SIZE = 512;
constexpr int EEPROM_ADDR_SENSOR_INTERVAL = 0; // int (4 bytes)
constexpr int EEPROM_ADDR_SGP30_BASELINE = 8;  // magic + eco2Base + tvocBase (6 bytes)
constexpr long EEPROM_SENSOR_INTERVAL_MIN_MS = 0;
constexpr long EEPROM_SENSOR_INTERVAL_MAX_MS = 1000000;

// SGP30 (see SensorManager). Sensirion's datasheet requires IAQmeasure() to
// be called at a fixed 1Hz cadence for the on-chip dynamic baseline
// compensation algorithm to work correctly - independent of how often
// sensor data is actually published (SENSOR_UPDATE_INTERVAL_DEFAULT_MS,
// runtime-configurable, is unrelated to this).
constexpr uint32_t SGP30_MEASURE_INTERVAL_MS = 1000UL; // 1s, required by the sensor

// SGP30 eCO2/TVOC baseline persistence. The on-chip IAQ algorithm needs a
// burn-in period before its internal baseline is trustworthy; only start
// persisting after that, then re-save periodically so a reboot doesn't lose
// progress (values per Adafruit's SGP30 example).
constexpr uint32_t SGP30_BASELINE_BURN_IN_MS = 43200000UL;      // 12h
constexpr uint32_t SGP30_BASELINE_SAVE_INTERVAL_MS = 3600000UL; // 1h

// Sensor update timing (persisted default; actual value may be overridden
// at runtime via EEPROM/MQTT, see SystemConfig).
constexpr unsigned long SENSOR_UPDATE_INTERVAL_DEFAULT_MS = 5000;

// Common data (IP address, current sensor interval) publish timing.
constexpr unsigned long COMMON_DATA_INTERVAL_MS = 60000UL * 5UL;

// How long Application waits for the initial WiFi connection during
// startup before proceeding non-blocking. WifiManager keeps retrying in
// the background regardless of this timeout.
constexpr uint32_t WIFI_INITIAL_CONNECT_TIMEOUT_MS = 15000;

#endif // CONFIG_H
