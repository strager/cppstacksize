#include <algorithm>
#include <cppstacksize/codeview.h>
#include <cppstacksize/example-file.h>
#include <cppstacksize/pdb.h>
#include <cppstacksize/pe.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
constexpr U8 pdb_file_magic[] = {
    'M', 'i', 'c',  'r',  'o',  's',  'o',  'f',  't',  ' ',  'C',
    '/', 'C', '+',  '+',  ' ',  'M',  'S',  'F',  ' ',  '7',  '.',
    '0', '0', '\r', '\n', 0x1a, 0x44, 0x53, 0x00, 0x00, 0x00,
};

class Byte_Writer {
 public:
  explicit Byte_Writer(std::span<U8> data) : data_(data) {}

  void set_u32(U64 offset, U32 value) {
    this->data_[offset + 0] = static_cast<U8>(value >> (0 * 8));
    this->data_[offset + 1] = static_cast<U8>(value >> (1 * 8));
    this->data_[offset + 2] = static_cast<U8>(value >> (2 * 8));
    this->data_[offset + 3] = static_cast<U8>(value >> (3 * 8));
  }

 private:
  std::span<U8> data_;
};

// Helper class to make asserting on PDB_Blocks_Reader easier.
struct PDB_Stream_Blocks_And_Size {
  std::vector<U32> blocks;
  U32 size;

  friend bool operator==(const PDB_Blocks_Reader<Span_Reader>& lhs,
                         const PDB_Stream_Blocks_And_Size& rhs) {
    auto lhs_blocks = lhs.blocks();
    return std::equal(lhs_blocks.begin(), lhs_blocks.end(), rhs.blocks.begin(),
                      rhs.blocks.end()) &&
           lhs.size() == rhs.size;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const PDB_Stream_Blocks_And_Size& v) {
    out << "blocks={";
    for (U32 block : v.blocks) {
      out << block << ",";
    }
    out << "} size=" << v.size;
    return out;
  }
};

TEST(Test_PDB, pdb_reads_stream_directory_block_indexes) {
  // In this test, we write code to manually construct a PDB file. It has a
  // stream directory which spans only one block and contains only one stream,
  // simulating a small (but incomplete) PDB file.

  U32 block_size = 512;
  U32 block_count = 1000;
  U32 free_block_map_block_index = 1;

  std::vector<U8> file_data(block_size * block_count);
  Byte_Writer file_data_writer(file_data);

  // Superblock (block #0):
  std::copy(std::begin(pdb_file_magic), std::end(pdb_file_magic),
            &file_data[0]);
  file_data_writer.set_u32(0x20, block_size);
  file_data_writer.set_u32(0x24, free_block_map_block_index);
  file_data_writer.set_u32(0x28, block_count);
  U32 directory_size = 4 + 1 * 4 + 1 * 4;
  file_data_writer.set_u32(0x2c, directory_size);
  U32 directory_map_block_index = 69;
  file_data_writer.set_u32(0x34, directory_map_block_index);

  // Directory map (block #69):
  U32 directory_block_index = 44;
  U32 directory_map_offset = directory_map_block_index * block_size;
  file_data_writer.set_u32(directory_map_offset, directory_block_index);

  // Directory (blocks #44):
  {
    U32 offset = directory_block_index * block_size;

    U32 stream_count = 1;
    file_data_writer.set_u32(offset, stream_count);
    offset += 4;

    // Stream sizes:
    file_data_writer.set_u32(offset, 1 * block_size);
    offset += 4;

    // Stream #0 blocks:
    file_data_writer.set_u32(offset, 200);
    offset += 4;
  }

  Span_Reader file(file_data);
  PDB_Super_Block super_block = parse_pdb_header(file);
  std::vector<PDB_Blocks_Reader<Span_Reader>> streams =
      parse_pdb_stream_directory(&file, super_block);
  EXPECT_THAT(streams, ::testing::ElementsAreArray({
                           // Stream #0:
                           PDB_Stream_Blocks_And_Size{{200}, block_size},
                       }));
}

