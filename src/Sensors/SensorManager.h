// SensorManager.h

#pragma once

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <vector>
#include <functional>
#include "./_interfaces/IPublisher.h"
#include "./_structures/SensorData.h"

// Include necessary sensor libraries
#include <WEMOS_SHT3X.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>

// SensorManager class to manage multiple sensors
class SensorManager
{
public:
	// Constructor to initialize the SensorManager
	SensorManager();

	// Destructor to clean up resources
	~SensorManager();

	// Method to subscribe to the event
	void registerSubscriberForUpdateSensorData(IPublisher *publisher);

	// Method to unsubscribe from the event
	void unsubscribeFromSensorDataEvent(IPublisher *publisher);

	// Method to initialize all sensors
	bool setup();
	// Member function to update sensor data
	void updateSensorData();

private:
	// Vector to store publishers
	std::vector<IPublisher *> publishers;

	// Private members for each sensor
	SHT3X sht30;		  // Temperature and humidity sensor
	Adafruit_SGP30 sgp30; // Air quality sensor (TVOC and CO2)
	BH1750 lightMeter;	  // Light level sensor

	// Each of these triggers exactly one hardware measurement per call and
	// leaves the result cached on the sensor driver object (sht30.cTemp /
	// sht30.humidity, sgp30.TVOC / sgp30.eCO2, sgp30.rawH2 / rawEthanol).
	// updateSensorData() calls each once per cycle and reads both derived
	// values from that cache, instead of re-triggering the same physical
	// measurement twice (previously readTemperature()/readHumidity() each
	// called readSht30() independently, and likewise for the SGP30 pairs).
	bool readSht30();
	bool readSgp30();
	bool readSgp30Raw();
	uint16_t readLightLevel(); // Read light level from BH1750

	// dynamic Array with errors strings during reading sensors
	std::vector<String> errors;
};

#endif // SENSOR_MANAGER_H