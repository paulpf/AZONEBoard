// OtaManager.h

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <ArduinoOTA.h>

class OtaManager
{
private:
    void setupOta();

public:
    OtaManager();
    void setup();
    void handle();
};

#endif // OTA_MANAGER_H