#include "EepromManager.h"

void EepromManager::writeUpdateSensorDataInterval(int dataUpdateTime)
{
    EEPROM.begin(512);
    EEPROM.put(0, dataUpdateTime);
    EEPROM.commit();
    EEPROM.end();
}

int EepromManager::readUpdateSensorDataInterval(int defaultUpdateSensorDataInterval)
{
    EEPROM.begin(512);
    int dataUpdateTime;
    EEPROM.get(0, dataUpdateTime);
    EEPROM.end();
    if (dataUpdateTime <= 0 || dataUpdateTime > 1000000)
    {
        dataUpdateTime = defaultUpdateSensorDataInterval;
        writeUpdateSensorDataInterval(dataUpdateTime);
    }
    return dataUpdateTime;
}