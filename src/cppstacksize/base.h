#ifndef CPPSTACKSIZE_BASE_H
#define CPPSTACKSIZE_BASE_H

#include <cassert>
#include <cstdint>

#define CSS_ASSERT(cond) assert(cond)

namespace cppstacksize {
using S16 = std::int16_t;
using S32 = std::int32_t;
using S64 = std::int64_t;
using S8 = std::int8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
using U8 = std::uint8_t;

template <class Out, class In>
Out narrow_cast(In value) {
  return static_cast<Out>(value);
}
}

#endif
