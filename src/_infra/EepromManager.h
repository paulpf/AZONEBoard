#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <EEPROM.h>
#include <stdint.h>

class EepromManager
{
public:
    void writeUpdateSensorDataInterval(int dataUpdateTime);
    int readUpdateSensorDataInterval(int defaultUpdateSensorDataInterval);

    void writeSgp30Baseline(uint16_t eco2Base, uint16_t tvocBase);
    // Returns false (and leaves eco2Base/tvocBase untouched) if no valid
    // baseline was ever persisted.
    bool readSgp30Baseline(uint16_t &eco2Base, uint16_t &tvocBase);
};

#endif // EEPROM_MANAGER_H