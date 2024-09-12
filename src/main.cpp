// main.cpp

#include <Arduino.h>
#include "SensorManager.h"
#include "PublishManager.h"
#include "OtaManager.h"

// Create instances of SensorManager and PublishManager
SensorManager sensorManager;
PublishManager pubManager;
OtaManager otaManager;

unsigned long lastUpdateTime;                  // Last time the values were published
unsigned long updateSensorDataInterval = 5000; // Interval to update sensor data

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(10);
  }
  Serial.println("Welcome to AZ-ONEBoard!");

  // Setup OTA
  otaManager.setup();

  // Initialize sensors
  sensorManager.setup();

  // Register PublishManager for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&pubManager);

  // Setup PublishManager
  pubManager.setup();
}

void loop()
{
  // Handle OTA
  otaManager.handle();

  // Get current time
  unsigned long currentTime = millis();

  // Update sensor data if the interval has passed
  if (currentTime - lastUpdateTime >= updateSensorDataInterval)
  {
    lastUpdateTime = currentTime;
    sensorManager.updateSensorData();
  }
}
