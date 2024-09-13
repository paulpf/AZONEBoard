// PublishManager.cpp

#include "WebserverPublisher.h"
#include <Arduino.h>
#include "config.h"

// Initialize the static instance pointer
WebserverPublisher *WebserverPublisher::instance = nullptr;

// Constructor to initialize WebserverPublisher
WebserverPublisher::WebserverPublisher()
{
    instance = this;
}

void WebserverPublisher::setup()
{

}

// Implement interface method publish
void WebserverPublisher::publish(const SensorData &data)
{

}