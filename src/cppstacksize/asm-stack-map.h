#pragma once

#include <cppstacksize/base.h>
#include <cppstacksize/register.h>
#include <iosfwd>
#include <span>
#include <vector>

namespace cppstacksize {
enum class Stack_Access_Kind : U8 {
  read_only,
  write_only,
  read_or_write,
  read_and_write,
};

bool is_read(Stack_Access_Kind);
bool is_write(Stack_Access_Kind);

struct Stack_Map_Touch {
  static constexpr Stack_Map_Touch read(U32 offset,
                                        S32 entry_rsp_relative_address,
                                        U32 byte_count) {
    return Stack_Map_Touch{
        .offset = offset,
        .entry_rsp_relative_address = entry_rsp_relative_address,
        .byte_count = byte_count,
        .access_kind = Stack_Access_Kind::read_only,
    };
  }

  static constexpr Stack_Map_Touch write(U32 offset,
                                         S64 entry_rsp_relative_address,
                                         U32 byte_count) {
    return Stack_Map_Touch{
        .offset = offset,
        .entry_rsp_relative_address = entry_rsp_relative_address,
        .byte_count = byte_count,
        .access_kind = Stack_Access_Kind::write_only,
    };
  }

  static constexpr Stack_Map_Touch read_or_write(U32 offset,
                                                 S64 entry_rsp_relative_address,
                                                 U32 byte_count) {
    return Stack_Map_Touch{
        .offset = offset,
        .entry_rsp_relative_address = entry_rsp_relative_address,
        .byte_count = byte_count,
        .access_kind = Stack_Access_Kind::read_or_write,
    };
  }

  static constexpr Stack_Map_Touch read_and_write(
      U32 offset, S64 entry_rsp_relative_address, U32 byte_count) {
    return Stack_Map_Touch{
        .offset = offset,
        .entry_rsp_relative_address = entry_rsp_relative_address,
        .byte_count = byte_count,
        .access_kind = Stack_Access_Kind::read_and_write,
    };
  }

  bool is_read() const { return cppstacksize::is_read(this->access_kind); }

  bool is_write() const { return cppstacksize::is_write(this->access_kind); }

  friend bool operator==(const Stack_Map_Touch&,
                         const Stack_Map_Touch&) = default;

  U32 offset;
  S64 entry_rsp_relative_address;
  U32 byte_count;
  Stack_Access_Kind access_kind;
};

struct Stack_Map {
  Register_File registers;
  std::vector<Stack_Map_Touch> touches;

  void clear() { *this = Stack_Map(); }
};

Stack_Map analyze_x86_64_stack_map(std::span<const U8> code);

std::ostream& operator<<(std::ostream& out, Stack_Access_Kind);
std::ostream& operator<<(std::ostream& out, const Stack_Map_Touch&);
}
