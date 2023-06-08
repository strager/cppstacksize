#include <cppstacksize/line-tables.h>
#include <ostream>

namespace cppstacksize {
std::ostream& operator<<(std::ostream& out, const Line_Source_Info& info) {
  out << "{line=" << info.line_number << "}";
  return out;
}
}
