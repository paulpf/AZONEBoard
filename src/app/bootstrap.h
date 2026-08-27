#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "application.h"
#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "../_infra/EepromManager.h"
#include "../Sensors/SensorManager.h"
#include "../Publishers/Serial/SerialPublisher.h"
#include "../Publishers/Mqtt/MqttPublisher.h"
#include "../Publishers/Webserver/WebserverPublisher.h"

// Composition root: owns every concrete instance and wires them into
// Application via constructor injection. main.cpp only ever talks to
// Bootstrap and the Application it hands back.
class Bootstrap
{
public:
  Bootstrap();
  Application &application();

private:
  WifiManager _wifiManager;
  OtaManager _otaManager;
  EepromManager _eepromManager;
  SensorManager _sensorManager;
  SerialPublisher _serialPublisher;
  MqttPublisher _mqttPublisher;
  WebserverPublisher _webserverPublisher;
  SystemConfig _systemConfig;
  Application _app;
};

#endif // BOOTSTRAP_H
