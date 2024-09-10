#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/secret.h"
#else
#include "secret.h"
#endif

#include "CommunicationManager.h"
#include <Arduino.h>
#include "SensorData.h"
#include "config.h"
#include <ESP8266mDNS.h>

// Initialize the static instance pointer
CommunicationManager *CommunicationManager::instance = nullptr;

// Constructor to initialize PublishManager with a SensorManager reference and interval
CommunicationManager::CommunicationManager()
{
    instance = this;
}

void CommunicationManager::setup()
{
    setupWifi();
    setupMqtt();
}

// Method to get device name
String CommunicationManager::getDeviceName()
{
    deviceName = "AZ-ONEBoard/";
    deviceName += WiFi.macAddress();
    deviceName.replace(":", "");
    return deviceName;
}

// Method to setup wifi
void CommunicationManager::setupWifi()
{
    espClient = WiFiClient();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");

    // Subscribe on callback for changing IP address
    WiFi.onStationModeGotIP(renewIpAddress);

    deviceName = getDeviceName();

    // Print ip address
    ipAddress = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(instance->ipAddress);

    if (MDNS.begin("esp8266"))
    {
        Serial.println("MDNS responder started");
    }
}

// Callback function to renew IP address
void CommunicationManager::renewIpAddress(const WiFiEventStationModeGotIP &event)
{
    Serial.print("IP address was changed, IP: ");
    instance->ipAddress = event.ip;
    Serial.println(instance->ipAddress.toString());

    // Publish the new IP address
    if (instance->client.connected())
    {
        instance->client.publish((instance->deviceName + "/ipAddress").c_str(), instance->ipAddress.toString().c_str());
    }
}

// Method to setup mqtt
void CommunicationManager::setupMqtt()
{
    client.setClient(espClient);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);
}

// Method to reconnect to the mqtt broker
void CommunicationManager::reconnectMqtt()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(deviceName.c_str(), mqtt_user, mqtt_password))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Callback function to handle incoming mqtt messages
void CommunicationManager::mqttCallback(char *topic, byte *payload, unsigned int length)
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

// Method to publish sensor values on Serial
void CommunicationManager::publishOnSerial(SensorData sensorData)
{
    // Print sensor values to the serial
    Serial.print("Temperature: ");
    Serial.print(sensorData.temperature, 2); // Print temperature with 2 decimal places
    Serial.print(" C, Humidity: ");
    Serial.print(sensorData.humidity, 2); // Print humidity with 2 decimal places
    Serial.print(" %, TVOC: ");
    Serial.print(sensorData.tvoc);
    Serial.print(" ppb, CO2: ");
    Serial.print(sensorData.co2);
    Serial.print(" ppm, Raw ethanol: ");
    Serial.print(sensorData.rawEthanol);
    Serial.print(", Raw H2: ");
    Serial.print(sensorData.rawH2);
    Serial.println();
}

// Method to publish sensor values on mqtt
void CommunicationManager::publishOnMqtt(SensorData sensorData)
{
    // Connect to the mqtt broker
    if (!client.connected())
    {
        reconnectMqtt();
    }
    client.loop();
    
    // Publish sensor values
    if (client.publish((deviceName + "/temperature").c_str(), String(sensorData.temperature).c_str()) == false)
    {
        Serial.println("Temperature not published");
    }
    if (client.publish((deviceName + "/humidity").c_str(), String(sensorData.humidity).c_str()) == false)
    {
        Serial.println("Humidity not published");
    }
    if (client.publish((deviceName + "/tvoc").c_str(), String(sensorData.tvoc).c_str()) == false)
    {
        Serial.println("TVOC not published");
    }
    if (client.publish((deviceName + "/co2").c_str(), String(sensorData.co2).c_str()) == false)
    {
        Serial.println("CO2 not published");
    }
    if (client.publish((deviceName + "/rawEthanol").c_str(), String(sensorData.rawEthanol).c_str()) == false)
    {
        Serial.println("Raw ethanol not published");
    }
    if (client.publish((deviceName + "/rawH2").c_str(), String(sensorData.rawH2).c_str()) == false)
    {
        Serial.println("Raw H2 not published");
    }
    
    // Publish ip address
    if (client.publish((deviceName + "/ipAddress").c_str(), ipAddress.toString().c_str()) == false)
    {
        Serial.println("IP address not published");
    }
}