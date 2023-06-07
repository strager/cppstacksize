#include <cppstacksize/base.h>
#include <cppstacksize/pdb-reader.h>
#include <cppstacksize/reader.h>
#include <deque>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>

using ::testing::ElementsAreArray;

namespace cppstacksize {
namespace {
template <class Reader_Factory>
class Test_Reader : public ::testing::Test {
 protected:
  auto make_reader(std::span<const U8> data) {
    return this->reader_factory_.make_reader(data);
  }

  Reader_Factory reader_factory_;
};

struct Span_Reader_Factory {
  Span_Reader make_reader(std::span<const U8> data) {
    return Span_Reader(data);
  }
};

struct Sub_File_Reader_With_Full_Span_Reader {
  Sub_File_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    Span_Reader* base_reader = &this->base_readers_.emplace_back(data);
    return Sub_File_Reader<Span_Reader>(base_reader, 0, base_reader->size());
  }
  std::deque<Span_Reader> base_readers_;
};

struct Sub_File_Reader_With_Full_Span_Reader_And_Implicit_Size {
  Sub_File_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    Span_Reader* base_reader = &this->base_readers_.emplace_back(data);
    return Sub_File_Reader<Span_Reader>(base_reader, 0);
  }
  std::deque<Span_Reader> base_readers_;
};

struct Sub_File_Reader_With_Full_Span_Reader_And_Too_Big_Size {
  Sub_File_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    Span_Reader* base_reader = &this->base_readers_.emplace_back(data);
    return Sub_File_Reader<Span_Reader>(base_reader, 0, 100);
  }
  std::deque<Span_Reader> base_readers_;
};

struct Sub_File_Reader_With_Partial_Span_Reader {
  Sub_File_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    std::vector<U8>& all_bytes = this->datas_.emplace_back();
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.insert(all_bytes.end(), data.begin(), data.end());
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.push_back(0x00);

    Span_Reader* base_reader = &this->base_readers_.emplace_back(all_bytes);
    return Sub_File_Reader<Span_Reader>(base_reader, 4, data.size());
  }
  std::deque<std::vector<U8>> datas_;
  std::deque<Span_Reader> base_readers_;
};

struct Sub_File_Reader_Inside_Sub_File_Reader_With_Span_Reader {
  Sub_File_Reader<Sub_File_Reader<Span_Reader>> make_reader(
      std::span<const U8> data) {
    std::vector<U8>& all_bytes = this->datas_.emplace_back();
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.insert(all_bytes.end(), data.begin(), data.end());
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.push_back(0x00);

    Span_Reader* base_reader = &this->base_readers_.emplace_back(all_bytes);
    Sub_File_Reader<Span_Reader>* inner_reader =
        &this->inner_readers_.emplace_back(base_reader, 2, data.size() + 5);
    Sub_File_Reader<Sub_File_Reader<Span_Reader>> outer_reader(inner_reader, 2,
                                                               data.size());
    return outer_reader;
  }
  std::deque<std::vector<U8>> datas_;
  std::deque<Span_Reader> base_readers_;
  std::deque<Sub_File_Reader<Span_Reader>> inner_readers_;
};

struct Sub_File_Reader_With_Partial_Span_Reader_And_Implicit_Size {
  Sub_File_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    std::vector<U8>& all_bytes = this->datas_.emplace_back();
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.insert(all_bytes.end(), data.begin(), data.end());

    Span_Reader* base_reader = &this->base_readers_.emplace_back(all_bytes);
    return Sub_File_Reader<Span_Reader>(base_reader, 4);
  }
  std::deque<std::vector<U8>> datas_;
  std::deque<Span_Reader> base_readers_;
};

struct PDB_Blocks_Reader_With_Block_Size_4_With_Span_Reader {
  PDB_Blocks_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    Span_Reader* base_reader = &this->base_readers_.emplace_back(data);
    U32 block_size = 4;
    std::vector<U32> block_indexes;
    for (U32 i = 0; i * block_size < base_reader->size(); ++i) {
      block_indexes.push_back(i);
    }
    return PDB_Blocks_Reader<Span_Reader>(base_reader, block_indexes,
                                          block_size, base_reader->size(),
                                          /*streamIndex=*/0);
  }
  std::deque<Span_Reader> base_readers_;
};

struct PDB_Blocks_Reader_With_Block_Size_4_With_Subset_Of_Span_Reader {
  PDB_Blocks_Reader<Span_Reader> make_reader(std::span<const U8> data) {
    std::vector<U8>& all_bytes = this->datas_.emplace_back();
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.insert(all_bytes.end(), data.begin(), data.end());
    all_bytes.push_back(0xcc);
    all_bytes.push_back(0xdd);
    all_bytes.push_back(0xee);
    all_bytes.push_back(0xff);
    all_bytes.push_back(0x00);

    Span_Reader* base_reader = &this->base_readers_.emplace_back(all_bytes);
    U32 block_size = 4;
    std::vector<U32> block_indexes;
    for (U32 i = 0; i * block_size < base_reader->size(); ++i) {
      block_indexes.push_back(i + 1);
    }
    return PDB_Blocks_Reader<Span_Reader>(base_reader, block_indexes,
                                          block_size, data.size(),
                                          /*streamIndex=*/0);
  }
  std::deque<std::vector<U8>> datas_;
  std::deque<Span_Reader> base_readers_;
};

