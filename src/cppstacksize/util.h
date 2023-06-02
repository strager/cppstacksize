#pragma once

#include <cppstacksize/base.h>

namespace cppstacksize {
inline U64 align_up(U64 n, U64 alignment) {
  U64 mask = alignment - 1;
  return (n + mask) & ~mask;
}
}
