// MqttPublisher.h

#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "./_structures/SensorData.h"
#include "../../_interfaces/IPublisher.h"
#include "../../_interfaces/delegates.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "./_structures/CommonData.h"
#include "./_structures/TopicValuePair.h"

class MqttPublisher : public IPublisher
{
public:
    MqttPublisher();
    void setup(WiFiClient *wifiClient, String deviceName);
    void publish(const SensorData &data);
    void publishCommonData(const CommonData &commonData);
    void registerCallback(void (*updateSensorDataInterval)(int));

private:
    static MqttPublisher *instance; // Static pointer to the instance
    PubSubClient mqttClient;
    String deviceName;
    IPAddress ipAddress;
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    void reconnectMqtt();
    void publishInternal(TopicValuePair *topics, size_t count);
    void (*callbackFunction)(int);
    CallbackDelegate updateSensorDataInterval;
};

#endif // MQTT_PUBLISHER_H