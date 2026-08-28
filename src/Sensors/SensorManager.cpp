// SensorManager.cpp

#include "SensorManager.h"
#include "./_structures/SensorData.h"
#include "trace.h"
#include "config.h"
#include <Wire.h>
#include <math.h>

// Constructor for SensorManager class
SensorManager::SensorManager(EepromManager &eepromManager)
    : sht30(0x44), sgp30(), lightMeter(), _eepromManager(eepromManager) {}

// Destructor for SensorManager class
SensorManager::~SensorManager() {}

// Setup function to initialize all sensors
bool SensorManager::setup()
{
    // Initialize I2C communication
    Wire.begin();

    // No need to call begin for SHT3X, it initializes directly

    // Initialize SGP30 sensor
    if (!sgp30.begin())
    {
        Trace::log(TraceLevel::ERROR, "Failed to initialize SGP30 sensor!");
        return false;
    }

    // Initialize BH1750 sensor
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
    {
        Trace::log(TraceLevel::ERROR, "Failed to initialize BH1750 sensor!");
        return false;
    }

    restoreSgp30Baseline();
    _setupTime = millis();

    return true;
}

// Restores a previously persisted SGP30 eCO2/TVOC baseline (if any).
void SensorManager::restoreSgp30Baseline()
{
    uint16_t eco2Base, tvocBase;
    if (_eepromManager.readSgp30Baseline(eco2Base, tvocBase))
    {
        sgp30.setIAQBaseline(eco2Base, tvocBase);
        Trace::logf(TraceLevel::INFO,
                    "Restored SGP30 baseline (eCO2=0x%04X, TVOC=0x%04X)",
                    eco2Base, tvocBase);
    }
    else
    {
        Trace::log(TraceLevel::INFO, "No persisted SGP30 baseline found; starting cold");
    }
}

// Persists the current SGP30 baseline once burn-in has passed, then
// periodically thereafter. No-op until the burn-in period elapses or
// between saves.
void SensorManager::maybeSaveSgp30Baseline()
{
    unsigned long now = millis();
    if (now - _setupTime < SGP30_BASELINE_BURN_IN_MS)
    {
        return;
    }
    if (_lastBaselineSaveTime != 0 && now - _lastBaselineSaveTime < SGP30_BASELINE_SAVE_INTERVAL_MS)
    {
        return;
    }

    uint16_t eco2Base, tvocBase;
    if (sgp30.getIAQBaseline(&eco2Base, &tvocBase))
    {
        _eepromManager.writeSgp30Baseline(eco2Base, tvocBase);
        _lastBaselineSaveTime = now;
        Trace::logf(TraceLevel::INFO,
                    "Saved SGP30 baseline (eCO2=0x%04X, TVOC=0x%04X)",
                    eco2Base, tvocBase);
    }
    else
    {
        Trace::log(TraceLevel::WARNING, "Failed to read SGP30 baseline for persistence");
    }
}

void SensorManager::loop()
{
    tickSgp30Measurement();
}

// Runs readSgp30() at the fixed cadence the sensor's on-chip baseline
// algorithm requires (SGP30_MEASURE_INTERVAL_MS), independent of
// updateSensorData()'s publish cadence.
void SensorManager::tickSgp30Measurement()
{
    unsigned long now = millis();
    if (now - _lastSgp30MeasureTime < SGP30_MEASURE_INTERVAL_MS)
    {
        return;
    }
    _lastSgp30MeasureTime = now;

    // Humidity compensation needs a real SHT30 reading; skip until one has
    // happened. Reusing the last cached reading (rather than triggering a
    // fresh SHT30 measurement here) is fine - temperature/humidity change
    // slowly relative to the 1s SGP30 cadence.
    if (_shtReadOnce)
    {
        uint32_t absoluteHumidity = calculateAbsoluteHumidity(sht30.cTemp, sht30.humidity);
        sgp30.setHumidity(absoluteHumidity);
    }

    _lastSgp30MeasureOk = readSgp30();
    if (_lastSgp30MeasureOk)
    {
        _lastEco2 = sgp30.eCO2;
        _lastTvoc = sgp30.TVOC;
    }
}

