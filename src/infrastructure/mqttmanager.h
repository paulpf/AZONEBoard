#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include "imessagepublisher.h"
#include "imqttconnectioncontrol.h"
#include "mqttsessionmanager.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <functional>

// Generic, app-agnostic MQTT client: setup() configures the broker, loop()
// (call every Application::loop() iteration) drives a non-blocking
// reconnect state machine (see domain/mqttsessionmanager.h) and services
// the PubSubClient. Callers publish/subscribe by topic string - this class
// has no knowledge of what data it carries. Connects/reconnects are
// externally gated via requestConnect()/forceDisconnect() (see
// services/connectivitycoordinator.h), not automatic on publish.
//
// Publishes a Last Will and Testament to "<clientId>/system/status": broker
// sets it to "offline" (retained) if the device drops off ungracefully;
// "online" (retained) is published right after every successful connect.
class MqttManager : public IMessagePublisher, public IMqttConnectionControl
{
public:
  MqttManager();

  void setup(const char *mqttServer, int mqttPort, const char *mqttUser,
             const char *mqttPassword, const String &clientId);
  void loop();

  void publish(const char *topic, const char *payload) override;
  void publishRetained(const char *topic, const char *payload) override;
  bool isConnected() override;

  void requestConnect() override;
  void forceDisconnect() override;

  // Re-subscribed automatically on every (re)connect. The topic pointer
  // must stay valid for the lifetime of MqttManager - pass a string literal
  // or a persistent (member/static) String's c_str(), never a temporary's.
  void subscribe(const char *topic);
  void setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback);

private:
  void reconnect();

  const char *_mqttServer;
  int _mqttPort;
  const char *_mqttUser;
  const char *_mqttPassword;
  String _clientId;
  String _lwtTopic;
  bool _connectRequested;

  static const int MAX_SUBSCRIPTIONS = 4;
  const char *_subscribeTopics[MAX_SUBSCRIPTIONS];
  int _subscribeCount;

  WiFiClient _wifiClient;
  PubSubClient _pubSubClient;
  MqttSessionManager _sessionManager;
};

#endif // MQTTMANAGER_H
