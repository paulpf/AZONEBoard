#ifndef OTAMANAGER_H
#define OTAMANAGER_H

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

#endif // OTAMANAGER_H