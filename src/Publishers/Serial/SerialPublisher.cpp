// SerialPublisher.cpp

#include "SerialPublisher.h"
#include <Arduino.h>

SerialPublisher *SerialPublisher::instance = nullptr;

SerialPublisher::SerialPublisher()
{
    instance = this;
}

void SerialPublisher::setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }
    Serial.println("Welcome to AZ-ONEBoard!");
}

void SerialPublisher::publish(const SensorData &sensorData)
{
    Serial.print("Temperature: ");
    Serial.print(sensorData.temperature);
    Serial.print(" C, Humidity: ");
    Serial.print(sensorData.humidity);
    Serial.print(" %, CO2: ");
    Serial.print(sensorData.co2);
    Serial.print(" ppm, TVOC: ");
    Serial.print(sensorData.tvoc);
    Serial.print(" ppb, Raw H2: ");
    Serial.print(sensorData.rawH2);
    Serial.print(", Raw Ethanol: ");
    Serial.print(sensorData.rawEthanol);
    Serial.print(", Light: ");
    Serial.print(sensorData.lightLevel);
    Serial.print(" lx");
    Serial.println();
}