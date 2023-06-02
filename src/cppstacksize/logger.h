#pragma once

#include <cppstacksize/reader.h>
#include <string_view>

namespace cppstacksize {
class Logger {
 public:
  virtual void log(std::string_view message, const Location& location) = 0;
};

class Console_Logger : public Logger {
 public:
  static Console_Logger instance;

  void log(std::string_view message, const Location& location) override;
};

extern Logger& fallback_logger;
}
