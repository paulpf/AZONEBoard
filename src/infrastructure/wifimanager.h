// wifimanager.h
#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "global_defines.h"
#include "iwificonnectivity.h"

// Non-blocking WiFi connection manager: setup() kicks off a connection
// attempt and returns immediately, loop() drives a small state machine
// (CONNECTING/CONNECTED/DISCONNECTED) and handles reconnects with
// exponential backoff + jitter (see domain/reconnectpolicy.h). Callers
// poll isConnected() / consumeConnectedEvent() / consumeDisconnectedEvent()
// instead of blocking on WiFi.status().
class WifiManager : public IWifiConnectivity
{
public:
  WifiManager();
  ~WifiManager();

  // deviceNamePrefix + the MAC address (colons stripped) becomes the
  // WiFi hostname and is exposed via getDeviceName() for callers that
  // need a unique, stable device identifier (e.g. MQTT topics).
  void setup(const char *ssid, const char *password, const char *deviceNamePrefix);
  bool loop();
  void manageConnection();

  bool isConnected() const override
  {
    return _wifiState == WIFI_CONNECTED;
  }
  bool consumeConnectedEvent() override;
  bool consumeDisconnectedEvent() override;

  WiFiClient *getWifiClient();
  String getDeviceName() const;

private:
  String _ssid;
  String _password;
  String _deviceName;
  WiFiClient _wifiClient;
  unsigned long _nextReconnectAttemptTime = 0;
  enum WifiState
  {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
  };
  WifiState _wifiState = WIFI_DISCONNECTED;
  unsigned long _wifiConnectStartTime = 0;
  uint8_t _reconnectAttempt = 0;
  bool _connectedEventPending = false;
  bool _disconnectedEventPending = false;
};

#endif // WIFIMANAGER_H