// Approximates absolute humidity [mg/m^3] from temperature [C] and relative
// humidity [%], per Sensirion's SGP30 driver integration guide (chapter
// 3.15) / Adafruit's reference SGP30 example.
uint32_t SensorManager::calculateAbsoluteHumidity(float temperatureC, float relativeHumidityPercent)
{
    float absoluteHumidityGramsPerM3 =
        216.7f * ((relativeHumidityPercent / 100.0f) * 6.112f *
                  exp((17.62f * temperatureC) / (243.12f + temperatureC)) /
                  (273.15f + temperatureC));
    return static_cast<uint32_t>(1000.0f * absoluteHumidityGramsPerM3);
}

// Method to subscribe to the event
void SensorManager::registerSubscriberForUpdateSensorData(IPublisher *publisher)
{
    publishers.push_back(publisher);
}

// Method to unsubscribe from the event
void SensorManager::unsubscribeFromSensorDataEvent(IPublisher *publisher)
{
    publishers.erase(
        std::remove(publishers.begin(), publishers.end(), publisher),
        publishers.end());
}

// Trigger one SHT30 measurement; on success, sht30.cTemp/sht30.humidity
// hold both the temperature and humidity readings from that same reading.
bool SensorManager::readSht30()
{
    if (sht30.get() != 0)
    {
        errors.push_back("SHT30 sensor reading failed");
        Trace::log(TraceLevel::WARNING, "SHT30 sensor reading failed");
        return false;
    }
    _shtReadOnce = true;
    return true;
}

// Trigger one SGP30 IAQ measurement; on success, sgp30.TVOC/sgp30.eCO2 hold
// both readings from that same measurement. Called from
// tickSgp30Measurement() on its own fixed cadence, not from
// updateSensorData() - failures are surfaced via _lastSgp30MeasureOk there,
// not pushed into `errors` directly (that vector is owned by
// updateSensorData()'s per-publish-cycle clear/accumulate, a different
// cadence).
bool SensorManager::readSgp30()
{
    if(!sgp30.IAQmeasure())
    {
        Trace::log(TraceLevel::WARNING, "SGP30 IAQmeasure measurement failed");
        return false;
    }
    return true;
}

// Read light level from BH1750 sensor
uint16_t SensorManager::readLightLevel()
{
    return lightMeter.readLightLevel();
}

// Trigger one SGP30 raw measurement; on success, sgp30.rawH2/rawEthanol
// hold both readings from that same measurement.
bool SensorManager::readSgp30Raw()
{
    if(!sgp30.IAQmeasureRaw())
    {
        errors.push_back("SGP30 IAQmeasureRaw measurement failed");
        Trace::log(TraceLevel::WARNING, "SGP30 IAQmeasureRaw measurement failed");
        return false;
    }
    return true;
}

// Get sensor data from all sensors
void SensorManager::updateSensorData()
{
    SensorData sensorData;

    // Clear errors
    errors.clear();

    // SHT30 and the SGP30 raw signal are measured exactly once per cycle;
    // both derived values per sensor (temperature+humidity, H2+ethanol) are
    // read from the single resulting measurement instead of triggering it
    // twice. eCO2/TVOC are the exception - see the comment below.
    // SensorData's temperature/humidity/tvoc/co2 fields have no default
    // member initializer, so the failure branches below explicitly zero
    // them - matching the original readTemperature()/readHumidity()/etc.
    // wrappers, which returned 0 on failure.
    if (readSht30())
    {
        sensorData.temperature = sht30.cTemp;
        sensorData.humidity = sht30.humidity;
    }
    else
    {
        sensorData.temperature = 0;
        sensorData.humidity = 0;
    }

    // eCO2/TVOC are measured on their own fixed 1Hz cadence
    // (tickSgp30Measurement(), driven by loop()) rather than here - this
    // just reads the most recently cached result.
    if (_lastSgp30MeasureOk)
    {
        sensorData.tvoc = _lastTvoc;
        sensorData.co2 = _lastEco2;
    }
    else
    {
        sensorData.tvoc = 0;
        sensorData.co2 = 0;
        errors.push_back("SGP30 IAQmeasure measurement failed");
    }

    if (readSgp30Raw())
    {
        sensorData.rawH2 = sgp30.rawH2;
        sensorData.rawEthanol = sgp30.rawEthanol;
    }
    else
    {
        sensorData.rawH2 = 0;
        sensorData.rawEthanol = 0;
    }

    sensorData.lightLevel = readLightLevel();

    maybeSaveSgp30Baseline();

    // Accumulate errors to one string
    for (auto &error : errors)
    {
        sensorData.errors += error + "; ";
    }

    // Notify all subscribers
    for (auto *publisher : publishers)
    {
        publisher->publish(sensorData);
    }
}
