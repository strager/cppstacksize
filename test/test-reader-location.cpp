#include <cppstacksize/pdb.h>
#include <cppstacksize/reader.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_Reader_Location, span_reader) {
  static const U8 data[] = {1, 2, 3, 4};
  Span_Reader reader(data);

  {
    Location location = reader.locate(0);
    EXPECT_EQ(location.file_offset, 0);
    EXPECT_EQ(location.stream_index, std::nullopt);
    EXPECT_EQ(location.stream_offset, std::nullopt);
  }

  {
    Location location = reader.locate(2);
    EXPECT_EQ(location.file_offset, 2);
    EXPECT_EQ(location.stream_index, std::nullopt);
    EXPECT_EQ(location.stream_offset, std::nullopt);
  }
}

TEST(Test_Reader_Location, pdb_blocks_reader) {
  static const U8 data[] = {1, 2, 3, 4};
  Span_Reader base_reader(data);
  PDB_Blocks_Reader<Span_Reader> reader(
      /*base_reader=*/&base_reader,
      /*block_indexes=*/{1, 0},
      /*block_size=*/2,
      /*byte_size=*/4,
      /*stream_index=*/42);

  {
    Location location = reader.locate(0);
    EXPECT_EQ(location.file_offset, 2);
    EXPECT_EQ(location.stream_index, 42);
    EXPECT_EQ(location.stream_offset, 0);
  }

  {
    Location location = reader.locate(1);
    EXPECT_EQ(location.file_offset, 3);
    EXPECT_EQ(location.stream_index, 42);
    EXPECT_EQ(location.stream_offset, 1);
  }

  {
    Location location = reader.locate(2);
    EXPECT_EQ(location.file_offset, 0);
    EXPECT_EQ(location.stream_index, 42);
    EXPECT_EQ(location.stream_offset, 2);
  }

  {
    Location location = reader.locate(3);
    EXPECT_EQ(location.file_offset, 1);
    EXPECT_EQ(location.stream_index, 42);
    EXPECT_EQ(location.stream_offset, 3);
  }
}

TEST(Test_Reader_Location, sub_file_reader_with_offset) {
  static const U8 data[] = {1, 2, 3, 4};
  Span_Reader base_reader(data);
  Sub_File_Reader<Span_Reader> reader(&base_reader,
                                      /*offset=*/1,
                                      /*size=*/2);

  {
    Location location = reader.locate(0);
    EXPECT_EQ(location.file_offset, 1);
    EXPECT_EQ(location.stream_index, std::nullopt);
    EXPECT_EQ(location.stream_offset, std::nullopt);
  }

  {
    Location location = reader.locate(2);
    EXPECT_EQ(location.file_offset, 3);
    EXPECT_EQ(location.stream_index, std::nullopt);
    EXPECT_EQ(location.stream_offset, std::nullopt);
  }
}

TEST(Test_Reader_Location,
     sub_file_reader_with_offset_of_sub_file_reader_with_offset) {
  static const U8 data[] = {1, 2, 3, 4, 5, 6, 7};
  Span_Reader base_reader(data);
  Sub_File_Reader<Span_Reader> inner_reader(&base_reader,
                                            /*offset=*/1,
                                            /*size=*/5);
  Sub_File_Reader<Sub_File_Reader<Span_Reader>> reader(&inner_reader,
                                                       /*offset=*/2,
                                                       /*size=*/2);

  {
    Location location = reader.locate(0);
    EXPECT_EQ(location.file_offset, 0 + 2 + 1);
    EXPECT_EQ(location.stream_index, std::nullopt);
    EXPECT_EQ(location.stream_offset, std::nullopt);
  }

  {
    Location location = reader.locate(2);
    EXPECT_EQ(location.file_offset, 2 + 2 + 1);
    EXPECT_EQ(location.stream_index, std::nullopt);
    EXPECT_EQ(location.stream_offset, std::nullopt);
  }
}

TEST(Test_Reader_Location, serializes_with_only_file_offset) {
  EXPECT_EQ((Location{42, std::nullopt, std::nullopt}).to_string(),
            "file offset 0x2a");
}

TEST(Test_Reader_Location, serializes_with_stream_offset) {
  EXPECT_EQ((Location{42, 420, 69}.to_string()),
            "stream #420 offset 0x45 (file offset 0x2a)");
}

TEST(Test_Reader_Location, serializes_with_stream_directory_offset) {
  EXPECT_EQ((Location{42, static_cast<U32>(-1), 69}.to_string()),
            "stream directory offset 0x45 (file offset 0x2a)");
}
}
}
