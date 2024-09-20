// SensorManager.cpp

#include "SensorManager.h"
#include "./_structures/SensorData.h"
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
        Serial.println("Failed to initialize SGP30 sensor!");
        return false;
    }

    // Initialize BH1750 sensor
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
    {
        Serial.println("Failed to initialize BH1750 sensor!");
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

// Read temperature from SHT30 sensor
float SensorManager::readTemperature()
{
    // Get data from SHT30 sensor
    if(readSht30())
    {
        return sht30.cTemp;
    }
    return 0;
}

bool SensorManager::readSht30()
{
    if (sht30.get() != 0)
    {
        errors.push_back("SHT30 sensor reading failed");
        Serial.println("SHT30 sensor reading failed");
        return false;
    }
    return true;
}

// Read humidity from SHT30 sensor
float SensorManager::readHumidity()
{
    // Get data from SHT30 sensor
    if(readSht30())
    {
        return sht30.humidity;
    }
    return 0;
}

// Read TVOC from SGP30 sensor
uint16_t SensorManager::readTVOC()
{
   if(readSgp30())
   {
       return sgp30.TVOC;
   }
    return 0;
}

bool SensorManager::readSgp30()
{
    if(!sgp30.IAQmeasure())
    {
        errors.push_back("SGP30 IAQmeasure measurement failed");
        Serial.println("SGP30 IAQmeasure measurement failed");
        return false;
    }
    return true;
}

// Read CO2 from SGP30 sensor
uint16_t SensorManager::readCO2()
{
    if(readSgp30())
    {
        return sgp30.eCO2;
    }
    return 0;
}

// Read light level from BH1750 sensor
uint16_t SensorManager::readLightLevel()
{
    return lightMeter.readLightLevel();
}

// Read raw ethanal value from SGP30 sensor
uint16_t SensorManager::readEthanol()
{
    if(readSgp30Raw())
    {
        return sgp30.rawEthanol;
    }
    return 0;
}

bool SensorManager::readSgp30Raw()
{
    if(!sgp30.IAQmeasureRaw())
    {
        errors.push_back("SGP30 IAQmeasureRaw measurement failed");
        Serial.println("SGP30 IAQmeasureRaw measurement failed");
        return false;
    }
    return true;
}

// Read raw H2 value from SGP30 sensor
uint16_t SensorManager::readH2()
{
    if(readSgp30Raw())
    {
        return sgp30.rawH2;
    }
    return 0;
}

// Get sensor data from all sensors
void SensorManager::updateSensorData()
{
    SensorData sensorData;

    // Clear errors
    errors.clear();

    sensorData.temperature = readTemperature();
    sensorData.humidity = readHumidity();
    sensorData.tvoc = readTVOC();
    sensorData.co2 = readCO2();
    sensorData.rawEthanol = readEthanol();
    sensorData.rawH2 = readH2();
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