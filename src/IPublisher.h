// IPublisher.h

#pragma once

#include "SensorData.h"

class IPublisher
{
public:
    virtual void publish(const SensorData &data) = 0;
    virtual ~IPublisher() = default;
};