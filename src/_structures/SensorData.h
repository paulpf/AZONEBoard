// SensorData.h

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>
#include <inttypes.h>
#include <string>

struct SensorData
{
    float temperature;
    float humidity;
    uint16_t tvoc;
    uint16_t co2;
    uint16_t lightLevel;
    uint16_t rawH2 = 0;
    uint16_t rawEthanol = 0;
    String errors;
};

#endif // SENSOR_DATA_H