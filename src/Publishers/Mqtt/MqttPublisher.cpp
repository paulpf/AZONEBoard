// MqttPublisher.cpp

#include "MqttPublisher.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/MqttSecret.h"
#include "../../_configs/MqttConfig.h"
#else
#include "MqttSecret.h"
#include "MqttConfig.h"
#endif

MqttPublisher *MqttPublisher::instance = nullptr;

MqttPublisher::MqttPublisher()
{
    instance = this;
}

void MqttPublisher::setup(WiFiClient *wifiClient, String deviceName)
{
    instance->deviceName = deviceName;
    instance->ipAddress = WiFi.localIP();
    mqttClient.setClient(*wifiClient);
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);
}

// Callback function to handle incoming mqtt messages
void MqttPublisher::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;

    Serial.println("Message arrived [" + String(topic) + "] ");

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    if (String(topic) == (instance->deviceName + "/dataUpdateTime"))
    {
        // Update dataUpdateTime
        int newValue = message.toInt();
        if (newValue > 0)
        {
            Serial.print("Updating dataUpdateTime to ");
            Serial.println(newValue);
        }
    }
}

void MqttPublisher::publish(const SensorData &sensorData)
{
    // Connect to the mqtt broker
    if (!mqttClient.connected())
    {
        instance->reconnectMqtt();
    }
    mqttClient.loop();

    // Publish sensor values
    if (mqttClient.publish((instance->deviceName + "/temperature").c_str(), String(sensorData.temperature).c_str()) == false)
    {
        Serial.println("Temperature not published");
    }
    if (mqttClient.publish((instance->deviceName + "/humidity").c_str(), String(sensorData.humidity).c_str()) == false)
    {
        Serial.println("Humidity not published");
    }
    if (mqttClient.publish((instance->deviceName + "/tvoc").c_str(), String(sensorData.tvoc).c_str()) == false)
    {
        Serial.println("TVOC not published");
    }
    if (mqttClient.publish((instance->deviceName + "/co2").c_str(), String(sensorData.co2).c_str()) == false)
    {
        Serial.println("CO2 not published");
    }
    if (mqttClient.publish((instance->deviceName + "/rawEthanol").c_str(), String(sensorData.rawEthanol).c_str()) == false)
    {
        Serial.println("Raw ethanol not published");
    }
    if (mqttClient.publish((instance->deviceName + "/rawH2").c_str(), String(sensorData.rawH2).c_str()) == false)
    {
        Serial.println("Raw H2 not published");
    }

    // Publish ip address
    if (mqttClient.publish((instance->deviceName + "/ipAddress").c_str(), instance->ipAddress.toString().c_str()) == false)
    {
        Serial.println("IP address not published");
    }
}

// Method to reconnect to the mqtt broker
void MqttPublisher::reconnectMqtt()
{
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient.connect(deviceName.c_str(), mqtt_user, mqtt_password))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
