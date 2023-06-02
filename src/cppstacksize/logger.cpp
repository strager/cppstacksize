#include <cppstacksize/logger.h>
#include <cppstacksize/reader.h>
#include <cstdio>
#include <string_view>

namespace cppstacksize {
Logger& fallback_logger = Console_Logger::instance;

Console_Logger Console_Logger::instance;

void Console_Logger::log(std::string_view message, const Location& location) {
  std::string location_string = location.to_string();
  std::fprintf(stderr, "%.*s: %.*s\n", narrow_cast<int>(location_string.size()),
               location_string.data(), narrow_cast<int>(message.size()),
               message.data());
}
}
