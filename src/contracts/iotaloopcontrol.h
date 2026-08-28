#ifndef IOTALOOPCONTROL_H
#define IOTALOOPCONTROL_H

class IOtaLoopControl
{
public:
  virtual ~IOtaLoopControl() = default;
  virtual void loop() = 0;
  virtual bool isUpdating() const = 0;
};

#endif // IOTALOOPCONTROL_H
