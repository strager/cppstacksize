#include <cppstacksize/asm-stack-map.h>
#include <ostream>

namespace cppstacksize {
std::ostream& operator<<(std::ostream& out, Stack_Access_Kind sak) {
  switch (sak) {
    case Stack_Access_Kind::read_only:
      out << "read_only";
      break;
    case Stack_Access_Kind::write_only:
      out << "write_only";
      break;
    case Stack_Access_Kind::read_or_write:
      out << "read_or_write";
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
}
