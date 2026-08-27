#include "mqttmanager.h"
#include "trace.h"

MqttManager::MqttManager()
    : _mqttServer(nullptr), _mqttPort(1883), _mqttUser(nullptr),
      _mqttPassword(nullptr), _connectRequested(false), _subscribeCount(0)
{
  _pubSubClient.setClient(_wifiClient);
}

void MqttManager::setup(const char *mqttServer, int mqttPort,
                        const char *mqttUser, const char *mqttPassword,
                        const String &clientId)
{
  _mqttServer = mqttServer;
  _mqttPort = mqttPort;
  _mqttUser = mqttUser;
  _mqttPassword = mqttPassword;
  _clientId = clientId;
  _lwtTopic = _clientId + "/system/status";
  _pubSubClient.setServer(_mqttServer, _mqttPort);
  Trace::log(TraceLevel::INFO, "MqttManager setup complete");
}

void MqttManager::loop()
{
  switch (_sessionManager.state())
  {
  case MqttSessionManager::MQTT_DISCONNECTED_STATE:
    if (_connectRequested && _sessionManager.shouldAttemptConnect(millis()))
    {
      reconnect();
    }
    break;

  case MqttSessionManager::MQTT_CONNECTED_STATE:
    if (!_pubSubClient.connected())
    {
      Trace::logf(TraceLevel::WARNING, "MQTT connection lost, rc=%d", _pubSubClient.state());
      _sessionManager.onConnectionLost();
    }
    else
    {
      _pubSubClient.loop();
    }
    break;

  case MqttSessionManager::MQTT_CONNECTING_STATE:
    break;
  }
}

void MqttManager::reconnect()
{
  Trace::log(TraceLevel::INFO, "MQTT connecting...");
  _sessionManager.onConnectAttemptStarted();

  if (_pubSubClient.connect(_clientId.c_str(), _mqttUser, _mqttPassword,
                            _lwtTopic.c_str(), 1, true, "offline"))
  {
    Trace::log(TraceLevel::INFO, "MQTT connected");
    _sessionManager.onConnectSuccess();
    _pubSubClient.publish(_lwtTopic.c_str(), "online", true);

    for (int i = 0; i < _subscribeCount; i++)
    {
      _pubSubClient.subscribe(_subscribeTopics[i]);
    }
  }
  else
  {
    _sessionManager.onConnectFailure();
    Trace::logf(TraceLevel::ERROR, "MQTT connect failed, rc=%d", _pubSubClient.state());
  }
}

void MqttManager::publish(const char *topic, const char *payload)
{
  if (_sessionManager.isConnected())
  {
    _pubSubClient.publish(topic, payload);
  }
}

void MqttManager::publishRetained(const char *topic, const char *payload)
{
  if (_sessionManager.isConnected())
  {
    _pubSubClient.publish(topic, payload, true);
  }
}

bool MqttManager::isConnected()
{
  return _sessionManager.isConnected();
}

void MqttManager::requestConnect()
{
  if (!_connectRequested)
  {
    Trace::log(TraceLevel::INFO, "MQTT connect requested");
  }
  _connectRequested = true;
}

void MqttManager::forceDisconnect()
{
  _connectRequested = false;
  if (_pubSubClient.connected())
  {
    _pubSubClient.disconnect();
  }
  _sessionManager.forceDisconnect();
}

void MqttManager::subscribe(const char *topic)
{
  if (_subscribeCount < MAX_SUBSCRIPTIONS)
  {
    _subscribeTopics[_subscribeCount++] = topic;
  }
}

void MqttManager::setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback)
{
  _pubSubClient.setCallback(callback);
}
