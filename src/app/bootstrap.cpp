#include "bootstrap.h"

Bootstrap::Bootstrap()
    : _sensorManager(_eepromManager),
      _mqttPublisher(_mqttManager),
      _connectivityCoordinator(_wifiManager, _mqttManager),
      _systemConfig(),
      _app(_wifiManager, _otaManager, _eepromManager, _sensorManager,
           _serialPublisher, _mqttPublisher, _mqttManager, _webserverPublisher,
           _connectivityCoordinator, _systemConfig)
{
}

Application &Bootstrap::application()
{
  return _app;
}
