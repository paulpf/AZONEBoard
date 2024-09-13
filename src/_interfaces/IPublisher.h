// IPublisher.h

#ifndef I_PUBLISHER_H
#define I_PUBLISHER_H

#pragma once

#include "./_structures/SensorData.h"

class IPublisher
{
public:
    virtual void publish(const SensorData &data) = 0;
    virtual ~IPublisher() = default;
};

#endif // I_PUBLISHER_H