TEST(
    Test_PDB,
    pdb_reads_stream_directory_block_indexes_with_multi_block_stream_directory) {
  // In this test, we write code to manually construct a PDB file. It has a
  // stream directory which spans two blocks. (This is hard to replicate with
  // a small sample program.)

  U32 block_size = 512;
  U32 block_count = 1000;
  U32 free_block_map_block_index = 1;
  PDB_Stream_Blocks_And_Size streams[] = {
      // Stream #0 (0.5 blocks):
      {.blocks = {200}, .size = U32(0.5 * block_size)},
      // Stream #1 (1 block):
      {.blocks = {300}, .size = 1 * block_size},
      // Stream #2 (3.5 blocks):
      {.blocks = {400, 401, 402, 403}, .size = U32(3.5 * block_size)},
      // Stream #3 (200 blocks):
      {.blocks = {}, .size = 0},
  };
  for (U32 i = 0; i < 200; ++i) {
    streams[3].blocks.push_back(500 + i);
  }
  streams[3].size = streams[3].blocks.size() * block_size;

  std::vector<U8> file_data(block_size * block_count);
  Byte_Writer file_data_writer(file_data);

  // Superblock (block #0):
  std::copy(std::begin(pdb_file_magic), std::end(pdb_file_magic),
            &file_data[0]);
  file_data_writer.set_u32(0x20, block_size);
  file_data_writer.set_u32(0x24, free_block_map_block_index);
  file_data_writer.set_u32(0x28, block_count);
  U32 directory_size = 4 + std::size(streams) * 4;
  for (const PDB_Stream_Blocks_And_Size& stream : streams) {
    directory_size += stream.blocks.size() * 4;
  }
  file_data_writer.set_u32(0x2c, directory_size);
  U32 directory_map_block_index = 42;
  file_data_writer.set_u32(0x34, directory_map_block_index);

  // Directory map (block #42):
  U32 directory_block_indexes[] = {44, 9};
  U32 directory_map_offset = directory_map_block_index * block_size;
  for (U64 i = 0; i < std::size(directory_block_indexes); ++i) {
    file_data_writer.set_u32(directory_map_offset + i * 4,
                             directory_block_indexes[i]);
  }

  // Directory (blocks #44 and #9):
  {
    U32 directory_block_0_begin = directory_block_indexes[0] * block_size;
    U32 directory_block_0_end = directory_block_0_begin + block_size;
    U32 directory_block_1_begin = directory_block_indexes[1] * block_size;
    U32 directory_block_1_end = directory_block_1_begin + block_size;
    U64 offset = directory_block_0_begin;

    U32 stream_count = std::size(streams);
    file_data_writer.set_u32(offset, stream_count);
    offset += 4;

    // Stream sizes:
    file_data_writer.set_u32(offset, streams[0].size);
    offset += 4;
    file_data_writer.set_u32(offset, streams[1].size);
    offset += 4;
    file_data_writer.set_u32(offset, streams[2].size);
    offset += 4;
    file_data_writer.set_u32(offset, streams[3].size);
    offset += 4;

    // Stream #0 blocks:
    file_data_writer.set_u32(offset, streams[0].blocks[0]);
    offset += 4;
    // Stream #1 blocks:
    file_data_writer.set_u32(offset, streams[1].blocks[0]);
    offset += 4;
    // Stream #2 blocks:
    file_data_writer.set_u32(offset, streams[2].blocks[0]);
    offset += 4;
    file_data_writer.set_u32(offset, streams[2].blocks[1]);
    offset += 4;
    file_data_writer.set_u32(offset, streams[2].blocks[2]);
    offset += 4;
    file_data_writer.set_u32(offset, streams[2].blocks[3]);
    offset += 4;

    // Stream #3 blocks (big stream) (first part):
    U64 i = 0;
    for (; i < streams[3].blocks.size() && offset < directory_block_0_end;
         ++i) {
      file_data_writer.set_u32(offset, streams[3].blocks[i]);
      offset += 4;
    }
    EXPECT_EQ(offset, directory_block_0_end);
    // Stream #3 blocks (big stream) (second part):
    offset = directory_block_1_begin;
    for (; i < streams[3].blocks.size(); ++i) {
      file_data_writer.set_u32(offset, streams[3].blocks[i]);
      offset += 4;
    }
    EXPECT_EQ(i, streams[3].blocks.size());
    EXPECT_LT(offset, directory_block_1_end);
  }

  Span_Reader file(file_data);
  PDB_Super_Block super_block = parse_pdb_header(file);
  std::vector<PDB_Blocks_Reader<Span_Reader>> parsed_streams =
      parse_pdb_stream_directory(&file, super_block);
  EXPECT_THAT(parsed_streams, ::testing::ElementsAreArray(streams));
}

