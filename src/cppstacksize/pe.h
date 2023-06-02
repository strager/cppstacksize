#ifndef CPPSTACKSIZE_PE_H
#define CPPSTACKSIZE_PE_H

#include <cppstacksize/base.h>
#include <cppstacksize/reader.h>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

// Documentation:
// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format

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

// A Windows PE (.dll or .exe) or COFF (.obj) file.
template <class Reader>
struct PE_File {
  Reader* reader;
  std::vector<PE_Section> sections;

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

  Sub_File_Reader<Reader> reader_for_section(const PE_Section& section) const {
    return Sub_File_Reader(this->reader, section.data_file_offset,
                           section.data_size);
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
    // pe.parse_optional_header_(coff_header_offset);
  } else {
    pe.parse_sections_(0);
  }

  return pe;
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
}

#endif
