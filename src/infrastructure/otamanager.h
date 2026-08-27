#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include "global_defines.h"
#include "iotaloopcontrol.h"
#include <ArduinoOTA.h>

// Fail-closed by default: OTA only becomes active if a non-empty password
// is supplied to setup(), unless OTA_ALLOW_INSECURE_NO_PASSWORD is
// explicitly set to true in config.h (isolated dev networks only).
class OtaManager : public IOtaLoopControl
{
public:
  OtaManager();

  void setup(const char *hostname, const char *password = nullptr);
  void loop() override;
  bool isUpdating() const override;
  void setEnabled(bool enabled);
  bool isEnabled() const;

private:
  bool _enabled;
  bool _isUpdating;
  unsigned long _lastProgressUpdate;

  void onStart();
  void onEnd();
  void onProgress(unsigned int progress, unsigned int total);
  void onError(ota_error_t error);

  const char *getErrorString(ota_error_t error);
};

#endif // OTAMANAGER_H
