#ifndef CPPSTACKSIZE_ASM_STACK_MAP_H
#define CPPSTACKSIZE_ASM_STACK_MAP_H

#include <cstdint>
#include <ostream>
#include <span>
#include <vector>

namespace cppstacksize {
using S32 = std::int32_t;
using S64 = std::int64_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
using U8 = std::uint8_t;

enum class Stack_Access_Kind : U8 {
  read,
  write,
};

struct Stack_Map_Touch {
  static Stack_Map_Touch read(U32 offset, S32 entry_rsp_relative_address,
                              U32 byte_count) {
    return Stack_Map_Touch{
        .offset = offset,
        .entry_rsp_relative_address = entry_rsp_relative_address,
        .byte_count = byte_count,
        .access_kind = Stack_Access_Kind::read,
    };
  }

  static Stack_Map_Touch write(U32 offset, S64 entry_rsp_relative_address,
                               U32 byte_count) {
    return Stack_Map_Touch{
        .offset = offset,
        .entry_rsp_relative_address = entry_rsp_relative_address,
        .byte_count = byte_count,
        .access_kind = Stack_Access_Kind::write,
    };
  }

  friend bool operator==(const Stack_Map_Touch&,
                         const Stack_Map_Touch&) = default;

  U32 offset;
  S64 entry_rsp_relative_address;
  U32 byte_count;
  Stack_Access_Kind access_kind;
};

std::ostream& operator<<(std::ostream& out, Stack_Access_Kind sak) {
  switch (sak) {
    case Stack_Access_Kind::read:
      out << "read";
      break;
    case Stack_Access_Kind::write:
      out << "write";
      break;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const Stack_Map_Touch& touch) {
  out << "Stack_Map_Touch{offset=" << touch.offset
      << ", entry_rsp_relative_address=" << touch.entry_rsp_relative_address
      << ", byte_count=" << touch.byte_count
      << ", access_kind=" << touch.access_kind << "}";
  return out;
}

struct Stack_Map {
  std::vector<Stack_Map_Touch> touches;
};

Stack_Map analyze_x86_64_stack_map(std::span<const U8> code);
}

#endif