TEST(Test_PDB, can_read_stream_directory_block_indexes_from_real_pdb_file) {
  Example_File file("pdb/example.pdb");
  PDB_Super_Block super_block = parse_pdb_header(file.reader());
  std::vector<PDB_Blocks_Reader<Span_Reader>> streams =
      parse_pdb_stream_directory(&file.reader(), super_block);
  EXPECT_THAT(
      streams,
      ::testing::ElementsAreArray({
          PDB_Stream_Blocks_And_Size
          //
          {.blocks = {7}, .size = 76},
          {.blocks = {84}, .size = 174},
          {.blocks = {83, 8, 74, 75, 76, 77, 78, 79, 80, 81, 82},
           .size = 41736},
          {.blocks = {65, 66, 67, 68, 69, 70, 71, 72, 73}, .size = 32795},
          {.blocks = {92, 90, 91}, .size = 8456},
          {.blocks = {86, 85, 87, 88, 89}, .size = 18718},
          {.blocks = {93}, .size = 3188},
          {.blocks = {94}, .size = 960},
          {.blocks = {}, .size = 0},
          {.blocks = {}, .size = 0},
          {.blocks = {24, 25}, .size = 4452},
          {.blocks = {26, 27}, .size = 6048},
          {.blocks = {28, 29}, .size = 5248},
          {.blocks = {30, 31, 32}, .size = 10740},
          {.blocks = {23}, .size = 200},
          {.blocks = {10}, .size = 648},
          {.blocks = {12}, .size = 728},
          {.blocks = {9}, .size = 308},
          {.blocks = {16}, .size = 740},
          {.blocks = {17}, .size = 560},
          {.blocks = {18, 19}, .size = 4284},
          {.blocks = {20, 21}, .size = 4764},
          {.blocks = {22}, .size = 636},
          {.blocks = {33}, .size = 212},
          {.blocks = {34, 35}, .size = 6080},
          {.blocks = {36}, .size = 488},
          {.blocks = {37, 38}, .size = 7528},
          {.blocks = {39}, .size = 648},
          {.blocks = {40}, .size = 192},
          {.blocks = {41, 42}, .size = 4372},
          {.blocks = {43, 44}, .size = 6180},
          {.blocks = {45, 46}, .size = 5920},
          {.blocks = {47, 48}, .size = 6340},
          {.blocks = {49}, .size = 1500},
          {.blocks = {50}, .size = 1620},
          {.blocks = {51}, .size = 1532},
          {.blocks = {52, 53}, .size = 6352},
          {.blocks = {54, 55, 56}, .size = 10728},
          {.blocks = {57}, .size = 1088},
          {.blocks = {58}, .size = 152},
          {.blocks = {59}, .size = 3528},
          {.blocks = {61}, .size = 2860},
          {.blocks = {60, 62, 63, 64}, .size = 13812},
          {.blocks = {}, .size = 0},
          {.blocks = {}, .size = 4294967295},
          {.blocks = {}, .size = 0},
      }));
}
}
}
