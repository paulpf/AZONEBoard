// MqttPublisher.h

#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "SensorData.h"
#include "IPublisher.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MqttPublisher : public IPublisher
{
public:
    MqttPublisher();
    void setup(WiFiClient *wifiClient, String deviceName);
    void publish(const SensorData &data);
    
private:
    static MqttPublisher *instance; // Static pointer to the instance
    PubSubClient mqttClient;
    String deviceName;
    IPAddress ipAddress;
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    void reconnectMqtt();
};

#endif // MQTT_PUBLISHER_H