using Reader_Factories = ::testing::Types<
    Span_Reader_Factory, Sub_File_Reader_With_Full_Span_Reader,
    Sub_File_Reader_With_Full_Span_Reader_And_Implicit_Size,
    Sub_File_Reader_With_Full_Span_Reader_And_Too_Big_Size,
    Sub_File_Reader_With_Partial_Span_Reader,
    Sub_File_Reader_Inside_Sub_File_Reader_With_Span_Reader,
    Sub_File_Reader_With_Partial_Span_Reader_And_Implicit_Size,
    PDB_Blocks_Reader_With_Block_Size_4_With_Span_Reader,
    PDB_Blocks_Reader_With_Block_Size_4_With_Subset_Of_Span_Reader>;
TYPED_TEST_SUITE(Test_Reader, Reader_Factories);

TYPED_TEST(Test_Reader, has_correct_size) {
  static const U8 data[] = {10, 20, 30, 40, 50};
  auto r = this->make_reader(data);
  EXPECT_EQ(r.size(), 5);
}

TYPED_TEST(Test_Reader, reads_u8) {
  static const U8 data[] = {10, 20, 30, 40, 50};
  auto r = this->make_reader(data);
  EXPECT_EQ(r.u8(0), 10);
  EXPECT_EQ(r.u8(1), 20);
  EXPECT_EQ(r.u8(2), 30);
  EXPECT_EQ(r.u8(3), 40);
  EXPECT_EQ(r.u8(4), 50);
}

TYPED_TEST(Test_Reader, reads_u16) {
  static const U8 data[] = {1, 2, 3, 4, 5};
  auto r = this->make_reader(data);
  EXPECT_EQ(r.u16(0), 0x0201);
  EXPECT_EQ(r.u16(1), 0x0302);
  EXPECT_EQ(r.u16(2), 0x0403);
}

TYPED_TEST(Test_Reader, reads_u32) {
  static const U8 data[] = {1, 2, 3, 4, 5};
  auto r = this->make_reader(data);
  if (std::is_same_v<decltype(r), PDB_Blocks_Reader<Span_Reader>>) {
    // TODO(strager): Reading u32 straddling multiple blocks is not yet
    // implemented by PDB_Blocks_Reader.
    return;
  }
  EXPECT_EQ(r.u32(0), 0x04030201);
  EXPECT_EQ(r.u32(1), 0x05040302);
}

TYPED_TEST(Test_Reader, reads_byte_array) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60, 70, 80};
  auto r = this->make_reader(data);

  {
    U8 bytes[8] = {};
    std::span<U8> bytes_span(bytes);
    r.copy_bytes_into(bytes_span, 0);
    EXPECT_THAT(bytes, ElementsAreArray({10, 20, 30, 40, 50, 60, 70, 80}));
  }

  {
    U8 bytes[4] = {};
    std::span<U8> bytes_span(bytes);
    r.copy_bytes_into(bytes_span, 2);
    EXPECT_THAT(bytes, ElementsAreArray({30, 40, 50, 60}));
  }

  {
    U8 bytes[4] = {};
    std::span<U8> bytes_span(bytes);
    r.copy_bytes_into(bytes_span.subspan(1, 2), 2);
    EXPECT_THAT(bytes, ElementsAreArray({0, 30, 40, 0}));
  }
}

TYPED_TEST(Test_Reader, reads_fixed_with_string) {
  static const U8 data[] = {u8'h', u8'e', u8'l', u8'l', u8'o', 0, 0, 0, u8'x'};
  auto r = this->make_reader(data);

  EXPECT_EQ(r.fixed_width_string(0, 8), u8"hello");
  EXPECT_EQ(r.fixed_width_string(2, 6), u8"llo");

  // No trailing zeros:
  EXPECT_EQ(r.fixed_width_string(0, 5), u8"hello");
  EXPECT_EQ(r.fixed_width_string(0, 4), u8"hell");
}

TYPED_TEST(Test_Reader, reads_utf_8_c_string) {
  static const U8 data[] = {
      // "hello\0"
      u8'h',
      u8'e',
      u8'l',
      u8'l',
      u8'o',
      0x00,
      // "wörld\0"
      u8'w',
      0xc3,
      0xb6,
      u8'r',
      u8'l',
      u8'd',
      0x00,
  };
  auto r = this->make_reader(data);
  EXPECT_EQ(r.utf_8_c_string(0), u8"hello");
  EXPECT_EQ(r.utf_8_c_string(6), u8"wörld");
}

