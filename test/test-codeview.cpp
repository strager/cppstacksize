#include <cppstacksize/codeview.h>
#include <cppstacksize/example-file.h>
#include <cppstacksize/pe.h>
#include <gmock/gmock.h>
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

TEST(Test_CodeView, primitives_obj_function_has_local_variables) {
  Example_File file("coff/primitives.obj");
  PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;
  Reader section_reader = pe.find_sections_by_name(u8".debug$S").at(0);

  CodeView_Function<Reader> func =
      find_all_codeview_functions(&section_reader).at(0);
  std::vector<CodeView_Function_Local<Reader>> locals =
      get_codeview_function_locals(func.reader, func.byte_offset);

  std::map<std::u8string, CodeView_Function_Local<Reader>*> locals_by_name;
  std::vector<std::u8string> local_names;
  for (CodeView_Function_Local<Reader>& local : locals) {
    // Duplicate locals are not allowed.
    EXPECT_EQ(locals_by_name[local.name], nullptr);

    locals_by_name[local.name] = &local;
    local_names.push_back(local.name);
  }
  EXPECT_THAT(local_names, ::testing::UnorderedElementsAreArray({
                               u8"c",
                               u8"d",
                               u8"f",
                               u8"ld",
                               u8"sc",
                               u8"si",
                               u8"sl",
                               u8"sll",
                               u8"ss",
                               u8"uc",
                               u8"ui",
                               u8"ul",
                               u8"ull",
                               u8"us",
                               u8"wc",
                           }));

  EXPECT_EQ(locals_by_name[u8"uc"]->sp_offset, 0);
  EXPECT_EQ(locals_by_name[u8"sc"]->sp_offset, 1);
  EXPECT_EQ(locals_by_name[u8"c"]->sp_offset, 2);
  EXPECT_EQ(locals_by_name[u8"us"]->sp_offset, 4);
  EXPECT_EQ(locals_by_name[u8"ss"]->sp_offset, 8);
  EXPECT_EQ(locals_by_name[u8"wc"]->sp_offset, 12);
  EXPECT_EQ(locals_by_name[u8"ui"]->sp_offset, 16);
  EXPECT_EQ(locals_by_name[u8"ul"]->sp_offset, 20);
  EXPECT_EQ(locals_by_name[u8"si"]->sp_offset, 24);
  EXPECT_EQ(locals_by_name[u8"sl"]->sp_offset, 28);
  EXPECT_EQ(locals_by_name[u8"f"]->sp_offset, 32);
  EXPECT_EQ(locals_by_name[u8"ull"]->sp_offset, 40);
  EXPECT_EQ(locals_by_name[u8"sll"]->sp_offset, 48);
  EXPECT_EQ(locals_by_name[u8"d"]->sp_offset, 56);
  EXPECT_EQ(locals_by_name[u8"ld"]->sp_offset, 64);
}
}
}
