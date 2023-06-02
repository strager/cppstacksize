#pragma once

#include <cppstacksize/base.h>
#include <span>
#include <string>

namespace cppstacksize {
class Loaded_File {
 public:
  static Loaded_File load(const char* path);

  std::span<const U8> data() const noexcept;

 private:
  explicit Loaded_File(std::string&& data);

  std::string data_;
};
}
