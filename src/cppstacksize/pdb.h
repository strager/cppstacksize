#pragma once

#include <cppstacksize/guid.h>
#include <cppstacksize/logger.h>
#include <cppstacksize/pdb-reader.h>
#include <cppstacksize/reader.h>
#include <cppstacksize/util.h>
#include <stdexcept>
#include <vector>

namespace cppstacksize {
class PDB_Magic_Mismatch : public std::exception {
 public:
  const char* what() const noexcept { return "PDB magic mismatched"; }
};

/// The header of a PDB file.
struct PDB_Super_Block {
  U32 block_size;
  U32 block_count;
  U32 directory_size;       // In bytes.
  U32 directory_map_block;  // Block index.
};

constexpr inline U32 pdb_magic[] = {
    0x7263694d, 0x666f736f, 0x2f432074, 0x202b2b43,
    0x2046534d, 0x30302e37, 0x441a0a0d, 0x00000053,
};

/// Parses the superblock.
template <class Reader>
PDB_Super_Block parse_pdb_header(Reader& reader, Logger& = fallback_logger) {
  PDB_Super_Block super_block;
  try {
    for (U64 i = 0; i < std::size(pdb_magic); ++i) {
      if (reader.u32(i * 4) != pdb_magic[i]) {
        throw PDB_Magic_Mismatch();
      }
    }
  } catch (Out_Of_Bounds_Read&) {
    throw PDB_Magic_Mismatch();
  }
  super_block.block_size = reader.u32(0x20);
  super_block.block_count = reader.u32(0x28);
  super_block.directory_size = reader.u32(0x2c);
  super_block.directory_map_block = reader.u32(0x34);
  return super_block;
}

/// A parsed PDB info stream (stream #1).
struct PDB_Info {
  GUID guid;

  std::string get_guid_string() { return this->guid.to_string(); }
};

/// Parse a PDB's stream #1.
template <class Reader>
PDB_Info parse_pdb_info_stream(const Reader& reader,
                               Logger& = fallback_logger) {
  U8 guid_bytes[16];
  reader.copy_bytes_into(guid_bytes, 12);
  return PDB_Info{.guid = GUID(guid_bytes)};
}

template <class Reader>
std::vector<PDB_Blocks_Reader<Reader>> parse_pdb_stream_directory(
    const Reader* reader, const PDB_Super_Block& super_block,
    Logger& = fallback_logger) {
  U32 directory_block_count =
      (U64{super_block.directory_size} + super_block.block_size - 1) /
      super_block.block_size;
  std::vector<U32> directory_blocks;
  for (U32 i = 0; i < directory_block_count; ++i) {
    directory_blocks.push_back(reader->u32(
        super_block.directory_map_block * super_block.block_size + i * 4));
  }
  PDB_Blocks_Reader<Reader> directory_reader(reader, directory_blocks,
                                             super_block.block_size,
                                             super_block.directory_size,
                                             /*streamIndex=*/-1);

  std::vector<PDB_Blocks_Reader<Reader>> streams;
  U64 offset = 0;
  U32 stream_count = directory_reader.u32(offset);
  offset += 4;
  std::vector<U32> stream_sizes;
  // FIXME(strager): These loops are infinite loops if stream_count==U32::max.
  for (U32 stream_index = 0; stream_index < stream_count; ++stream_index) {
    stream_sizes.push_back(directory_reader.u32(offset));
    offset += 4;
  }
  for (U32 stream_index = 0; stream_index < stream_count; ++stream_index) {
    U32 stream_size = stream_sizes[stream_index];
    U32 block_count = ((U64{stream_size} + super_block.block_size) - 1) /
                      super_block.block_size;
    if (stream_size == 0xffffffffU) {
      // HACK(strager): Sometimes we see a stream with size 4294967295. This
      // might be legit, but I suspect not. Pretend the size is 0 instead.
      block_count = 0;
    }

    std::vector<U32> stream_blocks;
    for (U32 i = 0; i < block_count; ++i) {
      stream_blocks.push_back(directory_reader.u32(offset));
      offset += 4;
    }
    streams.push_back(PDB_Blocks_Reader<Reader>(reader, stream_blocks,
                                                super_block.block_size,
                                                stream_size, stream_index));
  }
  return streams;
}

struct PDB_DBI_Module_Segment {
  std::optional<U16> pe_section_index;
  U64 offset;
  U64 size;
};

struct PDB_DBI_Module {
  std::u8string linked_object_path;
  std::u8string source_object_path;
  U16 debug_info_stream_index;
  U32 symbols_size;
  U32 c11_line_info_size;
  U32 c13_line_info_size;
  std::vector<PDB_DBI_Module_Segment> segments;

