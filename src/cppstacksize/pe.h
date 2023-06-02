#ifndef CPPSTACKSIZE_PE_H
#define CPPSTACKSIZE_PE_H

#include <cppstacksize/base.h>
#include <cppstacksize/reader.h>
#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Documentation:
// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format

enum {
  IMAGE_DEBUG_TYPE_CODEVIEW = 2,
};

namespace cppstacksize {
class PE_Magic_Mismatch_Error : public std::exception {
 public:
  const char* what() const noexcept override { return "PE magic mismatched"; }
};

struct PE_Section {
  std::u8string name;
  U32 virtual_size;
  U32 virtual_address;
  U32 data_size;
  U32 data_file_offset;
};

struct PE_Debug_Directory_Entry {
  U32 type;
  U32 data_size;
  U32 data_rva;
  U32 data_file_offset;
};

// A Windows PE (.dll or .exe) or COFF (.obj) file.
template <class Reader>
struct PE_File {
  Reader* reader;
  std::vector<PE_Section> sections;
  std::vector<PE_Debug_Directory_Entry> debug_directory;

  explicit PE_File(Reader* reader) : reader(reader) {}

  // Returns a Reader for each section with the given name.
  std::vector<Sub_File_Reader<Reader>> find_sections_by_name(
      std::u8string_view section_name) const {
    std::vector<Sub_File_Reader<Reader>> found_sections;
    for (const PE_Section& section : this->sections) {
      if (section.name == section_name) {
        found_sections.push_back(this->reader_for_section(section));
      }
    }
    return found_sections;
  }

  void parse_sections_(U64 offset) {
    U16 coff_magic = this->reader->u16(offset);
    if (coff_magic != 0x8664) {
      throw PE_Magic_Mismatch_Error();
    }
    U32 section_count = this->reader->u16(offset + 2);
    U64 optional_header_size = this->reader->u16(offset + 16);
    U64 section_table_offset = offset + 20 + optional_header_size;
    for (U32 section_index = 0; section_index < section_count;
         ++section_index) {
      this->sections.push_back(parse_coff_section(
          *this->reader, section_table_offset + section_index * 40));
    }
  }

  void parse_optional_header_(U64 coff_header_offset) {
    U64 optional_header_size = this->reader->u16(coff_header_offset + 16);
    if (optional_header_size == 0) {
      throw std::runtime_error("missing optional header");
    }

    Sub_File_Reader<Reader> optional_header_reader(
        this->reader, coff_header_offset + 20, optional_header_size);
    U16 optional_header_magic = optional_header_reader.u16(0);
    U64 data_directory_offset;
    if (optional_header_magic == 0x10b) {
      // PE32
      data_directory_offset = 96;
    } else if (optional_header_magic == 0x20b) {
      // PE32+
      data_directory_offset = 112;
    } else {
      throw std::runtime_error("unexpected optional header magic");
    }

    Sub_File_Reader<Sub_File_Reader<Reader>> data_directory_reader(
        &optional_header_reader, data_directory_offset);
    bool hasDebugDataDirectory = data_directory_reader.size() >= 56;
    if (!hasDebugDataDirectory) {
      return;
    }
    U32 debug_data_directory_rva = data_directory_reader.u32(48);
    U32 debug_data_directory_size = data_directory_reader.u32(48 + 4);
    Sub_File_Reader<Reader> debug_directory_reader = this->reader_for_rva(
        debug_data_directory_rva, debug_data_directory_size);

    U64 offset = 0;
    while (offset < debug_directory_reader.size()) {
      this->debug_directory.push_back(PE_Debug_Directory_Entry{
          .type = debug_directory_reader.u32(offset + 12),
          .data_size = debug_directory_reader.u32(offset + 16),
          .data_rva = debug_directory_reader.u32(offset + 20),
          .data_file_offset = debug_directory_reader.u32(offset + 24),
      });
      offset += 28;
    }
  }

  Sub_File_Reader<Reader> reader_for_section(const PE_Section& section) const {
    return Sub_File_Reader(this->reader, section.data_file_offset,
                           section.data_size);
  }

  Sub_File_Reader<Reader> reader_for_rva(U64 base_rva, U64 size) {
    // TODO(strager): Intelligent fallback if input spans multiple sections.
    // TODO(strager): Support 0 padding if baseRVA+size extends beyond dataSize.
    for (const PE_Section& section : this->sections) {
      bool in_bounds = section.virtual_address <= base_rva &&
                       base_rva + size < section.virtual_address +
                                             std::min(section.virtual_size,
                                                      section.data_size);
      if (in_bounds) {
        return Sub_File_Reader<Reader>(
            this->reader,
            base_rva - section.virtual_address + section.data_file_offset,
            size);
      }
    }
    throw std::runtime_error("cannot find section for RVA");
  }
};

template <class Reader>
inline PE_File<Reader> parse_pe_file(Reader* reader) {
  PE_File<Reader> pe(reader);

  bool is_dos_executable = reader->u16(0) == 0x5a4d;  // "MZ"
  if (is_dos_executable) {
    U64 pe_signature_offset = reader->u32(0x3c);
    if (reader->u32(pe_signature_offset) != 0x00004550) {  // "PE\0\0"
      throw PE_Magic_Mismatch_Error();
    }
    U64 coff_header_offset = pe_signature_offset + 4;
    pe.parse_sections_(coff_header_offset);
    pe.parse_optional_header_(coff_header_offset);
  } else {
    pe.parse_sections_(0);
  }

  return pe;
}

struct External_PDB_File_Reference {
  std::u8string pdb_path;
  // GUID pdbGUID;
};

template <class Reader>
inline std::optional<External_PDB_File_Reference> get_pe_pdb_reference(
    PE_File<Reader>& pe) {
  for (const PE_Debug_Directory_Entry& entry : pe.debug_directory) {
    if (entry.type == IMAGE_DEBUG_TYPE_CODEVIEW) {
      Sub_File_Reader<Reader> data_reader(pe.reader, entry.data_file_offset,
                                          entry.data_size);
      std::optional<External_PDB_File_Reference> result =
          parse_code_view_debug_directory_data(data_reader);
      if (result.has_value()) {
        return result;
      }
    }
  }
  return std::nullopt;
}

template <class Reader>
inline PE_Section parse_coff_section(Reader& reader, U64 offset) {
  return PE_Section{
      .name = reader.fixed_width_string(offset, 8),
      .virtual_size = reader.u32(offset + 8),
      .virtual_address = reader.u32(offset + 12),
      .data_size = reader.u32(offset + 16),
      .data_file_offset = reader.u32(offset + 20),
  };
}

template <class Reader>
inline std::optional<External_PDB_File_Reference>
parse_code_view_debug_directory_data(Reader& reader) {
  U32 magic = reader.u32(0);
  if (magic != 0x53445352) {
    // "RSDS"
    return std::nullopt;
  }
  std::u8string pdb_path = reader.utf_8_c_string(24);
  // TODO(port): GUID.
  // let pdbGUIDBytes = new Uint8Array(16);
  // reader.copyBytesInto(pdbGUIDBytes, 4, 16);
  return External_PDB_File_Reference{
      .pdb_path = std::move(pdb_path),
      //.pdb_guid = new GUID(pdbGUIDBytes),
  };
}
}

#endif
