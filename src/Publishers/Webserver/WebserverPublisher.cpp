#include "WebserverPublisher.h"
#include <Arduino.h>
#include "config.h"
#include "ESP8266WebServer.h"
#include <ESP8266mDNS.h>

WebserverPublisher *WebserverPublisher::instance = 0;

WebserverPublisher::WebserverPublisher()
{
    instance = this;
}

void WebserverPublisher::setup(String deviceName)
{
    this->deviceName = deviceName;

    // Start the webserver
    webServer.on("/", HTTP_GET, handleRoot);
    webServer.begin();
}

void WebserverPublisher::handle()
{
    // Handle the webserver
    webServer.handleClient();
}

// Implement the interface method
void WebserverPublisher::publish(const SensorData &data)
{
    this->lastData = data;

    // Update the webpage with new sensor data
    instance->handleRoot();

    // Handle the webserver
    webServer.handleClient();
}

void WebserverPublisher::handleRoot()
{
  // Create a html site with the overview of the sensor data
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'></head><body>";
  html += "<h1>Overview of sensor data on '" + instance->deviceName + "'</h1>";
  html += "<p>Temperature: " + String(instance->lastData.temperature) + " °C</p>";
  html += "<p>Humidity: " + String(instance->lastData.humidity) + " %</p>";
  html += "<p>CO2: " + String(instance->lastData.co2) + " ppm</p>";
  html += "<p>TVOC: " + String(instance->lastData.tvoc) + " ppb</p>";
  html += "<p>Light: " + String(instance->lastData.lightLevel) + " lux</p>";
  html += "<p>Raw H2: " + String(instance->lastData.rawH2) + "</p>";
  html += "<p>Raw Ethanol: " + String(instance->lastData.rawEthanol) + "</p>";
  html += "</body></html>";
  
  // Send the html site with content-type header including charset=utf-8
  instance->webServer.send(200, "text/html; charset=utf-8", html);
}
