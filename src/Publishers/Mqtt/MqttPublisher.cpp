// MqttPublisher.cpp

#include "MqttPublisher.h"
#include <ESP8266WiFi.h>
#include "config.h"

MqttPublisher::MqttPublisher(IMessagePublisher &messagePublisher)
    : _messagePublisher(messagePublisher)
{
}

void MqttPublisher::setup(const String &deviceName)
{
    _deviceName = deviceName;
}

void MqttPublisher::publish(const SensorData &sensorData)
{
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_TEMPERATURE).c_str(),
                                       String(sensorData.temperature).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_HUMIDITY).c_str(),
                                       String(sensorData.humidity).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_TVOC).c_str(),
                                       String(sensorData.tvoc).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_CO2).c_str(),
                                       String(sensorData.co2).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_RAW_ETHANOL).c_str(),
                                       String(sensorData.rawEthanol).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_RAW_H2).c_str(),
                                       String(sensorData.rawH2).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_LIGHT_LEVEL).c_str(),
                                       String(sensorData.lightLevel).c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_ERRORS).c_str(),
                                       sensorData.errors.c_str());
}

void MqttPublisher::publishCommonData(const CommonData &commonData)
{
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_IP).c_str(),
                                       WiFi.localIP().toString().c_str());
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_SENSOR_INTERVAL_MS).c_str(),
                                       String(commonData.updateSensorDataInterval).c_str());
}

void MqttPublisher::publishRssi(int rssi)
{
    // Live signal-strength metric, not a lasting state - not retained,
    // matches how RSSI is published in the water-tank-monitor template.
    _messagePublisher.publish((_deviceName + MQTT_TOPIC_SUFFIX_RSSI).c_str(), String(rssi).c_str());
}

void MqttPublisher::publishHealth(const char *healthJson)
{
    _messagePublisher.publishRetained((_deviceName + MQTT_TOPIC_SUFFIX_HEALTH).c_str(), healthJson);
}
