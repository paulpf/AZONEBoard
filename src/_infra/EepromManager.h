#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <EEPROM.h>

class EepromManager
{
public:
    void writeUpdateSensorDataInterval(int dataUpdateTime);
    int readUpdateSensorDataInterval(int defaultUpdateSensorDataInterval);
};

#endif // EEPROM_MANAGER_H