// PublishManager.h

#ifndef PUBLISH_MANAGER_H
#define PUBLISH_MANAGER_H

#include "SensorManager.h"
#include "SensorData.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include "IPublisher.h"

class PublishManager : public IPublisher
{
public:
    PublishManager();

    void publish(const SensorData &data) override
    {
        publishOnMqtt(data);
    }

    // Device name
    String deviceName;

    void setup();
    // Method to publish sensor values on mqtt
    void publishOnMqtt(SensorData sensorData);

private:
    WiFiClient espClient;
    PubSubClient client;
    ESP8266WebServer webServer;
    SensorData lastSensorData;

    void setupWifi();
    void setupWebserver();
    void setupMqtt();
    String getDeviceName();
    void reconnectMqtt();

    static PublishManager *instance; // Static pointer to the instance
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    static void renewIpAddress(const WiFiEventStationModeGotIP &event);
    static void handleRoot();
    static void handleData();
    IPAddress ipAddress;
};

#endif // PUBLISH_MANAGER_H