#ifndef IMESSAGEPUBLISHER_H
#define IMESSAGEPUBLISHER_H

class IMessagePublisher
{
public:
  virtual ~IMessagePublisher() = default;
  virtual bool isConnected() = 0;
  virtual void publish(const char *topic, const char *payload) = 0;
  virtual void publishRetained(const char *topic, const char *payload) = 0;
};

#endif // IMESSAGEPUBLISHER_H
