// main.cpp

#include <Arduino.h>
#include "./Sensors/SensorManager.h"
#include "./Publishers/Serial/SerialPublisher.h"
#include "./Publishers/Mqtt/MqttPublisher.h"
#include "./Publishers/Webserver/WebserverPublisher.h"
#include "./_infra/EspWifiClient.h"
#include "./_infra/OtaManager.h"

EspWifiClient espWifiClient;
SensorManager sensorManager;
SerialPublisher serialPublisher;
MqttPublisher mqttPublisher;
WebserverPublisher wsPublisher;
OtaManager otaManager;

unsigned long lastUpdateTime;                  // Last time the values were published
unsigned long updateSensorDataInterval = 5000; // Interval to update sensor data

void setup()
{
  // SerialPubslisher setup
  serialPublisher.setup();

  espWifiClient.setup();

  // Get wifiClient and deviceName
  WiFiClient *wifiClient = espWifiClient.getWifiClient();
  String deviceName = espWifiClient.getDeviceName();

  // MqttPublisher setup
  mqttPublisher.setup(wifiClient, deviceName);

  // Setup OTA
  otaManager.setup();

  // Setup WebserverPublisher
  wsPublisher.setup(deviceName);

  // Initialize sensors
  sensorManager.setup();

  // Register SerialPublisher for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&serialPublisher);

  // Register MqttPublisher for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&mqttPublisher);

  // Register PublishManager for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&wsPublisher);
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
