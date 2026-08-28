#ifndef IWIFICONNECTIVITY_H
#define IWIFICONNECTIVITY_H

class IWifiConnectivity
{
public:
  virtual ~IWifiConnectivity() = default;
  virtual bool consumeConnectedEvent() = 0;
  virtual bool consumeDisconnectedEvent() = 0;
  virtual bool isConnected() const = 0;
};

#endif // IWIFICONNECTIVITY_H
