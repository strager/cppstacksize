#include <cppstacksize/codeview.h>
#include <cppstacksize/example-file.h>
#include <cppstacksize/pe.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_CodeView, primitives_obj_has_one_function) {
  Example_File file("coff/primitives.obj");
  PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;
  Reader section_reader = pe.find_sections_by_name(u8".debug$S").at(0);

  std::vector<CodeView_Function<Reader>> functions;
  find_all_codeview_functions(&section_reader, functions);
  ASSERT_EQ(functions.size(), 1);
  EXPECT_EQ(functions[0].name, u8"primitives");
  EXPECT_EQ(functions[0].reader.locate(functions[0].byte_offset).file_offset,
            0x237);
  EXPECT_EQ(functions[0].self_stack_size, 88);
  // TODO[coff-relocations]: Perform relocations to get the correct
  // codeSectionIndex and codeOffset.
  if (false) {
    EXPECT_EQ(functions[0].code_section_index, 2);  // .text$mn
    EXPECT_EQ(functions[0].code_offset, 0);
  }
  EXPECT_EQ(functions[0].code_size, 124);
}
}
}
