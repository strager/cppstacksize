#include <cppstacksize/reader.h>
#include <iterator>
#include <string>

// TODO(strager): Switch to <format>.
#include <fmt/format.h>

namespace cppstacksize {
std::string Location::to_string() const {
  std::string result;
  fmt::format_to(std::back_inserter(result), "file offset 0x{:x}",
                 this->file_offset);
  if (this->stream_index.has_value() && this->stream_offset.has_value()) {
    if (*this->stream_index == static_cast<U32>(-1)) {
      result = fmt::format("stream directory offset 0x{:x} ({})",
                           *this->stream_offset, result);
    } else {
      result = fmt::format("stream #{} offset 0x{:x} ({})", *this->stream_index,
                           *this->stream_offset, result);
    }
  }
  return result;
}
}
