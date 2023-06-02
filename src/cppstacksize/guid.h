#pragma once

#include <cppstacksize/base.h>
#include <span>
#include <string>

namespace cppstacksize {
class GUID {
 public:
  explicit GUID(std::span<const U8> bytes);

  std::string to_string() const;

 private:
  U8 bytes_[16];
};
}
