#ifndef CPPSTACKSIZE_READER_H
#define CPPSTACKSIZE_READER_H

#include <algorithm>
#include <cppstacksize/base.h>
#include <exception>
#include <optional>
#include <span>
#include <string>

namespace cppstacksize {
class Out_Of_Bounds_Read : public std::exception {};

class C_String_Null_Terminator_Not_Found : public std::exception {};

// Mixin class using CRTP.
template <class Derived>
class Reader_Base {
 public:
  std::u8string fixed_width_string(U64 offset, U64 size) {
    std::optional<U64> string_end_offset =
        this->derived()->find_u8(0, offset, offset + size);
    if (string_end_offset.has_value()) {
      size = *string_end_offset - offset;
    }
    return this->derived()->utf_8_string(offset, size);
  }

  std::u8string utf_8_c_string(U64 offset) {
    std::optional<U64> end_offset = this->derived()->find_u8(0, offset);
    if (!end_offset.has_value()) {
      throw C_String_Null_Terminator_Not_Found();
    }
    return this->derived()->utf_8_string(offset, *end_offset - offset);
  }

  std::u8string utf_8_string(U64 offset, U64 size) {
    std::u8string result;
    this->derived()->enumerate_bytes(
        offset, size, [&](std::span<const U8> chunk) -> void {
          result.append(chunk.begin(), chunk.end());
        });
    return result;
  }

  void copy_bytes_into(std::span<U8> out, U64 offset) {
    this->derived()->enumerate_bytes(
        offset, out.size(), [&](std::span<const U8> chunk) -> void {
          std::copy(chunk.begin(), chunk.end(), out.begin());
          out = out.subspan(chunk.size());
        });
  }

 private:
  Derived* derived() noexcept { return static_cast<Derived*>(this); }
};

class Span_Reader : public Reader_Base<Span_Reader> {
 public:
  explicit Span_Reader(std::span<const U8> data) : data_(data) {}

  U64 size() const { return this->data_.size(); }

  U8 u8(U64 offset) {
    this->check_bounds(offset, 1);
    return this->data_[offset];
  }

  U16 u16(U64 offset) {
    this->check_bounds(offset, 2);
    return (static_cast<U16>(this->data_[offset + 0]) << (0 * 8)) |
           (static_cast<U16>(this->data_[offset + 1]) << (1 * 8));
  }

  U32 u32(U64 offset) {
    this->check_bounds(offset, 4);
    return (static_cast<U32>(this->data_[offset + 0]) << (0 * 8)) |
           (static_cast<U32>(this->data_[offset + 1]) << (1 * 8)) |
           (static_cast<U32>(this->data_[offset + 2]) << (2 * 8)) |
           (static_cast<U32>(this->data_[offset + 3]) << (3 * 8));
  }

  // Searches for a byte equal b starting from offset.
  //
  // Returns the offset of the first match, or null if there is no match.
  std::optional<U64> find_u8(U8 b, U64 offset) {
    return this->find_u8(b, offset, this->size());
  }

  std::optional<U64> find_u8(U8 b, U64 offset, U64 end_offset) {
    if (end_offset > this->size()) {
      end_offset = this->size();
    }
    U64 i = offset;
    for (;;) {
      if (i >= end_offset) {
        return std::nullopt;
      }
      if (this->data_[i] == b) {
        return i;
      }
      i += 1;
    }
  }

  template <class Callback>
  void enumerate_bytes(U64 offset, U64 size, Callback callback) {
    this->check_bounds(offset, size);
    callback(this->data_.subspan(offset, size));
  }

 private:
  void check_bounds(U64 offset, U64 size) {
    U64 last_offset = offset + size - 1;
    if (last_offset >= this->size() || last_offset < offset) {
      throw Out_Of_Bounds_Read();
    }
  }

  std::span<const U8> data_;
};
}

#endif
