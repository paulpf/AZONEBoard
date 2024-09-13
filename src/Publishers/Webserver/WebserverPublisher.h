// PublishManager.h

#ifndef WEBSERVER_PUBLISHER_H
#define WEBSERVER_PUBLISHER_H

#include "./_structures/SensorData.h"
#include "../../_interfaces/IPublisher.h"

class WebserverPublisher : public IPublisher
{
public:
    WebserverPublisher();
    void setup();
    void publish(const SensorData &data);

private:
    static WebserverPublisher *instance; // Static pointer to the instance
};

#endif // WEBSERVER_PUBLISHER_H