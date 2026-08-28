// trace.h
#ifndef TRACE_H
#define TRACE_H

#include "config.h"
#include <Arduino.h>

/**
 * Represents different trace levels for logging
 */
enum class TraceLevel
{
  TRACE,   // Detailed trace information
  DEBUG,   // Debug information for development
  INFO,    // General information messages
  WARNING, // Warning messages for recoverable issues
  ERROR,   // Error messages indicating issues
  NONE     // No logging
};

class Trace
{
public:
  static void log(TraceLevel level, String message);
  static void logf(TraceLevel level, const char *format, ...);

  // Helper methods for specific levels
  static void info(String message)
  {
    log(TraceLevel::INFO, message);
  }
  static void error(String message)
  {
    log(TraceLevel::ERROR, message);
  }
  static void debug(String message)
  {
    log(TraceLevel::DEBUG, message);
  }
  static void warning(String message)
  {
    log(TraceLevel::WARNING, message);
  }
  static void trace(String message)
  {
    log(TraceLevel::TRACE, message);
  }

private:
  // Check if the message should be logged based on the configured level
  static bool shouldLog(TraceLevel level)
  {
    return static_cast<int>(level) >= static_cast<int>(TRACE_LEVEL);
  }
};

#endif // TRACE_H
