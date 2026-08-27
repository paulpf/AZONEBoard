#include "WifiSecret.h"

#include "EspWifiClient.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

EspWifiClient::EspWifiClient()
{
}

void EspWifiClient::setup()
{
    wifiClient = WiFiClient();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected at IP address: ");
    Serial.println(WiFi.localIP());
}

WiFiClient *EspWifiClient::getWifiClient()
{
    return &wifiClient;
}

// Method to get device name
String EspWifiClient::getDeviceName()
{
    String deviceName = "AZ-ONEBoard/";
    deviceName += WiFi.macAddress();
    deviceName.replace(":", "");
    return deviceName;
}