#pragma once

#include <cppstacksize/base.h>
#include <optional>
#include <string>

namespace cppstacksize {
inline U64 align_up(U64 n, U64 alignment) {
  U64 mask = alignment - 1;
  return (n + mask) & ~mask;
}

template <class T>
T* get(std::optional<T>& o) {
  return o.has_value() ? &*o : nullptr;
}

inline std::string u8string_to_string(const std::u8string& s) {
  return std::string(reinterpret_cast<const char*>(s.data()), s.size());
}
}
