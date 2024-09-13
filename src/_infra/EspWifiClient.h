// WifiClient.h

#ifndef ESP_WIFI_CLIENT_H
#define ESP_WIFI_CLIENT_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class EspWifiClient
{
public:
    EspWifiClient();
    void setup();
    WiFiClient *getWifiClient();
    String getDeviceName();

private:
    WiFiClient wifiClient;
    void connect();
    void reconnect();
};

#endif // ESP_WIFI_CLIENT_H