#include <Arduino.h>
#include "./Sensors/SensorManager.h"
#include "./Publishers/Serial/SerialPublisher.h"
#include "./Publishers/Mqtt/MqttPublisher.h"
#include "./Publishers/Webserver/WebserverPublisher.h"
#include "./_infra/EspWifiClient.h"
#include "./_infra/OtaManager.h"
#include "./_infra/EepromManager.h"

EspWifiClient espWifiClient;
SensorManager sensorManager;
SerialPublisher serialPublisher;
MqttPublisher mqttPublisher;
WebserverPublisher wsPublisher;
OtaManager otaManager;
EepromManager eepromManager;

unsigned long lastSensorUpdateTime;                  // Last time the values were updated
unsigned long lastCommonDataUpdateTime;              // Last time the common data was updated
unsigned long updateSensorDataInterval; // Interval to update sensor data
unsigned long defaultUpdateSensorDataInterval = 5000; // Interval to update sensor data
unsigned long UpdateCommonDataInterval = 60000 * 5;

void updateSensorDataIntervalCallback(int newValue)
{
    updateSensorDataInterval = newValue;
    eepromManager.writeUpdateSensorDataInterval(newValue);
}

void publishAtSetup()
{
    CommonData commonData;
    commonData.updateSensorDataInterval = updateSensorDataInterval;
    Serial.println("Publishing common data");
    mqttPublisher.publishCommonData(commonData);
}

void setup()
{
    // Read updateSensorDataInterval from EEPROM
    updateSensorDataInterval = eepromManager.readUpdateSensorDataInterval(defaultUpdateSensorDataInterval);

    // SerialPublisher setup
    serialPublisher.setup();

    // EspWifiClient setup
    espWifiClient.setup();

    // Get wifiClient and deviceName
    WiFiClient *wifiClient = espWifiClient.getWifiClient();
    String deviceName = espWifiClient.getDeviceName();

    // MqttPublisher setup
    mqttPublisher.setup(wifiClient, deviceName);

    // Subscribe to the topic to update the updateSensorDataInterval
    mqttPublisher.setCallback(updateSensorDataIntervalCallback);

    // Setup OTA
    otaManager.setup();

    // Setup WebserverPublisher
    wsPublisher.setup(deviceName);

    // Initialize sensors
    sensorManager.setup();

    publishAtSetup();

    // Register SerialPublisher for sensor data updates
    sensorManager.registerSubscriberForUpdateSensorData(&serialPublisher);

    // Register MqttPublisher for sensor data updates
    sensorManager.registerSubscriberForUpdateSensorData(&mqttPublisher);

    // Register WebserverPublisher for sensor data updates
    sensorManager.registerSubscriberForUpdateSensorData(&wsPublisher);
}

void loop()
{
    // Handle OTA
    otaManager.handle();

    // Handle WebserverPublisher
    wsPublisher.handle();

    // Get current time
    unsigned long currentTime = millis();

    // Update sensor data if the interval has passed
    if (currentTime - lastSensorUpdateTime >= updateSensorDataInterval)
    {
        lastSensorUpdateTime = currentTime;
        sensorManager.updateSensorData();
    }

    // Wait for a while
    delay(100);
}