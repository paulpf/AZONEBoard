// SerialPublisher.h

#ifndef SERIAL_PUBLISHER_H
#define SERIAL_PUBLISHER_H

#include "SensorData.h"
#include "IPublisher.h"

class SerialPublisher : public IPublisher
{
public:
    SerialPublisher();
    void setup();
    void publish(const SensorData &data);
    
private:
    static SerialPublisher *instance; // Static pointer to the instance
};

#endif // SERIAL_PUBLISHER_H