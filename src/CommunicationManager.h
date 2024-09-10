#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include "SensorManager.h"
#include "SensorData.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class CommunicationManager
{
private:
    
    WiFiClient espClient;
    PubSubClient client;

    void setupWifi();
    
    void setupMqtt();
    String getDeviceName();
    void reconnectMqtt();

    static CommunicationManager* instance; // Static pointer to the instance
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    static void renewIpAddress(const WiFiEventStationModeGotIP &event);
    IPAddress ipAddress;

public:
    CommunicationManager();
    
    // Device name
    String deviceName;

    void setup();
    // Method to publish sensor values on Serial
    void publishOnSerial(SensorData sensorData);
    // Method to publish sensor values on mqtt
    void publishOnMqtt(SensorData sensorData);
};

#endif // COMMUNICATION_MANAGER_H