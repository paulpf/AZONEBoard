#include "EepromManager.h"
#include "trace.h"
#include "intervalpolicy.h"

void EepromManager::writeUpdateSensorDataInterval(int dataUpdateTime)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(0, dataUpdateTime);
    EEPROM.commit();
    EEPROM.end();
}

int EepromManager::readUpdateSensorDataInterval(int defaultUpdateSensorDataInterval)
{
    EEPROM.begin(EEPROM_SIZE);
    int dataUpdateTime;
    EEPROM.get(0, dataUpdateTime);
    EEPROM.end();
    if (!IntervalPolicy::isValid(dataUpdateTime, EEPROM_SENSOR_INTERVAL_MIN_MS, EEPROM_SENSOR_INTERVAL_MAX_MS))
    {
        Trace::logf(TraceLevel::WARNING,
                    "EEPROM interval %d out of bounds, using default %d",
                    dataUpdateTime, defaultUpdateSensorDataInterval);
        dataUpdateTime = defaultUpdateSensorDataInterval;
        writeUpdateSensorDataInterval(dataUpdateTime);
    }
    return dataUpdateTime;
}
