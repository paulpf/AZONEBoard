#include "bootstrap.h"

Bootstrap bootstrap;
Application &app = bootstrap.application();

void setup()
{
  app.setup();
}

void loop()
{
  app.loop();
}
