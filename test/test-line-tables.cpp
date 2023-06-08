#include <cppstacksize/example-file.h>
#include <cppstacksize/line-tables.h>
#include <cppstacksize/pdb.h>
#include <gtest/gtest.h>

// TODO(strager): Switch to <format>.
#include <fmt/format.h>

namespace cppstacksize {
namespace {
TEST(Test_Line_Tables, out_of_bounds_line_source_info) {
  EXPECT_TRUE(Line_Source_Info::out_of_bounds().is_out_of_bounds());
  EXPECT_FALSE(Line_Source_Info{.line_number = 42}.is_out_of_bounds());
}

TEST(Test_Line_Tables, line_information_from_pdb) {
  Example_File pdb_file("pdb-pe/line-numbers.pdb");
  PDB_Super_Block super_block = parse_pdb_header(pdb_file.reader());
  auto pdb_streams =
      parse_pdb_stream_directory(&pdb_file.reader(), super_block);
  PDB_DBI dbi = parse_pdb_dbi_stream(pdb_streams.at(3));
  PDB_DBI_Module &dbi_module = dbi.modules.at(0);  // line-numbers.obj

  Line_Tables line_tables;
  Line_Tables::Handle h =
      line_tables.add_module_line_tables(dbi_module, pdb_streams);

  // Output of cvdump.exe line-numbers.pdb:
  //
  // [snip]\line-numbers.c, 0001:00000000-00000003, line/addr pairs = 1
  //     1 00000000
  //
  // [snip]\line-numbers.c, 0001:00000010-0000001E, line/addr pairs = 3
  //     3 00000010      4 00000014      5 00000019
  //
  // [snip]\line-numbers.inc, 0001:0000001E-00000023, line/addr pairs = 1
  //     2 0000001E
  //
  // [snip]\line-numbers.c, 0001:00000023-0000002D, line/addr pairs = 2
  //     7 00000023      8 00000028
  //
  // [snip]\line-numbers.c, 0001:00000040-00000053, line/addr pairs = 3
  //    11 00000040     12 00000044     13 0000004E
  //
  // [snip]\line-numbers.c, 0001:00000060-00000063, line/addr pairs = 1
  //    16 00000060

  struct Test_Case {
    U32 instruction_offset;
    U32 line_number;
  };
  static constexpr Test_Case test_cases[] = {
      {.instruction_offset = 0x10, .line_number = 3},
      {.instruction_offset = 0x11, .line_number = 3},
      {.instruction_offset = 0x14, .line_number = 4},
      {.instruction_offset = 0x18, .line_number = 4},
      {.instruction_offset = 0x19, .line_number = 5},
      {.instruction_offset = 0x1d, .line_number = 5},
      {.instruction_offset = 0x1e, .line_number = 2},  // line-numbers.inc
      {.instruction_offset = 0x22, .line_number = 2},  // line-numbers.inc
      {.instruction_offset = 0x23, .line_number = 7},
      {.instruction_offset = 0x2c, .line_number = 8},
  };
  for (const Test_Case &test_case : test_cases) {
    SCOPED_TRACE(fmt::format("{:#x}", test_case.instruction_offset));
    Line_Source_Info info =
        line_tables.source_info_for_offset(h, 0, test_case.instruction_offset);
    EXPECT_EQ(info.line_number, test_case.line_number);
  }

  static constexpr U32 out_of_bounds_instruction_offsets[] = {0x03, 0x0f, 0x63,
                                                              0xffffffff};
  for (U32 instruction_offset : out_of_bounds_instruction_offsets) {
    SCOPED_TRACE(fmt::format("{:#x}", instruction_offset));
    Line_Source_Info info =
        line_tables.source_info_for_offset(h, 0, instruction_offset);
    EXPECT_TRUE(info.is_out_of_bounds()) << info;
  }
}

// TODO[obj-lines]: Exact line information from .obj files. Parsing
// DEBUG_S_LINES subsections probably requires redesigning
// find_all_codeview_functions (which currently parses DEBUG_S_SYMBOLS) (to
// avoid re-parsing subsections), and also redesigning
// Line_Tables::add_module_line_tables to support non-conjoined DEBUG_S_SYMBOLS
// sections.
}
}
