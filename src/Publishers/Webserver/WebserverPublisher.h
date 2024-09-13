// PublishManager.h

#ifndef WEBSERVER_PUBLISHER_H
#define WEBSERVER_PUBLISHER_H

#include "./_structures/SensorData.h"
#include "../../_interfaces/IPublisher.h"
#include <ESP8266WebServer.h>

class WebserverPublisher : public IPublisher
{
public:
    WebserverPublisher();
    void setup(String deviceName);
    void publish(const SensorData &data);

private:
    static WebserverPublisher *instance; // Static pointer to the instance
    ESP8266WebServer webServer;
    static void handleRoot();
    static void handleData();
    String deviceName;
    SensorData sensorData;
};

#endif // WEBSERVER_PUBLISHER_H