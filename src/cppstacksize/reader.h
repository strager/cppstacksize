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

struct Location {
  U64 file_offset;
  std::optional<U32> stream_index = std::nullopt;
  std::optional<U32> stream_offset = std::nullopt;

  std::string to_string();
};

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

 protected:
  void check_bounds(U64 offset, U64 size) {
    U64 last_offset = size == 0 ? offset : offset + size - 1;
    if (last_offset >= this->derived()->size() || last_offset < offset) {
      throw Out_Of_Bounds_Read();
    }
  }

 private:
  Derived* derived() noexcept { return static_cast<Derived*>(this); }
};

class Span_Reader : public Reader_Base<Span_Reader> {
 public:
  explicit Span_Reader(std::span<const U8> data) : data_(data) {}

  U64 size() const { return this->data_.size(); }

  Location locate(U64 offset) { return Location{.file_offset = offset}; }

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
  std::span<const U8> data_;
};

template <class Base_Reader_T>
class Sub_File_Reader : public Reader_Base<Sub_File_Reader<Base_Reader_T>> {
 public:
  using Base_Reader = Base_Reader_T;

  explicit Sub_File_Reader(Base_Reader* base_reader, U64 offset)
      : Sub_File_Reader(base_reader, offset, base_reader->size() - offset) {}

  explicit Sub_File_Reader(Base_Reader* base_reader, U64 offset, U64 size) {
    // TODO(port): Combine nested Sub_File_Reader-s.
    this->base_reader_ = base_reader;
    // TODO(strager): Ensure offset does not exceed base_reader->size().
    this->sub_file_offset_ = offset;
    if (offset + size >= this->base_reader_->size()) {
      this->sub_file_size_ = this->base_reader_->size() - offset;
    } else {
      this->sub_file_size_ = size;
    }
  }

  Base_Reader* base_reader() { return this->base_reader_; }
  U64 sub_file_offset() { return this->sub_file_offset_; }

  U64 size() { return this->sub_file_size_; }

  Location locate(U64 offset) {
    return this->base_reader_->locate(offset + this->sub_file_offset_);
  }

  U8 u8(U64 offset) {
    this->check_bounds(offset, 1);
    return this->base_reader_->u8(offset + this->sub_file_offset_);
  }

  U16 u16(U64 offset) {
    this->check_bounds(offset, 2);
    return this->base_reader_->u16(offset + this->sub_file_offset_);
  }

  U32 u32(U64 offset) {
    this->check_bounds(offset, 4);
    return this->base_reader_->u32(offset + this->sub_file_offset_);
  }

  std::u8string utf_8_string(U64 offset, U64 size) {
    this->check_bounds(offset, size);
    return this->base_reader_->utf_8_string(offset + this->sub_file_offset_,
                                            size);
  }

  std::optional<U64> find_u8(U8 b, U64 offset) {
    return this->find_u8(b, offset, this->size());
  }

  std::optional<U64> find_u8(U8 b, U64 offset, U64 end_offset) {
    if (offset >= this->sub_file_size_) {
      return std::nullopt;
    }
    U64 sub_file_end_offset = this->sub_file_offset_ + this->sub_file_size_;
    end_offset = end_offset + this->sub_file_offset_;
    if (end_offset > sub_file_end_offset) {
      end_offset = sub_file_end_offset;
    }

    std::optional<U64> i = this->base_reader_->find_u8(
        b, offset + this->sub_file_offset_, end_offset);
    if (!i.has_value()) {
      return std::nullopt;
    }
    return *i - this->sub_file_offset_;
  }

  template <class Callback>
  void enumerate_bytes(U64 offset, U64 size, Callback callback) {
    this->check_bounds(offset, size);
    this->base_reader_->enumerate_bytes(offset + this->sub_file_offset_, size,
                                        callback);
  }

 private:
  Base_Reader* base_reader_;
  U64 sub_file_offset_;
  U64 sub_file_size_;
};
}

#endif
