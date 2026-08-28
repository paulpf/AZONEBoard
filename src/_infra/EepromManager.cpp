#include "EepromManager.h"
#include "trace.h"
#include "intervalpolicy.h"
#include "config.h"

namespace
{
// Marks a written SGP30 baseline record as valid, distinguishing it from
// erased/uninitialized EEPROM or unrelated leftover data.
constexpr uint16_t SGP30_BASELINE_MAGIC = 0xB157;

struct Sgp30BaselineRecord
{
    uint16_t magic;
    uint16_t eco2Base;
    uint16_t tvocBase;
};
} // namespace

void EepromManager::writeUpdateSensorDataInterval(int dataUpdateTime)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_ADDR_SENSOR_INTERVAL, dataUpdateTime);
    EEPROM.commit();
    EEPROM.end();
}

int EepromManager::readUpdateSensorDataInterval(int defaultUpdateSensorDataInterval)
{
    EEPROM.begin(EEPROM_SIZE);
    int dataUpdateTime;
    EEPROM.get(EEPROM_ADDR_SENSOR_INTERVAL, dataUpdateTime);
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

void EepromManager::writeSgp30Baseline(uint16_t eco2Base, uint16_t tvocBase)
{
    Sgp30BaselineRecord record{SGP30_BASELINE_MAGIC, eco2Base, tvocBase};
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_ADDR_SGP30_BASELINE, record);
    EEPROM.commit();
    EEPROM.end();
}

bool EepromManager::readSgp30Baseline(uint16_t &eco2Base, uint16_t &tvocBase)
{
    Sgp30BaselineRecord record;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(EEPROM_ADDR_SGP30_BASELINE, record);
    EEPROM.end();
    if (record.magic != SGP30_BASELINE_MAGIC)
    {
        return false;
    }
    eco2Base = record.eco2Base;
    tvocBase = record.tvocBase;
    return true;
}
