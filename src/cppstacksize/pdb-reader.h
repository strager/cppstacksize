#pragma once

#include <cppstacksize/base.h>
#include <cppstacksize/reader.h>
#include <optional>
#include <span>
#include <vector>

namespace cppstacksize {
template <class Base_Reader_T>
class PDB_Blocks_Reader : public Reader_Base<PDB_Blocks_Reader<Base_Reader_T>> {
 public:
  using Base_Reader = Base_Reader_T;

  explicit PDB_Blocks_Reader(const Base_Reader* base_reader,
                             std::vector<U32> block_indexes, U32 block_size,
                             U32 byte_size, U32 stream_index)
      : base_reader_(base_reader),
        block_indexes_(std::move(block_indexes)),
        block_size_(block_size),
        byte_size_(byte_size),
        stream_index_(stream_index) {}

  U64 size() const { return this->byte_size_; }

  std::span<const U32> blocks() const { return this->block_indexes_; }

  Location locate(U64 offset) const {
    U32 block_index = this->block_indexes_[offset / this->block_size_];
    U64 relative_offset = offset % this->block_size_;
    Location location = this->base_reader_->locate(
        block_index * this->block_size_ + relative_offset);
    location.stream_index = this->stream_index_;
    location.stream_offset = offset;
    return location;
  }

  U8 u8(U64 offset) const {
    this->check_bounds(offset, 1);

    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    U32 block_index = this->block_indexes_[offset / this->block_size_];
    U64 relative_offset = offset % this->block_size_;
    return this->base_reader_->u8(block_index * this->block_size_ +
                                  relative_offset);
  }

#define CSS_DEFINE_READ_FUNCTION(type, name, size)                       \
  type name(U64 offset) const {                                          \
    this->check_bounds(offset, size);                                    \
                                                                         \
    /* TODO(strager): Avoid divisions. */                                \
    /* TODO(strager): Avoid multiplications. */                          \
    U64 block_index_index = offset / this->block_size_;                  \
    U32 block_index = this->block_indexes_[block_index_index];           \
    U64 end_block_index_index = (offset + size - 1) / this->block_size_; \
    if (block_index_index == end_block_index_index) {                    \
      U64 relative_offset = offset % this->block_size_;                  \
      return this->base_reader_->name(block_index * this->block_size_ +  \
                                      relative_offset);                  \
    } else {                                                             \
      throw std::runtime_error("not yet implemented: reading " #type     \
                               " straddling multiple blocks");           \
    }                                                                    \
  }

  CSS_DEFINE_READ_FUNCTION(U16, u16, 2)
  CSS_DEFINE_READ_FUNCTION(U32, u32, 4)

#undef CSS_DEFINE_READ_FUNCTION

  std::optional<U64> find_u8(U8 b, U64 offset) const {
    return this->find_u8(b, offset, this->byte_size_);
  }

  std::optional<U64> find_u8(U8 b, U64 offset, U64 end_offset) const {
    if (end_offset > this->byte_size_) {
      end_offset = this->byte_size_;
    }
    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    U64 begin_block_index_index = (offset / this->block_size_);
    U64 end_block_index_index = (end_offset - 1) / this->block_size_;
    U64 relative_offset = offset % this->block_size_;

    for (U64 block_index_index = begin_block_index_index;
         block_index_index <= end_block_index_index; ++block_index_index) {
      // TODO(strager): Bounds check.
      U32 block_index = this->block_indexes_[block_index_index];

      // TODO(strager): Properly bounds check.
      std::optional<U64> i = this->base_reader_->find_u8(
          b, block_index * this->block_size_ + relative_offset,
          (block_index + 1) * this->block_size_);
      if (i.has_value()) {
        return (*i % this->block_size_) + block_index_index * this->block_size_;
      }
      relative_offset = 0;
    }
    return std::nullopt;
  }

  template <class Callback>
  void enumerate_bytes(U64 offset, U64 size, Callback callback) const {
    this->check_bounds(offset, size);
    U64 end_offset = offset + size;
    // TODO(strager): Avoid divisions.
    // TODO(strager): Avoid multiplications.
    U64 begin_block_index_index = offset / this->block_size_;
    U64 end_block_index_index = end_offset / this->block_size_;
    U64 relative_offset = offset % this->block_size_;

    for (U64 block_index_index = begin_block_index_index;
         block_index_index <= end_block_index_index; ++block_index_index) {
      U32 block_index = this->block_indexes_[block_index_index];

      bool is_last_block = block_index_index == end_block_index_index;
      U64 size_needed_in_block =
          (is_last_block ? end_offset & (this->block_size_ - 1)
                         : this->block_size_) -
          relative_offset;

      this->base_reader_->enumerate_bytes(
          block_index * this->block_size_ + relative_offset,
          size_needed_in_block, callback);
      relative_offset = 0;
    }
  }

 private:
  const Base_Reader* base_reader_;
  std::vector<U32> block_indexes_;
  U32 block_size_;
  U32 byte_size_;
  U32 stream_index_;
};
}
