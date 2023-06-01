#include <cppstacksize/base.h>
#include <cppstacksize/reader.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>

using ::testing::ElementsAreArray;

namespace cppstacksize {
namespace {
TEST(Test_Reader, has_correct_size) {
  static const U8 data[] = {10, 20, 30, 40, 50};
  Span_Reader r(data);
  EXPECT_EQ(r.size(), 5);
}

TEST(Test_Reader, reads_u8) {
  static const U8 data[] = {10, 20, 30, 40, 50};
  Span_Reader r(data);
  EXPECT_EQ(r.u8(0), 10);
  EXPECT_EQ(r.u8(1), 20);
  EXPECT_EQ(r.u8(2), 30);
  EXPECT_EQ(r.u8(3), 40);
  EXPECT_EQ(r.u8(4), 50);
}

TEST(Test_Reader, reads_u16) {
  static const U8 data[] = {1, 2, 3, 4, 5};
  Span_Reader r(data);
  EXPECT_EQ(r.u16(0), 0x0201);
  EXPECT_EQ(r.u16(1), 0x0302);
  EXPECT_EQ(r.u16(2), 0x0403);
}

TEST(Test_Reader, reads_u32) {
  static const U8 data[] = {1, 2, 3, 4, 5};
  Span_Reader r(data);
#if 0  // TODO(strager)
  if (r instanceof PDB_Blocks_Reader) {
    // TODO(strager): Reading u32 straddling multiple blocks is not yet
    // implemented by PDB_Blocks_Reader.
    return;
  }
#endif
  EXPECT_EQ(r.u32(0), 0x04030201);
  EXPECT_EQ(r.u32(1), 0x05040302);
}

TEST(Test_Reader, reads_byte_array) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60, 70, 80};
  Span_Reader r(data);

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

TEST(Test_Reader, reads_fixed_with_string) {
  static const U8 data[] = {u8'h', u8'e', u8'l', u8'l', u8'o', 0, 0, 0, u8'x'};
  Span_Reader r(data);

  EXPECT_EQ(r.fixed_width_string(0, 8), u8"hello");
  EXPECT_EQ(r.fixed_width_string(2, 6), u8"llo");

  // No trailing zeros:
  EXPECT_EQ(r.fixed_width_string(0, 5), u8"hello");
  EXPECT_EQ(r.fixed_width_string(0, 4), u8"hell");
}

TEST(Test_Reader, reads_utf_8_c_string) {
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
  Span_Reader r(data);
  EXPECT_EQ(r.utf_8_c_string(0), u8"hello");
  EXPECT_EQ(r.utf_8_c_string(6), u8"wörld");
}

TEST(Test_Reader, finds_u8_if_present) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60, 70};
  Span_Reader r(data);

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

TEST(Test_Reader, fails_to_find_u8_if_missing) {
  static const U8 data[] = {10, 20, 30, 40, 50, 60, 70};
  Span_Reader r(data);

  EXPECT_EQ(r.find_u8(0, 0), std::nullopt);
  EXPECT_EQ(r.find_u8(15, 0), std::nullopt);
  EXPECT_EQ(r.find_u8(15, 0, 7), std::nullopt);
  EXPECT_EQ(r.find_u8(15, 0, 100), std::nullopt);

  EXPECT_EQ(r.find_u8(10, 2), std::nullopt);

  EXPECT_EQ(r.find_u8(70, 0, 2), std::nullopt);
  EXPECT_EQ(r.find_u8(50, 0, 4), std::nullopt);
}

TEST(Test_Reader, out_of_bounds_u8_fails) {
  static const U8 data[] = {10, 20};
  Span_Reader r(data);
  EXPECT_THROW({ r.u8(2); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u8(100); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u8(std::numeric_limits<U64>::max()); }, Out_Of_Bounds_Read);
}

TEST(Test_Reader, out_of_bounds_u16_fails) {
  static const U8 data[] = {10, 20};
  Span_Reader r(data);
  EXPECT_THROW({ r.u16(1); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u16(100); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u16(std::numeric_limits<U64>::max() - 1); },
               Out_Of_Bounds_Read);
  EXPECT_THROW({ r.u16(std::numeric_limits<U64>::max()); }, Out_Of_Bounds_Read);
}

TEST(Test_Reader, out_of_bounds_u32_fails) {
  static const U8 data[] = {10, 20, 30, 40};
  Span_Reader r(data);
#if 0  // TODO(strager)
  if (r instanceof PDB_Blocks_Reader) {
    // TODO(strager): Reading u32 straddling multiple blocks is not yet
    // implemented by PDB_Blocks_Reader.
    return;
  }
#endif
  EXPECT_THROW({ r.u32(2); }, Out_Of_Bounds_Read);
}

TEST(Test_Reader, out_of_bounds_utf_8_c_string_fails) {
  static const U8 data[] = {0x6c, 0x6f, 0x6c};
  Span_Reader r(data);
  EXPECT_THROW({ r.utf_8_c_string(0); }, C_String_Null_Terminator_Not_Found);
}

TEST(Test_Reader, out_of_bounds_utf_8_string_fails) {
  static const U8 data[] = {0x6c, 0x6f, 0x6c};
  Span_Reader r(data);
  EXPECT_THROW({ r.utf_8_string(0, 4); }, Out_Of_Bounds_Read);
  EXPECT_THROW({ r.utf_8_string(0, 100); }, Out_Of_Bounds_Read);
}
}

TEST(Test_Reader, out_of_bounds_copy_bytes_into_fails) {
  static const U8 data[] = {0x6c, 0x6f, 0x6c};
  Span_Reader r(data);
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
}
