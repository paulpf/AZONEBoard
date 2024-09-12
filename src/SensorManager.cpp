// SensorManager.cpp

#include "SensorManager.h"
#include "SensorData.h"
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
    sht30.get();
    return sht30.cTemp;
}

// Read humidity from SHT30 sensor
float SensorManager::readHumidity()
{
    // Get data from SHT30 sensor
    sht30.get();
    return sht30.humidity;
}

// Read TVOC from SGP30 sensor
uint16_t SensorManager::readTVOC()
{
    // Measure air quality
    sgp30.IAQmeasure();
    return sgp30.TVOC;
}

// Read CO2 from SGP30 sensor
uint16_t SensorManager::readCO2()
{
    // Measure air quality
    sgp30.IAQmeasure();
    return sgp30.eCO2;
}

// Read light level from BH1750 sensor
uint16_t SensorManager::readLightLevel()
{
    return lightMeter.readLightLevel();
}

// Read raw ethanal value from SGP30 sensor
uint16_t SensorManager::readEthanol()
{
    // Measure air quality
    sgp30.IAQmeasureRaw();
    return sgp30.rawEthanol;
}

// Read raw H2 value from SGP30 sensor
uint16_t SensorManager::readH2()
{
    // Measure air quality
    sgp30.IAQmeasureRaw();
    return sgp30.rawH2;
}

// Get sensor data from all sensors
void SensorManager::updateSensorData()
{
    SensorData sensorData;

    sensorData.temperature = readTemperature();
    sensorData.humidity = readHumidity();
    sensorData.tvoc = readTVOC();
    sensorData.co2 = readCO2();
    sensorData.rawEthanol = readEthanol();
    sensorData.rawH2 = readH2();
    sensorData.lightLevel = readLightLevel();

    // Notify all subscribers
    for (auto *publisher : publishers)
    {
        publisher->publish(sensorData);
    }
}