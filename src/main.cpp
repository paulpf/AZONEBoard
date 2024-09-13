// main.cpp

#include <Arduino.h>
#include "SensorManager.h"
#include "./Publishers/Serial/SerialPublisher.h"
#include "./Publishers/Mqtt/MqttPublisher.h"
#include "./Publishers/Webserver/WebserverPublisher.h"
#include "OtaManager.h"

#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#endif

WiFiClient espClient;

SensorManager sensorManager;
SerialPublisher serialPublisher;
MqttPublisher mqttPublisher;
WebserverPublisher wsPublisher;
OtaManager otaManager;

unsigned long lastUpdateTime;                  // Last time the values were published
unsigned long updateSensorDataInterval = 5000; // Interval to update sensor data

void setupWifi();
String getDeviceName();

void setup()
{
  // SerialPubslisher setup
  serialPublisher.setup();

  setupWifi();

  // MqttPublisher setup
  mqttPublisher.setup(&espClient, getDeviceName());

  // Setup OTA
  otaManager.setup();

  // Setup WebserverPublisher
  wsPublisher.setup();

  // Initialize sensors
  sensorManager.setup();

  // Register SerialPublisher for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&serialPublisher);

  // Register MqttPublisher for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&mqttPublisher);

  // Register PublishManager for sensor data updates
  sensorManager.registerSubscriberForUpdateSensorData(&wsPublisher);
}

void setupWifi()
{
    espClient = WiFiClient();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");    
}

// Method to get device name
String getDeviceName()
{
    String deviceName = "AZ-ONEBoard/";
    deviceName += WiFi.macAddress();
    deviceName.replace(":", "");
    return deviceName;
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
