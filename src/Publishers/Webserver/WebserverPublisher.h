#ifndef WEBSERVER_PUBLISHER_H
#define WEBSERVER_PUBLISHER_H

#include "./_structures/SensorData.h"
#include "../../_interfaces/IPublisher.h"
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> // Hinzufügen der WebSocket-Bibliothek

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
    static void handleData();
    void updateWebpage(); // Neue Methode zum Aktualisieren der Webseite
};

#endif // WEBSERVER_PUBLISHER_H