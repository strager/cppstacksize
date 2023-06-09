#pragma once

#include <cppstacksize/base.h>
#include <cppstacksize/codeview-constants.h>
#include <cppstacksize/logger.h>
#include <cppstacksize/pdb-reader.h>
#include <cppstacksize/pdb.h>
#include <cppstacksize/reader.h>
#include <cppstacksize/util.h>
#include <iosfwd>
#include <variant>
#include <vector>

namespace cppstacksize {
struct Line_Source_Info {
  static inline constexpr U32 out_of_bounds_line_number = static_cast<U32>(-1);

  U32 line_number;

  static Line_Source_Info out_of_bounds() {
    return Line_Source_Info{
        .line_number = out_of_bounds_line_number,
    };
  }

  bool is_out_of_bounds() const {
    return this->line_number == out_of_bounds_line_number;
  }

  friend bool operator==(const Line_Source_Info&,
                         const Line_Source_Info&) = default;
  friend std::ostream& operator<<(std::ostream&, const Line_Source_Info&);
};

class Line_Tables {
 public:
  struct Handle {
    static constexpr U64 null_module_index = static_cast<U64>(-1);

    U64 module_index;

    static Handle null() { return Handle{.module_index = null_module_index}; }

    bool is_null() const { return this->module_index == null_module_index; }
  };

  void clear() { this->modules_.clear(); }

  // pdb_streams[module.debug_info_stream_index] must remain valid.
  template <class Reader>
  Handle add_module_line_tables(
      const PDB_DBI_Module& module,
      const std::vector<PDB_Blocks_Reader<Reader>>& pdb_streams) {
    return this->add_module_line_tables(
        module, std::span<const PDB_Blocks_Reader<Reader>>(pdb_streams));
  }

  // pdb_streams[module.debug_info_stream_index] must remain valid.
  template <class Reader>
  Handle add_module_line_tables(
      const PDB_DBI_Module& module,
      std::span<const PDB_Blocks_Reader<Reader>> pdb_streams) {
    CSS_ASSERT(module.debug_info_stream_index < pdb_streams.size());
    Sub_File_Reader line_tables_reader(
        &pdb_streams[module.debug_info_stream_index], module.symbols_size,
        module.c13_line_info_size);
    return this->add_module_line_tables(line_tables_reader);
  }

  // Copies codeview_reader. Data referenced by codeview_reader must remain
  // valid.
  template <class Reader>
  Handle add_module_line_tables(Reader codeview_reader) {
    U64 module_index = this->modules_.size();
    Module& module = this->modules_.emplace_back(codeview_reader);

    U64 offset = 0;
    for (;;) {
      offset = align_up(offset, 4);
      if (offset >= codeview_reader.size()) {
        break;
      }
      U64 subsection_offset = offset;
      U32 subsection_type = codeview_reader.u32(offset);
      offset += 4;
      U32 subsection_size = codeview_reader.u32(offset);
      offset += 4;
      switch (subsection_type) {
        case DEBUG_S_LINES:
          module.subsection_offsets.push_back(subsection_offset);
          break;
        default:
          // Ignore.
          break;
      }
      offset += subsection_size;
    }
    return Handle{.module_index = module_index};
  }

  Line_Source_Info source_info_for_offset(Handle handle, U32 code_section_index,
                                          U32 instruction_offset,
                                          Logger& logger = fallback_logger);

 private:
  struct Module {
    template <class Reader>
    explicit Module(Reader r) : reader(std::move(r)) {}

    std::variant<Sub_File_Reader<PDB_Blocks_Reader<Span_Reader>>> reader;
    std::vector<U64> subsection_offsets;
  };

  // Searches for a match in a DEBUG_S_LINES subsection.
  template <class Reader>
  Line_Source_Info source_info_for_offset_in_subsection(const Reader& reader,
                                                        U32 code_section_index,
                                                        U32 instruction_offset,
                                                        Logger& logger);

  std::vector<Module> modules_;
};

template <class Reader>
inline Line_Source_Info Line_Tables::source_info_for_offset_in_subsection(
    const Reader& reader, U32 code_section_index, U32 instruction_offset,
    Logger& logger) {
  U32 instructions_start_offset = reader.u32(0);
  U32 current_code_section_index = U32{reader.u16(4)} - 1;
  U16 flags = reader.u16(6);
  U32 instructions_size = reader.u32(8);
  // TODO(strager): Check for overflow.
  U32 instructions_end_offset = instructions_start_offset + instructions_size;
  if (current_code_section_index != code_section_index) {
    return Line_Source_Info::out_of_bounds();
  }
  if (!(instructions_start_offset <= instruction_offset &&
        instruction_offset < instructions_end_offset)) {
    return Line_Source_Info::out_of_bounds();
  }
  bool have_column_info = flags & CV_LINES_HAVE_COLUMNS;
  if (have_column_info) {
    logger.log(
        "line table has column info, but column info parsing is not "
        "yet implemented",
        reader.locate(0));
  }

  U32 instruction_relative_offset =
      instruction_offset - instructions_start_offset;

  struct Raw_Entry_Data {
    U32 file_id;
    U32 line_number_and_flags;
  };
  std::optional<Raw_Entry_Data> last_entry = std::nullopt;

  U64 offset = 12;
  while (offset < reader.size()) {
    // Read the block header:
    U32 file_id = reader.u32(offset + 0);
    U32 line_count = reader.u32(offset + 4);
    U32 block_byte_size = reader.u32(offset + 8);
    U32 block_end_offset = offset + block_byte_size;

    offset += 12;
    // Read the line numbers in this block:
    for (U32 i = 0; i < line_count; ++i) {
      constexpr U64 entry_size = 8;
      U32 line_entry_relative_offset = reader.u32(offset + 0);
      U32 line_number_and_flags = reader.u32(offset + 4);
      if (line_entry_relative_offset > instruction_relative_offset) {
        goto found_match;
      }
      last_entry = Raw_Entry_Data{
          .file_id = file_id,
          .line_number_and_flags = line_number_and_flags,
      };
      offset += entry_size;
    }

    offset = block_end_offset;
  }

  if (!last_entry.has_value()) {
    logger.log("line table looks corrupt; there were no entries",
               reader.locate(0));
    return Line_Source_Info::out_of_bounds();
  }

found_match:
  if (!last_entry.has_value()) {
    // There is no previous entry.
    logger.log(
        "line table looks corrupt; the first entry's relative offset should "
        "have been 0",
        reader.locate(0));
    return Line_Source_Info::out_of_bounds();
  }
  U32 line_number = last_entry->line_number_and_flags & 0x00ffffff;
  return Line_Source_Info{.line_number = line_number};
}

inline Line_Source_Info Line_Tables::source_info_for_offset(
    Handle handle, U32 code_section_index, U32 instruction_offset,
    Logger& logger) {
  CSS_ASSERT(!handle.is_null());
  Module& module = this->modules_.at(handle.module_index);
  return std::visit(
      [&](auto& reader) -> Line_Source_Info {
        for (U64 subsection_offset : module.subsection_offsets) {
          CSS_ASSERT(reader.u32(subsection_offset + 0) == DEBUG_S_LINES);
          U32 subsection_size = reader.u32(subsection_offset + 4);
          Sub_File_Reader subsection_reader =
              reader.sub_reader(subsection_offset + 8, subsection_size);
          Line_Source_Info info = this->source_info_for_offset_in_subsection(
              subsection_reader, code_section_index, instruction_offset,
              logger);
          if (!info.is_out_of_bounds()) {
            return info;
          }
        }
        return Line_Source_Info::out_of_bounds();
      },
      module.reader);
}

}
