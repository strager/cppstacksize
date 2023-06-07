#pragma once

#include <cppstacksize/base.h>
#include <cppstacksize/reader.h>
#include <deque>
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

struct Captured_Log_Message {
  Location location;
  std::string message;
};

/// Captures log messages, and also forwards them to a base logger.
class Capturing_Logger : public Logger {
 public:
  explicit Capturing_Logger(Logger* base_logger) {
    CSS_ASSERT(base_logger != nullptr);
    this->base_logger_ = base_logger;
  }

  bool did_log_message() const { return !this->messages_.empty(); }

  const std::deque<Captured_Log_Message>& logged_messages() const {
    return this->messages_;
  }

  std::string get_logged_messages_string_for_tool_tip() {
    std::string result;
    bool need_newline = false;
    for (const Captured_Log_Message& message : this->messages_) {
      if (need_newline) {
        result += '\n';
      }
      result += message.message;
      need_newline = true;
    }
    return result;
  }

  void log(std::string_view message, const Location& location) override {
    this->messages_.push_back(Captured_Log_Message{
        .location = location,
        .message = std::string(message),
    });
    this->base_logger_->log(message, location);
  }

 private:
  Logger* base_logger_;
  std::deque<Captured_Log_Message> messages_;
};

extern Logger& fallback_logger;
}
