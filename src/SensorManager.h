#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

// Include necessary sensor libraries
#include <WEMOS_SHT3X.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>

#include "SensorData.h"

// SensorManager class to manage multiple sensors
class SensorManager
{
private:
	// Private members for each sensor
	SHT3X sht30;           // Temperature and humidity sensor
	Adafruit_SGP30 sgp30;  // Air quality sensor (TVOC and CO2)
	BH1750 lightMeter;     // Light level sensor

	// Methods to read data from each sensor
	float readTemperature();  // Read temperature from SHT30
	float readHumidity();     // Read humidity from SHT30
	uint16_t readTVOC();      // Read TVOC from SGP30
	uint16_t readCO2();       // Read CO2 from SGP30
	uint16_t readEthanol();   // Read raw ethanol value from SGP30
	uint16_t readH2();        // Read raw H2 value from SGP30
	uint16_t readLightLevel();// Read light level from BH1750

public:
	// Constructor to initialize the SensorManager
	SensorManager();

	// Destructor to clean up resources
	~SensorManager();

	// Method to initialize all sensors
	bool setup();
	// Member function to get sensor data
    SensorData getSensorData();
};

#endif // SENSOR_MANAGER_H