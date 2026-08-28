#include "application.h"
#include "trace.h"
#include "../_structures/CommonData.h"
#include <ESP8266WiFi.h>
#include <cstring>
#include <cstdlib>

// External secrets - located outside this project in ../_secrets/
// Include path is set via build_flags in platformio.ini: -I ../_secrets
#include "WifiSecret.h"
#include "OtaSecret.h"
#include "MqttSecret.h"
#include "MqttConfig.h"

Application::Application(WifiManager &wifiManager, OtaManager &otaManager,
                         EepromManager &eepromManager, SensorManager &sensorManager,
                         SerialPublisher &serialPublisher, MqttPublisher &mqttPublisher,
                         MqttManager &mqttManager, WebserverPublisher &webserverPublisher,
                         ConnectivityCoordinator &connectivityCoordinator,
                         SystemConfig &systemConfig)
    : _wifiManager(wifiManager), _otaManager(otaManager),
      _eepromManager(eepromManager), _sensorManager(sensorManager),
      _serialPublisher(serialPublisher), _mqttPublisher(mqttPublisher),
      _mqttManager(mqttManager), _webserverPublisher(webserverPublisher),
      _connectivityCoordinator(connectivityCoordinator), _systemConfig(systemConfig)
{
}

void Application::setup()
{
  // SerialPublisher owns Serial.begin() - do this first so Trace, which
  // writes to Serial, works for everything that follows.
  _serialPublisher.setup();

  Trace::log(TraceLevel::INFO, "Application setup started");

  // Restore the persisted sensor update interval before anything reads it.
  _systemConfig.sensorUpdateIntervalMs = _eepromManager.readUpdateSensorDataInterval(
      static_cast<int>(SENSOR_UPDATE_INTERVAL_DEFAULT_MS));

  // WiFi connect is non-blocking from here on; WifiManager::loop() (called
  // every loop() iteration below) drives connection state and reconnects.
  _wifiManager.setup(WIFI_SSID, WIFI_PWD, DEVICE_NAME_PREFIX);

  // The device name is available synchronously right after setup(),
  // independent of whether the connection has actually completed yet.
  String deviceName = _wifiManager.getDeviceName();

  _mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, deviceName);
  _mqttManager.setCallback([this](char *topic, uint8_t *payload, unsigned int length) {
    handleMqttMessage(topic, payload, length);
  });
  // MqttManager::subscribe() only stores the char* pointer, so the backing
  // String must outlive it - see application.h.
  _sensorIntervalSetTopic = deviceName + MQTT_TOPIC_SUFFIX_SENSOR_INTERVAL_SET;
  _mqttManager.subscribe(_sensorIntervalSetTopic.c_str());

  _mqttPublisher.setup(deviceName);
  _webserverPublisher.setup(deviceName);

  _sensorReady = _sensorManager.setup();
  if (!_sensorReady)
  {
    Trace::log(TraceLevel::ERROR,
               "Sensor initialization failed - continuing, sensor readings may be unreliable");
  }

  _sensorManager.registerSubscriberForUpdateSensorData(&_serialPublisher);
  _sensorManager.registerSubscriberForUpdateSensorData(&_mqttPublisher);
  _sensorManager.registerSubscriberForUpdateSensorData(&_webserverPublisher);

  _startupWaitStart = millis();
  _startupState = StartupState::WAITING_FOR_WIFI;
  Trace::log(TraceLevel::INFO, "Startup is non-blocking, waiting for WiFi in main loop");

  Trace::log(TraceLevel::INFO, "Application setup complete");
}

