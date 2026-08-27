#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "../_infra/EepromManager.h"
#include "../Sensors/SensorManager.h"
#include "../Publishers/Serial/SerialPublisher.h"
#include "../Publishers/Mqtt/MqttPublisher.h"
#include "../Publishers/Webserver/WebserverPublisher.h"

// Orchestrates startup sequencing and the main loop's timing. Owns no
// hardware itself - everything is injected by Bootstrap (composition
// root), so this class stays testable/readable in isolation.
class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager,
              EepromManager &eepromManager, SensorManager &sensorManager,
              SerialPublisher &serialPublisher, MqttPublisher &mqttPublisher,
              WebserverPublisher &webserverPublisher, SystemConfig &systemConfig);

  void setup();
  void loop();

private:
  enum class StartupState
  {
    WAITING_FOR_WIFI,
    RUNNING
  };

  void handleStartup();
  void publishCommonData();
  void onSensorUpdateIntervalChanged(int newValue);

  // MqttPublisher::registerCallback() only accepts a plain function
  // pointer (see _interfaces/delegates.h), so a static trampoline bridges
  // back to the single Application instance - the same pattern already
  // used by MqttPublisher/SerialPublisher/WebserverPublisher for their
  // own static callbacks.
  static void sensorUpdateIntervalTrampoline(int newValue);
  static Application *_instance;

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  EepromManager &_eepromManager;
  SensorManager &_sensorManager;
  SerialPublisher &_serialPublisher;
  MqttPublisher &_mqttPublisher;
  WebserverPublisher &_webserverPublisher;
  SystemConfig &_systemConfig;

  unsigned long _lastSensorUpdateTime = 0;
  unsigned long _lastCommonDataUpdateTime = 0;
  unsigned long _startupWaitStart = 0;
  StartupState _startupState = StartupState::WAITING_FOR_WIFI;
  bool _otaInitialized = false;
  bool _initialCommonDataPublished = false;
};

#endif // APPLICATION_H
