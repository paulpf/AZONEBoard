// SensorManager.cpp

#include "SensorManager.h"
#include "./_structures/SensorData.h"
#include "trace.h"
#include <Wire.h>

// Constructor for SensorManager class
SensorManager::SensorManager() : sht30(0x44), sgp30(), lightMeter() {}

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
    return true;
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
    return true;
}

// Trigger one SGP30 IAQ measurement; on success, sgp30.TVOC/sgp30.eCO2
// hold both readings from that same measurement.
bool SensorManager::readSgp30()
{
    if(!sgp30.IAQmeasure())
    {
        errors.push_back("SGP30 IAQmeasure measurement failed");
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

    // Each physical sensor is measured exactly once per cycle; both derived
    // values per sensor (temperature+humidity, TVOC+CO2, H2+ethanol) are
    // read from the single resulting measurement instead of triggering it
    // twice.
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

    if (readSgp30())
    {
        sensorData.tvoc = sgp30.TVOC;
        sensorData.co2 = sgp30.eCO2;
    }
    else
    {
        sensorData.tvoc = 0;
        sensorData.co2 = 0;
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
