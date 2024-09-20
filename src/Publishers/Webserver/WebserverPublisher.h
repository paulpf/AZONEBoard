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
    void handle();

private:
    // Instance of this class
    static WebserverPublisher *instance;
    SensorData lastData;
    ESP8266WebServer webServer;
    String deviceName;
    static void handleRoot();
};

#endif // WEBSERVER_PUBLISHER_H