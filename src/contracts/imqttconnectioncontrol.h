#ifndef IMQTTCONNECTIONCONTROL_H
#define IMQTTCONNECTIONCONTROL_H

class IMqttConnectionControl
{
public:
  virtual ~IMqttConnectionControl() = default;
  virtual void requestConnect() = 0;
  virtual void forceDisconnect() = 0;
};

#endif // IMQTTCONNECTIONCONTROL_H
