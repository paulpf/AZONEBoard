#include <Arduino.h>
#include "SensorManager.h"
#include "CommunicationManager.h"
#include "OtaManager.h"

// Create instances of SensorManager and PublishManager
SensorManager sensorManager;
CommunicationManager communicationManager;
OtaManager otaManager;

unsigned long lastPublishTime; // Last time the values were published
unsigned long publishInterval = 5000; // Interval between publishes

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

  // Setup CommunicationManager
  communicationManager.setup();
}

void loop()
{
  // Handle OTA
  otaManager.handle();

  // Get current time
  unsigned long currentTime = millis();

  // If the time since the last publish is greater than the interval,
  // read sensor data and publish the values
  if (currentTime - lastPublishTime >= publishInterval)
  {
      lastPublishTime = currentTime;

      SensorData sensorData = sensorManager.getSensorData();

      // Periodically publish sensor values
      communicationManager.publishOnSerial(sensorData);

      // Publish sensor values on mqtt
      communicationManager.publishOnMqtt(sensorData);
  }
}
