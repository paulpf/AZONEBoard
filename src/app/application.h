#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "mqttmanager.h"
#include "systemconfig.h"
#include "connectivitycoordinator.h"
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
              MqttManager &mqttManager, WebserverPublisher &webserverPublisher,
              ConnectivityCoordinator &connectivityCoordinator,
              SystemConfig &systemConfig);

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
  void publishHealth();
  void handleMqttMessage(char *topic, uint8_t *payload, unsigned int length);

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  EepromManager &_eepromManager;
  SensorManager &_sensorManager;
  SerialPublisher &_serialPublisher;
  MqttPublisher &_mqttPublisher;
  MqttManager &_mqttManager;
  WebserverPublisher &_webserverPublisher;
  ConnectivityCoordinator &_connectivityCoordinator;
  SystemConfig &_systemConfig;

  // Persistent storage for the "config/.../set" topic string passed to
  // MqttManager::subscribe(): that call only stores the raw char* pointer,
  // so the underlying String must outlive it (deviceName is a runtime MAC-
  // based value, not a compile-time literal - see config.h).
  String _sensorIntervalSetTopic;

  unsigned long _lastSensorUpdateTime = 0;
  unsigned long _lastCommonDataUpdateTime = 0;
  unsigned long _lastRssiPublishTime = 0;
  unsigned long _startupWaitStart = 0;
  StartupState _startupState = StartupState::WAITING_FOR_WIFI;
  bool _otaInitialized = false;
  bool _initialCommonDataPublished = false;
  bool _sensorReady = false;
};

#endif // APPLICATION_H
