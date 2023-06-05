#pragma once

#include <cppstacksize/base.h>
#include <span>
#include <string>

namespace cppstacksize {
class Loaded_File {
 public:
  static Loaded_File load(const char* path);

  Loaded_File(const Loaded_File&) = delete;
  Loaded_File& operator=(const Loaded_File&) = delete;

  Loaded_File(Loaded_File&&) = default;
  Loaded_File& operator=(Loaded_File&&) = default;

  std::span<const U8> data() const noexcept;

 private:
  explicit Loaded_File(std::string&& data);

  std::string data_;
};
}
