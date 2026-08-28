// trace.cpp
#include "trace.h"
#include <stdarg.h>
#include <stdio.h>

void Trace::log(TraceLevel level, String message)
{
  // Only log if the message level is >= the configured trace level
  if (!shouldLog(level))
  {
    return;
  }

  String prefix;
  switch (level)
  {
  case TraceLevel::TRACE:
    prefix = "[TRACE] ";
    break;
  case TraceLevel::INFO:
    prefix = "[INFO] ";
    break;
  case TraceLevel::WARNING:
    prefix = "[WARNING] ";
    break;
  case TraceLevel::DEBUG:
    prefix = "[DEBUG] ";
    break;
  case TraceLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  default:
    prefix = "[LOG] ";
    break;
  }

  Serial.println(prefix + message);
}

void Trace::logf(TraceLevel level, const char *format, ...)
{
  // Formatted logging variant:
  // avoids repeated dynamic String concatenation in hot paths.
  if (!shouldLog(level))
  {
    return;
  }

  const char *prefix = "[LOG] ";
  switch (level)
  {
  case TraceLevel::TRACE:
    prefix = "[TRACE] ";
    break;
  case TraceLevel::INFO:
    prefix = "[INFO] ";
    break;
  case TraceLevel::WARNING:
    prefix = "[WARNING] ";
    break;
  case TraceLevel::DEBUG:
    prefix = "[DEBUG] ";
    break;
  case TraceLevel::ERROR:
    prefix = "[ERROR] ";
    break;
  default:
    break;
  }

  // Fixed-size stack buffer keeps memory behavior predictable on MCU.
  // Messages longer than buffer are truncated by vsnprintf.
  char messageBuffer[160];
  va_list args;
  va_start(args, format);
  vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
  va_end(args);

  Serial.print(prefix);
  Serial.println(messageBuffer);
}