TYPED_TEST(Test_Reader, finds_u8_if_present) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60, 70};
  auto r = this->make_reader(data);

  EXPECT_EQ(r.find_u8(10, 0), 0);
  EXPECT_EQ(r.find_u8(20, 0), 1);
  EXPECT_EQ(r.find_u8(30, 0), 2);
  EXPECT_EQ(r.find_u8(40, 0), 3);
  EXPECT_EQ(r.find_u8(50, 0), 4);

  EXPECT_EQ(r.find_u8(30, 2), 2);
  EXPECT_EQ(r.find_u8(50, 2), 4);

  EXPECT_EQ(r.find_u8(10, 0, 6), 0);
  EXPECT_EQ(r.find_u8(50, 0, 6), 4);
}

TYPED_TEST(Test_Reader, fails_to_find_u8_if_missing) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60, 70};
  auto r = this->make_reader(data);

  EXPECT_EQ(r.find_u8(0, 0), std::nullopt);
  EXPECT_EQ(r.find_u8(15, 0), std::nullopt);
  EXPECT_EQ(r.find_u8(15, 0, 7), std::nullopt);
  EXPECT_EQ(r.find_u8(15, 0, 100), std::nullopt);

  EXPECT_EQ(r.find_u8(10, 2), std::nullopt);

  EXPECT_EQ(r.find_u8(70, 0, 2), std::nullopt);
  EXPECT_EQ(r.find_u8(50, 0, 4), std::nullopt);
}

TYPED_TEST(Test_Reader, out_of_bounds_u8_fails) {
  static const U8 data[] = {10, 20};
  auto r = this->make_reader(data);
  EXPECT_THROW({ r.u8(2); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u8(100); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u8(std::numeric_limits<U64>::max()); }, Out_Of_Bounds_Read);
}

TYPED_TEST(Test_Reader, out_of_bounds_u16_fails) {
  static const U8 data[] = {10, 20};
  auto r = this->make_reader(data);
  EXPECT_THROW({ r.u16(1); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u16(100); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u16(std::numeric_limits<U64>::max() - 1); },
               Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u16(std::numeric_limits<U64>::max()); }, Out_Of_Bounds_Read);
}

TYPED_TEST(Test_Reader, out_of_bounds_u32_fails) {
  static const U8 data[] = {10, 20, 30, 40};
  auto r = this->make_reader(data);
  if (std::is_same_v<decltype(r), PDB_Blocks_Reader<Span_Reader>>) {
    // TODO(strager): Reading u32 straddling multiple blocks is not yet
    // implemented by PDB_Blocks_Reader.
    return;
  }
  EXPECT_THROW({ r.u32(2); }, Out_Of_Bounds_Read);
}

TYPED_TEST(Test_Reader, out_of_bounds_utf_8_c_string_fails) {
  static const U8 data[] = {0x6c, 0x6f, 0x6c};
  auto r = this->make_reader(data);
  EXPECT_THROW({ r.utf_8_c_string(0); }, C_String_Null_Terminator_Not_Found);
}

TYPED_TEST(Test_Reader, out_of_bounds_utf_8_string_fails) {
  static const U8 data[] = {0x6c, 0x6f, 0x6c};
  auto r = this->make_reader(data);
  EXPECT_THROW({ r.utf_8_string(0, 4); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.utf_8_string(0, 100); }, Out_Of_Bounds_Read);
}

TYPED_TEST(Test_Reader, out_of_bounds_copy_bytes_into_fails) {
  static const U8 data[] = {0x6c, 0x6f, 0x6c};
  auto r = this->make_reader(data);
  EXPECT_THROW(
      {
        U8 buffer[4];
        r.copy_bytes_into(buffer, 0);
      },
      Out_Of_Bounds_Read);
  EXPECT_THROW(
      {
        U8 buffer[100];
        r.copy_bytes_into(buffer, 0);
      },
      Out_Of_Bounds_Read);
  EXPECT_THROW(
      {
        U8 buffer[1];
        r.copy_bytes_into(buffer, 4);
      },
      Out_Of_Bounds_Read);
}

TEST(Test_Sub_File_Reader, combine_nested_readers) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60};
  Span_Reader base_reader(data);
  Sub_File_Reader inner_reader(&base_reader, 1, 5);
  Sub_File_Reader outer_reader = inner_reader.sub_reader(1);
  EXPECT_EQ(outer_reader.base_reader(), &base_reader);
  EXPECT_EQ(outer_reader.sub_file_offset(), 1 + 1);
  EXPECT_EQ(outer_reader.size(), 4);
}

TEST(Test_Sub_File_Reader, combine_nested_readers_with_outer_size) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60};
  Span_Reader base_reader(data);
  Sub_File_Reader inner_reader(&base_reader, 1, 5);
  Sub_File_Reader outer_reader = inner_reader.sub_reader(1, 3);
  EXPECT_EQ(outer_reader.base_reader(), &base_reader);
  EXPECT_EQ(outer_reader.sub_file_offset(), 1 + 1);
  EXPECT_EQ(outer_reader.size(), 3);
}
}
}
