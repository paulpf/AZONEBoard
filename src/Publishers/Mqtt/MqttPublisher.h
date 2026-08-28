// MqttPublisher.h

#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include <Arduino.h>
#include "./_structures/SensorData.h"
#include "./_structures/CommonData.h"
#include "../../_interfaces/IPublisher.h"
#include "imessagepublisher.h"

// Translates AZONEBoard's structured sensor/common data into topic
// publishes on the generic MqttManager (see infrastructure/mqttmanager.h).
// Keeps IPublisher so SensorManager's subscriber fan-out (Serial/MQTT/
// Webserver) stays unchanged; all connection/reconnect/LWT handling lives
// in MqttManager - publishes are simply no-ops while disconnected (see
// IMessagePublisher::publish/publishRetained), no guard needed here.
class MqttPublisher : public IPublisher
{
public:
    explicit MqttPublisher(IMessagePublisher &messagePublisher);
    void setup(const String &deviceName);
    void publish(const SensorData &data) override;
    void publishCommonData(const CommonData &commonData);
    void publishRssi(int rssi);
    void publishHealth(const char *healthJson);

private:
    IMessagePublisher &_messagePublisher;
    String _deviceName;
};

#endif // MQTT_PUBLISHER_H
