#include "application.h"
#include "trace.h"
#include "../_structures/CommonData.h"

// External secrets - located outside this project in ../_secrets/
// Include path is set via build_flags in platformio.ini: -I ../_secrets
#include "WifiSecret.h"
#include "OtaSecret.h"

Application *Application::_instance = nullptr;

Application::Application(WifiManager &wifiManager, OtaManager &otaManager,
                         EepromManager &eepromManager, SensorManager &sensorManager,
                         SerialPublisher &serialPublisher, MqttPublisher &mqttPublisher,
                         WebserverPublisher &webserverPublisher, SystemConfig &systemConfig)
    : _wifiManager(wifiManager), _otaManager(otaManager),
      _eepromManager(eepromManager), _sensorManager(sensorManager),
      _serialPublisher(serialPublisher), _mqttPublisher(mqttPublisher),
      _webserverPublisher(webserverPublisher), _systemConfig(systemConfig)
{
  _instance = this;
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
  _wifiManager.setup(ssid, password, DEVICE_NAME_PREFIX);

  // The WiFiClient handle and device name are available synchronously
  // right after setup(), independent of whether the connection has
  // actually completed yet - PubSubClient/ESP8266WebServer only need the
  // handle, they manage their own connection state.
  WiFiClient *wifiClient = _wifiManager.getWifiClient();
  String deviceName = _wifiManager.getDeviceName();
  _mqttPublisher.setup(wifiClient, deviceName);
  _mqttPublisher.registerCallback(&Application::sensorUpdateIntervalTrampoline);
  _webserverPublisher.setup(deviceName);

  if (!_sensorManager.setup())
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
  _wifiManager.loop();
  _otaManager.loop();

  if (_wifiManager.consumeConnectedEvent())
  {
    Trace::log(TraceLevel::INFO, "WiFi connected event");
  }
  if (_wifiManager.consumeDisconnectedEvent())
  {
    Trace::log(TraceLevel::WARNING, "WiFi disconnected event");
  }

  if (_wifiManager.isConnected())
  {
    // Initialize OTA the first time WiFi becomes available - not only
    // during the initial startup window - so a slow first connect doesn't
    // permanently skip OTA for the rest of the device's uptime.
    if (!_otaInitialized)
    {
      String deviceName = _wifiManager.getDeviceName();
      _otaManager.setup(deviceName.c_str(), ota_password);
      _otaInitialized = true;
    }

    // Publish common data (IP address, current sensor interval) once as
    // soon as we are connected, then periodically. Both branches are
    // guarded by isConnected() so this never runs while WiFi is known to
    // be down - MqttPublisher's own reconnect handling can block for a
    // while if it is invoked without connectivity (see Phase 6 notes in
    // docs/TEMPLATE_MIGRATION_PLAN.md).
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

  // Update sensor data if the interval has passed. Runs regardless of
  // WiFi state: SerialPublisher and WebserverPublisher don't need it.
  if (currentTime - _lastSensorUpdateTime >= _systemConfig.sensorUpdateIntervalMs)
  {
    _lastSensorUpdateTime = currentTime;
    _sensorManager.updateSensorData();
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

void Application::onSensorUpdateIntervalChanged(int newValue)
{
  _systemConfig.sensorUpdateIntervalMs = newValue;
  _eepromManager.writeUpdateSensorDataInterval(newValue);
}

void Application::sensorUpdateIntervalTrampoline(int newValue)
{
  if (_instance != nullptr)
  {
    _instance->onSensorUpdateIntervalChanged(newValue);
  }
}
