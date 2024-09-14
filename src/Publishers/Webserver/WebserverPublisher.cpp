#include "WebserverPublisher.h"
#include <Arduino.h>
#include "config.h"
#include "ESP8266WebServer.h"
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h> // Hinzufügen der WebSocket-Bibliothek

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
    webServer.on("/data", HTTP_GET, handleData);
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
    updateWebpage();

    // Handle the webserver
    webServer.handleClient();
}

void WebserverPublisher::handleRoot()
{
  // Create a html site with the overview of the sensor data
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'></head><body>";
  html += "<h1>Overview</h1>";
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

void WebserverPublisher::handleData()
{
    // Create a JSON object
    StaticJsonDocument<200> doc;
    doc["deviceName"] = instance->deviceName;
    doc["temperature"] = instance->lastData.temperature;
    doc["humidity"] = instance->lastData.humidity;
    doc["co2"] = instance->lastData.co2;
    doc["tvoc"] = instance->lastData.tvoc;
    doc["light"] = instance->lastData.lightLevel;
    doc["rawH2"] = instance->lastData.rawH2;
    doc["rawEthanol"] = instance->lastData.rawEthanol;

    // Serialize the JSON object
    String output;
    serializeJson(doc, output);

    // Send the JSON object
    instance->webServer.send(200, "application/json", output);
}

void WebserverPublisher::updateWebpage()
{
    // Create a html site with the overview of the sensor data
    String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='5'></head><body>";
    html += "<h1>Overview</h1>";
    html += "<p>Temperature: " + String(lastData.temperature) + " °C</p>";
    html += "<p>Humidity: " + String(lastData.humidity) + " %</p>";
    html += "<p>CO2: " + String(lastData.co2) + " ppm</p>";
    html += "<p>TVOC: " + String(lastData.tvoc) + " ppb</p>";
    html += "<p>Light: " + String(lastData.lightLevel) + " lux</p>";
    html += "<p>Raw H2: " + String(lastData.rawH2) + "</p>";
    html += "<p>Raw Ethanol: " + String(lastData.rawEthanol) + "</p>";
    html += "</body></html>";
  
    // Send the html site
    webServer.send(200, "text/html; charset=utf-8", html);
}