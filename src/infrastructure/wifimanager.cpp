#include "wifimanager.h"
#include "reconnectpolicy.h"

WifiManager::WifiManager()
{
}

WifiManager::~WifiManager()
{
}

void WifiManager::setup(const char *ssid, const char *password, const char *deviceNamePrefix)
{
  // Store credentials for reconnect attempts and diagnostics.
  _ssid = ssid;
  _password = password;

  // Configure station mode before touching WiFi.macAddress()/setHostname().
  WiFi.mode(WIFI_STA);

  // Build the unique device name from the MAC address so multiple boards
  // on the same network never collide (hostname, OTA hostname, MQTT
  // topics/client-id all derive from this).
  _deviceName = String(deviceNamePrefix) + WiFi.macAddress();
  _deviceName.replace(":", "");
  WiFi.setHostname(_deviceName.c_str());
  randomSeed(micros());

  // Kick off first connect explicitly. We do not rely on implicit connect
  // behavior or event ordering side effects.
  WiFi.begin(_ssid.c_str(), _password.c_str());
  _wifiConnectStartTime = millis();
  _wifiState = WIFI_CONNECTING;
  _nextReconnectAttemptTime = _wifiConnectStartTime;
  _connectedEventPending = false;
  _disconnectedEventPending = false;

  Trace::log(TraceLevel::DEBUG, "WiFi setup complete. Device name: " + _deviceName);
}

bool WifiManager::consumeConnectedEvent()
{
  bool hadEvent = _connectedEventPending;
  _connectedEventPending = false;
  return hadEvent;
}

bool WifiManager::consumeDisconnectedEvent()
{
  bool hadEvent = _disconnectedEventPending;
  _disconnectedEventPending = false;
  return hadEvent;
}

WiFiClient *WifiManager::getWifiClient()
{
  return &_wifiClient;
}

String WifiManager::getDeviceName() const
{
  return _deviceName;
}

bool WifiManager::loop()
{
  const bool isConnectedNow = (WiFi.status() == WL_CONNECTED);

  // Track connection edges via polling to stay compatible across cores.
  if (isConnectedNow && _wifiState != WIFI_CONNECTED)
  {
    const IPAddress ip = WiFi.localIP();
    Trace::logf(TraceLevel::INFO, "WiFi connected, IP: %u.%u.%u.%u", ip[0],
                ip[1], ip[2], ip[3]);
    _wifiState = WIFI_CONNECTED;
    _connectedEventPending = true;
    _disconnectedEventPending = false;
    _reconnectAttempt = 0;
  }
  else if (!isConnectedNow && _wifiState == WIFI_CONNECTED)
  {
    Trace::log(TraceLevel::INFO,
               "WiFi disconnected, attempting to reconnect...");
    _wifiState = WIFI_DISCONNECTED;
    _disconnectedEventPending = true;
    _connectedEventPending = false;
    _nextReconnectAttemptTime = millis();
  }

  // Safety net: if the initial connect stalls, move to DISCONNECTED so the
  // managed reconnect (with backoff) kicks in instead of waiting forever.
  if (_wifiState == WIFI_CONNECTING && !isConnectedNow &&
      (millis() - _wifiConnectStartTime) >= WIFI_CONNECTION_TIMEOUT)
  {
    Trace::log(TraceLevel::WARNING,
               "WiFi connect timeout, scheduling reconnect");
    _wifiState = WIFI_DISCONNECTED;
    _nextReconnectAttemptTime = millis();
  }

  // Fast-path: transition CONNECTING -> CONNECTED as soon as the link is up.
  if (_wifiState == WIFI_CONNECTING && isConnectedNow)
  {
    Trace::logf(TraceLevel::DEBUG,
                "WiFi connected after connection attempt %u",
                _reconnectAttempt);
    _wifiState = WIFI_CONNECTED;
    _reconnectAttempt = 0;
  }
  else if (_wifiState == WIFI_DISCONNECTED &&
           millis() >= _nextReconnectAttemptTime)
  {
    // Reconnect attempt is due according to the scheduled backoff window.
    manageConnection();
  }

  return _wifiState == WIFI_CONNECTED;
}

void WifiManager::manageConnection()
{
  // Retry budget is bounded to avoid infinite tight reconnect loops.
  if (_reconnectAttempt < WIFI_MAX_RECONNECT_ATTEMPTS)
  {
    Trace::log(TraceLevel::INFO, "Attempting to reconnect to WiFi...");
    WiFi.disconnect();
    WiFi.begin(_ssid.c_str(), _password.c_str());
    _wifiConnectStartTime = millis();
    _reconnectAttempt++;

    // Random jitter prevents synchronized reconnect storms across a fleet
    // of boards that all lost connectivity at the same time.
    const uint32_t jitter = static_cast<uint32_t>(
      random(static_cast<long>(WIFI_RECONNECT_JITTER_MS) + 1L));
    const uint32_t reconnectDelayMs = ReconnectPolicy::computeDelayMs(
        _reconnectAttempt, WIFI_RECONNECT_BASE_DELAY_MS,
        WIFI_RECONNECT_MAX_DELAY_MS, jitter);
    _nextReconnectAttemptTime = _wifiConnectStartTime + reconnectDelayMs;

    Trace::logf(TraceLevel::DEBUG, "Next reconnect in ms: %lu",
                static_cast<unsigned long>(reconnectDelayMs));
  }
  else
  {
    Trace::log(TraceLevel::ERROR, "Max reconnection attempts reached");
#if WIFI_RESTART_ON_RECONNECT_FAILURE
    Trace::log(TraceLevel::WARNING, "Restarting due to reconnect policy");
    delay(1000);
    ESP.restart();
#else
    // Default diagnostics-first mode: stay alive for serial/OTA/webserver
    // diagnostics and retry again later instead of forcing a reboot.
    Trace::log(TraceLevel::WARNING,
               "Keeping device alive for diagnostics (no forced restart)");
    _nextReconnectAttemptTime = millis() + WIFI_RECONNECT_MAX_DELAY_MS;
    _reconnectAttempt = 0;
#endif
  }
}
