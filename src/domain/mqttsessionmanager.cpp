#include "mqttsessionmanager.h"

MqttSessionManager::MqttSessionManager(unsigned long retryIntervalMs,
                                       int maxReconnectAttempts)
    : _state(MQTT_DISCONNECTED_STATE), _lastMqttAttemptMillis(0),
      _mqttRetryInterval(retryIntervalMs), _mqttReconnectAttempts(0),
      _maxMqttReconnectAttempts(maxReconnectAttempts)
{
}

bool MqttSessionManager::shouldAttemptConnect(unsigned long now)
{
  if (_state != MQTT_DISCONNECTED_STATE)
  {
    return false;
  }

  if (now - _lastMqttAttemptMillis >= _mqttRetryInterval)
  {
    _lastMqttAttemptMillis = now;
    return true;
  }

  return false;
}

void MqttSessionManager::onConnectAttemptStarted()
{
  _state = MQTT_CONNECTING_STATE;
}

void MqttSessionManager::onConnectSuccess()
{
  _state = MQTT_CONNECTED_STATE;
  _mqttReconnectAttempts = 0;
}

void MqttSessionManager::onConnectFailure()
{
  _mqttReconnectAttempts++;
  _state = MQTT_DISCONNECTED_STATE;

  if (_mqttReconnectAttempts >= _maxMqttReconnectAttempts)
  {
    _mqttReconnectAttempts = 0;
  }
}

void MqttSessionManager::onConnectionLost()
{
  _state = MQTT_DISCONNECTED_STATE;
}

void MqttSessionManager::forceDisconnect()
{
  _state = MQTT_DISCONNECTED_STATE;
}

bool MqttSessionManager::isConnected() const
{
  return _state == MQTT_CONNECTED_STATE;
}

MqttSessionManager::MqttState MqttSessionManager::state() const
{
  return _state;
}

int MqttSessionManager::reconnectAttempts() const
{
  return _mqttReconnectAttempts;
}
