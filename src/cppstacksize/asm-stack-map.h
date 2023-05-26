#ifndef CPPSTACKSIZE_ASM_STACK_MAP_H
#define CPPSTACKSIZE_ASM_STACK_MAP_H

#include <cstdint>
#include <ostream>
#include <span>
#include <vector>

namespace cppstacksize {
using S32 = std::int32_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
using U8 = std::uint8_t;

struct Stack_Map_Touch {
  explicit Stack_Map_Touch(U32 offset, S32 entry_rsp_relative_address,
                           U32 byte_count)
      : offset(offset),
        entry_rsp_relative_address(entry_rsp_relative_address),
        byte_count(byte_count) {}

  friend bool operator==(const Stack_Map_Touch&,
                         const Stack_Map_Touch&) = default;

  U32 offset;
  S32 entry_rsp_relative_address;
  U32 byte_count;
};

std::ostream& operator<<(std::ostream& out, const Stack_Map_Touch& touch) {
  out << "Stack_Map_Touch{offset=" << touch.offset
      << ", entry_rsp_relative_address=" << touch.entry_rsp_relative_address
      << ", byte_count=" << touch.byte_count << "}";
  return out;
}

struct Stack_Map {
  std::vector<Stack_Map_Touch> touches;
};

Stack_Map analyze_x86_64_stack_map(std::span<const U8> code);
}

#endif
