#include "otamanager.h"
#include "trace.h"

OtaManager::OtaManager()
    : _enabled(ENABLE_OTA), _isUpdating(false), _lastProgressUpdate(0)
{
}

void OtaManager::setup(const char *hostname, const char *password)
{
  // Global OTA kill-switch from configuration.
  if (!_enabled)
  {
    Trace::log(TraceLevel::INFO, "OTA is disabled");
    return;
  }

  Trace::log(TraceLevel::INFO, "Setting up OTA...");

  ArduinoOTA.setHostname(hostname);

  // Security policy:
  // - Preferred: password configured -> protected OTA.
  // - Fail-closed default: no password means OTA is disabled.
  // - Explicit insecure mode can be enabled for isolated dev networks only.
  const bool hasPassword = (password != nullptr && strlen(password) > 0);
  if (hasPassword)
  {
    ArduinoOTA.setPassword(password);
    Trace::log(TraceLevel::DEBUG, "OTA password protection enabled");
  }
  else if (!OTA_ALLOW_INSECURE_NO_PASSWORD)
  {
    Trace::log(TraceLevel::WARNING,
               "OTA disabled: no OTA password configured (fail-closed)");
    _enabled = false;
    return;
  }
  else
  {
    Trace::log(TraceLevel::WARNING,
               "OTA running without password (explicitly allowed)");
  }

  ArduinoOTA.setPort(OTA_PORT);

  ArduinoOTA.onStart([this]() { this->onStart(); });
  ArduinoOTA.onEnd([this]() { this->onEnd(); });
  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total)
                        { this->onProgress(progress, total); });
  ArduinoOTA.onError([this](ota_error_t error) { this->onError(error); });

  ArduinoOTA.begin();

  Trace::log(TraceLevel::INFO, "OTA initialized successfully");
  Trace::logf(TraceLevel::DEBUG, "OTA Hostname: %s", hostname);
  Trace::logf(TraceLevel::DEBUG, "OTA Port: %u", OTA_PORT);
}

void OtaManager::loop()
{
  // Must be called frequently from the main loop, otherwise OTA sessions
  // can stall or time out.
  if (!_enabled)
  {
    return;
  }

  ArduinoOTA.handle();
}

bool OtaManager::isUpdating() const
{
  return _isUpdating;
}

void OtaManager::setEnabled(bool enabled)
{
  _enabled = enabled;
  Trace::log(TraceLevel::INFO, enabled ? "OTA enabled" : "OTA disabled");
}

bool OtaManager::isEnabled() const
{
  return _enabled;
}

void OtaManager::onStart()
{
  _isUpdating = true;
  _lastProgressUpdate = millis();

  String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
  Trace::log(TraceLevel::INFO, "OTA Update Started - Type: " + type);
}

void OtaManager::onEnd()
{
  _isUpdating = false;
  Trace::log(TraceLevel::INFO, "OTA Update Completed Successfully");
  Trace::log(TraceLevel::INFO, "Device will restart...");
}

void OtaManager::onProgress(unsigned int progress, unsigned int total)
{
  unsigned long currentTime = millis();

  // Rate limit progress logs to reduce serial overhead and log noise.
  if (currentTime - _lastProgressUpdate >= 1000)
  {
    _lastProgressUpdate = currentTime;
    unsigned int percentage = (total > 0) ? ((progress * 100U) / total) : 0;
    Trace::logf(TraceLevel::INFO, "OTA Progress: %u%% (%u/%u bytes)",
                percentage, progress, total);
  }
}

void OtaManager::onError(ota_error_t error)
{
  _isUpdating = false;
  Trace::logf(TraceLevel::ERROR, "OTA Update Error: %s", getErrorString(error));
}

const char *OtaManager::getErrorString(ota_error_t error)
{
  switch (error)
  {
  case OTA_AUTH_ERROR:
    return "Authentication Failed";
  case OTA_BEGIN_ERROR:
    return "Begin Failed";
  case OTA_CONNECT_ERROR:
    return "Connect Failed";
  case OTA_RECEIVE_ERROR:
    return "Receive Failed";
  case OTA_END_ERROR:
    return "End Failed";
  default:
    return "Unknown Error";
  }
}