void Application::loop()
{
  unsigned long currentTime = millis();

  // Progress the initial-startup timeout bookkeeping every cycle.
  handleStartup();

  // Service WiFi state machine/reconnect behavior and OTA session handling.
  // WifiManager itself traces connect/disconnect transitions, so no
  // separate logging is needed here.
  _wifiManager.loop();
  _otaManager.loop();

  // Drives the SGP30's required fixed 1Hz measurement cadence - must run
  // every iteration, independent of the (much lower, configurable) sensor
  // publish interval below.
  _sensorManager.loop();

  // The sole consumer of WiFi connected/disconnected events: translates
  // them into MQTT connect-request/force-disconnect. Events are consumed
  // (cleared) on read, so nothing else may also call
  // consumeConnectedEvent()/consumeDisconnectedEvent() or they'd race for
  // the same flag.
  _connectivityCoordinator.handleEvents();
  _mqttManager.loop();

  if (_wifiManager.isConnected())
  {
    // Initialize OTA the first time WiFi becomes available - not only
    // during the initial startup window - so a slow first connect doesn't
    // permanently skip OTA for the rest of the device's uptime.
    if (!_otaInitialized)
    {
      String deviceName = _wifiManager.getDeviceName();
      _otaManager.setup(deviceName.c_str(), OTA_PASSWORD);
      _otaInitialized = true;
    }

    // Publish common data (IP address, current sensor interval) once as
    // soon as we are connected, then periodically. MqttPublisher's
    // underlying MqttManager silently skips publishing while MQTT itself
    // isn't connected yet, so this only needs to be gated on WiFi.
    if (!_initialCommonDataPublished)
    {
      publishCommonData();
      _lastCommonDataUpdateTime = currentTime;
      _initialCommonDataPublished = true;
    }
    else if (currentTime - _lastCommonDataUpdateTime >= _systemConfig.commonDataIntervalMs)
    {
      _lastCommonDataUpdateTime = currentTime;
      publishCommonData();
    }
  }

  if (_mqttManager.isConnected() &&
      currentTime - _lastRssiPublishTime >= MQTT_RSSI_INTERVAL_MS)
  {
    _lastRssiPublishTime = currentTime;
    _mqttPublisher.publishRssi(WiFi.RSSI());
  }

  // Update sensor data if the interval has passed. Runs regardless of
  // WiFi state: SerialPublisher and WebserverPublisher don't need it.
  if (currentTime - _lastSensorUpdateTime >= _systemConfig.sensorUpdateIntervalMs)
  {
    _lastSensorUpdateTime = currentTime;
    _sensorManager.updateSensorData();
    publishHealth();
  }

  _webserverPublisher.handle();
}

void Application::handleStartup()
{
  // Once startup is complete, this function becomes a fast no-op.
  if (_startupState != StartupState::WAITING_FOR_WIFI)
  {
    return;
  }

  if (_wifiManager.isConnected())
  {
    _connectivityCoordinator.ensureMqttConnected();
    _startupState = StartupState::RUNNING;
    return;
  }

  // Guard against waiting forever for the initial WiFi connection: the
  // application continues even without immediate connectivity.
  // WifiManager keeps reconnecting in the background regardless.
  if (millis() - _startupWaitStart > WIFI_INITIAL_CONNECT_TIMEOUT_MS)
  {
    Trace::log(TraceLevel::WARNING,
               "Initial WiFi connection timeout; continuing non-blocking");
    _startupState = StartupState::RUNNING;
  }
}

void Application::publishCommonData()
{
  CommonData commonData;
  commonData.updateSensorDataInterval = _systemConfig.sensorUpdateIntervalMs;
  Trace::log(TraceLevel::INFO, "Publishing common data");
  _mqttPublisher.publishCommonData(commonData);
}

void Application::publishHealth()
{
  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"sensorReady\":%s,\"wifiRssi\":%d,\"uptimeMs\":%lu,\"freeHeap\":%u,"
           "\"otaEnabled\":%s,\"otaUpdating\":%s}",
           _sensorReady ? "true" : "false",
           WiFi.RSSI(),
           millis(),
           ESP.getFreeHeap(),
           _otaManager.isEnabled() ? "true" : "false",
           _otaManager.isUpdating() ? "true" : "false");
  _mqttPublisher.publishHealth(payload);
}

void Application::handleMqttMessage(char *topic, uint8_t *payload, unsigned int length)
{
  if (strcmp(topic, _sensorIntervalSetTopic.c_str()) == 0)
  {
    char buf[16];
    unsigned int len = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
    memcpy(buf, payload, len);
    buf[len] = '\0';

    unsigned long newValue = strtoul(buf, nullptr, 10);
    if (newValue > 0)
    {
      Trace::logf(TraceLevel::INFO, "Updating sensorUpdateIntervalMs to %lu", newValue);
      _systemConfig.sensorUpdateIntervalMs = newValue;
      _eepromManager.writeUpdateSensorDataInterval(newValue);

      CommonData commonData;
      commonData.updateSensorDataInterval = _systemConfig.sensorUpdateIntervalMs;
      _mqttPublisher.publishCommonData(commonData);
    }
  }
}