  U32 c13_line_info_offset() const {
    return this->symbols_size + this->c11_line_info_size;
  }
};

struct PDB_DBI {
  std::vector<PDB_DBI_Module> modules;
};

template <class Reader>
PDB_DBI parse_pdb_dbi_stream(const Reader& reader,
                             Logger& logger = fallback_logger) {
  PDB_DBI dbi;
  if (reader.size() == 0) {
    return dbi;
  }

  U32 module_info_size = reader.u32(0x18);
  U64 module_infos_begin = 0x40;
  Sub_File_Reader<Reader> module_infos_reader(&reader, module_infos_begin,
                                              module_info_size);
  U64 offset = 0;
  while (offset < module_infos_reader.size()) {
    offset = align_up(offset, 4);
    U16 module_section_section = module_infos_reader.u16(offset + 0x04);
    U16 module_section_offset = module_infos_reader.u16(offset + 0x08);
    U16 module_section_size = module_infos_reader.u16(offset + 0x0c);
    U16 module_sym_stream = module_infos_reader.u16(offset + 0x22);
    U32 symbols_size = module_infos_reader.u32(offset + 0x24);
    U32 c11_line_info_size = module_infos_reader.u32(offset + 0x28);
    U32 c13_line_info_size = module_infos_reader.u32(offset + 0x2c);
    std::optional<U64> module_name_null_terminator_offset =
        module_infos_reader.find_u8(0, offset + 0x40);
    if (!module_name_null_terminator_offset.has_value()) {
      logger.log("incomplete module info entry",
                 module_infos_reader.locate(offset));
      break;
    }
    std::u8string module_name = module_infos_reader.utf_8_string(
        offset + 0x40, *module_name_null_terminator_offset - (offset + 0x40));
    offset = *module_name_null_terminator_offset + 1;
    std::optional<U64> obj_name_null_terminator_offset =
        module_infos_reader.find_u8(0, offset);
    if (!obj_name_null_terminator_offset.has_value()) {
      logger.log("incomplete module info entry",
                 module_infos_reader.locate(offset));
      break;
    }
    std::u8string obj_name = module_infos_reader.utf_8_string(
        offset, *obj_name_null_terminator_offset - offset);
    offset = *obj_name_null_terminator_offset + 1;

    dbi.modules.push_back(PDB_DBI_Module{
        .linked_object_path = std::move(obj_name),
        .source_object_path = std::move(module_name),
        .debug_info_stream_index = module_sym_stream,
        .symbols_size = symbols_size,
        .c11_line_info_size = c11_line_info_size,
        .c13_line_info_size = c13_line_info_size,
        .segments =
            {
                PDB_DBI_Module_Segment{
                    .pe_section_index =
                        module_section_section == 0 ||
                                module_section_section == 0xffff
                            ? std::nullopt
                            : std::optional<U16>(module_section_section - 1),
                    .offset = module_section_offset,
                    .size = module_section_size,
                },
            },
    });
  }
  return dbi;
}

template <class Reader>
struct PDB_TPI {
  Sub_File_Reader<Reader> type_reader;
};

template <class Reader>
PDB_TPI<Reader> parse_pdb_tpi_stream_header(const Reader* reader,
                                            Logger& = fallback_logger) {
  U32 header_size = reader->u32(0x4);
  U32 type_records_size = reader->u32(0x10);
  return PDB_TPI<Reader>{
      .type_reader = Sub_File_Reader(reader, header_size, type_records_size),
  };
}
}
