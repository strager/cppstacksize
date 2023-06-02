#include <algorithm>
#include <cppstacksize/base.h>
#include <cppstacksize/guid.h>
#include <cppstacksize/reader.h>
#include <span>

// TODO(strager): Switch to <format>.
#include <fmt/format.h>

namespace cppstacksize {
GUID::GUID(std::span<const U8> bytes) {
  CSS_ASSERT(bytes.size() == 16);
  std::copy(bytes.begin(), bytes.end(), this->bytes_);
}

std::string GUID::to_string() const {
  Span_Reader reader(this->bytes_);
  return fmt::format(
      "{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
      reader.u32(0), reader.u16(4), reader.u16(6), reader.u8(8), reader.u8(9),
      reader.u8(10), reader.u8(11), reader.u8(12), reader.u8(13), reader.u8(14),
      reader.u8(15));
}
}
