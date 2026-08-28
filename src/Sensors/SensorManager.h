// SensorManager.h

#pragma once

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <vector>
#include <functional>
#include "./_interfaces/IPublisher.h"
#include "./_structures/SensorData.h"
#include "./_infra/EepromManager.h"

// Include necessary sensor libraries
#include <WEMOS_SHT3X.h>
#include <Adafruit_SGP30.h>
#include <BH1750.h>

// SensorManager class to manage multiple sensors
class SensorManager
{
public:
	// Constructor to initialize the SensorManager
	SensorManager(EepromManager &eepromManager);

	// Destructor to clean up resources
	~SensorManager();

	// Method to subscribe to the event
	void registerSubscriberForUpdateSensorData(IPublisher *publisher);

	// Method to unsubscribe from the event
	void unsubscribeFromSensorDataEvent(IPublisher *publisher);

	// Method to initialize all sensors
	bool setup();
	// Must be called every Application::loop() iteration (unconditionally,
	// not gated on the publish interval): drives the SGP30's required fixed
	// 1Hz measurement cadence, see tickSgp30Measurement().
	void loop();
	// Member function to update sensor data
	void updateSensorData();

private:
	// Vector to store publishers
	std::vector<IPublisher *> publishers;

	// Private members for each sensor
	SHT3X sht30;		  // Temperature and humidity sensor
	Adafruit_SGP30 sgp30; // Air quality sensor (TVOC and CO2)
	BH1750 lightMeter;	  // Light level sensor

	EepromManager &_eepromManager;
	unsigned long _setupTime = 0;
	unsigned long _lastBaselineSaveTime = 0;

	// SGP30 eCO2/TVOC measurement, on its own fixed 1Hz cadence (see
	// tickSgp30Measurement()) rather than updateSensorData()'s publish
	// cadence - required by the sensor's on-chip baseline algorithm.
	unsigned long _lastSgp30MeasureTime = 0;
	uint16_t _lastEco2 = 0;
	uint16_t _lastTvoc = 0;
	bool _lastSgp30MeasureOk = false;
	// Set once readSht30() has succeeded at least once, so
	// tickSgp30Measurement() knows sht30.cTemp/sht30.humidity hold a real
	// reading before using them for humidity compensation.
	bool _shtReadOnce = false;

	// Restores a previously persisted SGP30 eCO2/TVOC baseline (if any) so
	// the on-chip algorithm doesn't have to re-learn it from scratch after
	// every reboot.
	void restoreSgp30Baseline();
	// Persists the current SGP30 baseline once the burn-in period has
	// passed, then periodically thereafter (see SGP30_BASELINE_* in
	// config.h). No-op until then.
	void maybeSaveSgp30Baseline();
	// Runs readSgp30() at the fixed SGP30_MEASURE_INTERVAL_MS cadence the
	// sensor requires (independent of the publish interval), first feeding
	// it absolute humidity (derived from the last SHT30 reading) for
	// on-chip compensation. Caches the result in _lastEco2/_lastTvoc.
	void tickSgp30Measurement();
	// Approximates absolute humidity [mg/m^3] from temperature and relative
	// humidity, per Sensirion's SGP30 driver integration guide - used to
	// feed Adafruit_SGP30::setHumidity() for compensation.
	static uint32_t calculateAbsoluteHumidity(float temperatureC, float relativeHumidityPercent);

	// Each of these triggers exactly one hardware measurement per call and
	// leaves the result cached on the sensor driver object (sht30.cTemp /
	// sht30.humidity, sgp30.TVOC / sgp30.eCO2, sgp30.rawH2 / rawEthanol).
	// readSht30()/readSgp30Raw() are called once per updateSensorData()
	// cycle and read both derived values from that cache, instead of
	// re-triggering the same physical measurement twice (previously
	// readTemperature()/readHumidity() each called readSht30()
	// independently, and likewise for the SGP30 raw pair). readSgp30() is
	// the exception - it's driven by tickSgp30Measurement() instead, on its
	// own fixed cadence.
	bool readSht30();
	bool readSgp30();
	bool readSgp30Raw();
	uint16_t readLightLevel(); // Read light level from BH1750

	// dynamic Array with errors strings during reading sensors
	std::vector<String> errors;
};

#endif // SENSOR_MANAGER_H