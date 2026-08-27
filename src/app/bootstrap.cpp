#include "bootstrap.h"

Bootstrap::Bootstrap()
    : _systemConfig(),
      _app(_wifiManager, _otaManager, _eepromManager, _sensorManager,
           _serialPublisher, _mqttPublisher, _webserverPublisher, _systemConfig)
{
}

Application &Bootstrap::application()
{
  return _app;
}
