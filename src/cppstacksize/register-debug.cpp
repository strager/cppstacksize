#include <cppstacksize/register.h>
#include <ostream>

namespace cppstacksize {
std::ostream& operator<<(std::ostream& out, const Register_Value& value) {
  switch (value.kind) {
    case Register_Value_Kind::unknown:
      out << "unknown";
      break;
    case Register_Value_Kind::entry_rsp_relative:
      out << "entry_rsp_relative(" << value.entry_rsp_relative_offset << ")";
      break;
    case Register_Value_Kind::literal:
      out << value.literal;
      break;
  }
  out << " @" << value.last_update_offset;
  return out;
}
}
