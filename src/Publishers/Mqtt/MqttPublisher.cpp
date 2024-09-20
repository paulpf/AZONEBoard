// MqttPublisher.cpp

#include "MqttPublisher.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "../../_structures/TopicValuePair.h"
#include "../../_structures/CommonData.h"

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

    if (String(topic) == (instance->deviceName + "/updateSensorDataInterval"))
    {
        // Update updateSensorDataInterval
        int newValue = message.toInt();
        if (newValue > 0)
        {
            Serial.print("Updating updateSensorDataInterval to ");
            Serial.println(newValue);

           // Call updateSensorDataIntervalCallback with the new value
            instance->callbackFunction(newValue);
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

    TopicValuePair topics[] = 
    {
        {instance->deviceName + "/temperature", String(sensorData.temperature)},
        {instance->deviceName + "/humidity", String(sensorData.humidity)},
        {instance->deviceName + "/tvoc", String(sensorData.tvoc)},
        {instance->deviceName + "/co2", String(sensorData.co2)},
        {instance->deviceName + "/rawEthanol", String(sensorData.rawEthanol)},
        {instance->deviceName + "/rawH2", String(sensorData.rawH2)}
    };

    publishInternal(topics, 6);
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

            // Subscribe to messages
            mqttClient.subscribe((instance->deviceName + "/updateSensorDataInterval").c_str());
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

void MqttPublisher::publishCommonData(const CommonData &commonData)
{
    // Connect to the mqtt broker
    if (!mqttClient.connected())
    {
        instance->reconnectMqtt();
    }
    mqttClient.loop();

    TopicValuePair topics[] = 
    {
        {instance->deviceName + "/ipAddress", instance->ipAddress.toString()},
        {instance->deviceName + "/updateSensorDataInterval", String(commonData.updateSensorDataInterval)}
    };

    instance->publishInternal(topics, 2);
}

void MqttPublisher::publishInternal(TopicValuePair *topics, size_t count)
{
    // Iterate over the array and publish each topic
    for (size_t i = 0; i < count; ++i)
    {
        const auto &topicValuePair = topics[i];
        if (mqttClient.publish(topicValuePair.topic.c_str(), topicValuePair.value.c_str()) == false)
        {
            Serial.println(topicValuePair.topic + " not published");
        }
    }
}

void MqttPublisher::setCallback(void (*callback)(int))
{
    instance->callbackFunction = callback;
}