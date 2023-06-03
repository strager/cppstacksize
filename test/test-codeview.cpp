#include <cppstacksize/codeview.h>
#include <cppstacksize/example-file.h>
#include <cppstacksize/pe.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
std::string u8string_to_string(const std::u8string& s) {
  return std::string(reinterpret_cast<const char*>(s.data()), s.size());
}

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

  struct Test_Case {
    std::u8string local_name;
    U64 expected_byte_size;
    std::u8string_view expected_name;
  };
  CodeView_Type_Table<Reader>* type_table =
      nullptr;  // Primitive types do not need the type table.
  static const Test_Case expected_local_types[] = {
      {u8"c", 1, u8"char"},
      {u8"sc", 1, u8"signed char"},
      {u8"uc", 1, u8"unsigned char"},
      {u8"ss", 2, u8"short"},
      {u8"us", 2, u8"unsigned short"},
      {u8"f", 4, u8"float"},
      {u8"si", 4, u8"int"},
      {u8"sl", 4, u8"long"},
      {u8"ui", 4, u8"unsigned"},
      {u8"ul", 4, u8"unsigned long"},
      {u8"wc", 4, u8"wchar_t"},
      {u8"d", 8, u8"double"},
      // NOTE(strager): On Microsoft x64, long double is the same as double.
      {u8"ld", 8, u8"double"},
      {u8"sll", 8, u8"long long"},
      {u8"ull", 8, u8"unsigned long long"},
  };
  for (const Test_Case& test_case : expected_local_types) {
    SCOPED_TRACE("localName = '" + u8string_to_string(test_case.local_name) +
                 "'");
    std::optional<CodeView_Type> actual_type =
        locals_by_name[test_case.local_name]->get_type(type_table);
    ASSERT_TRUE(actual_type.has_value());
    EXPECT_EQ(actual_type->byte_size, test_case.expected_byte_size);
    EXPECT_EQ(actual_type->name, test_case.expected_name);
  }
}

TEST(Test_CodeView, loads_all_variables_in_all_blocks) {
  Example_File file("coff/block.obj");
  PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;
  Reader section_reader = pe.find_sections_by_name(u8".debug$S").at(0);

  CodeView_Function<Reader> func =
      find_all_codeview_functions(&section_reader).at(0);
  std::vector<CodeView_Function_Local<Reader>> locals =
      get_codeview_function_locals(func.reader, func.byte_offset);

  std::vector<std::u8string> local_names;
  for (CodeView_Function_Local<Reader>& local : locals) {
    local_names.push_back(local.name);
  }

  EXPECT_THAT(local_names, ::testing::UnorderedElementsAreArray({
                               u8"before_blocks",
                               u8"before_innermost_blocks",
                               u8"inside_first_innermost_block",
                               u8"between_innermost_blocks",
                               u8"inside_second_innermost_block",
                               u8"after_innermost_blocks",
                               u8"after_blocks",
                           }));
}

TEST(Test_CodeView, codeview_special_types) {
  struct Test_Case {
    U32 type_id;
    U8 byte_size;
    std::u8string_view name;
  };

  static constexpr Test_Case test_cases[] = {
      // Normal non-pointer types:
      {.type_id = 0x10, .byte_size = 1, .name = u8"signed char"},
      {.type_id = 0x11, .byte_size = 2, .name = u8"short"},
      {.type_id = 0x12, .byte_size = 4, .name = u8"long"},
      {.type_id = 0x13, .byte_size = 8, .name = u8"long long"},
      {.type_id = 0x20, .byte_size = 1, .name = u8"unsigned char"},
      {.type_id = 0x21, .byte_size = 2, .name = u8"unsigned short"},
      {.type_id = 0x22, .byte_size = 4, .name = u8"unsigned long"},
      {.type_id = 0x23, .byte_size = 8, .name = u8"unsigned long long"},
      {.type_id = 0x30, .byte_size = 1, .name = u8"bool"},
      {.type_id = 0x31, .byte_size = 2, .name = u8"bool(u16)"},
      {.type_id = 0x32, .byte_size = 4, .name = u8"bool(u32)"},
      {.type_id = 0x33, .byte_size = 8, .name = u8"bool(u64)"},
      {.type_id = 0x40, .byte_size = 4, .name = u8"float"},
      {.type_id = 0x41, .byte_size = 8, .name = u8"double"},
      {.type_id = 0x70, .byte_size = 1, .name = u8"char"},
      {.type_id = 0x71, .byte_size = 4, .name = u8"wchar_t"},
      {.type_id = 0x74, .byte_size = 4, .name = u8"int"},
      {.type_id = 0x75, .byte_size = 4, .name = u8"unsigned"},

      // 64-bit pointer types:
      {.type_id = 0x603, .byte_size = 8, .name = u8"void *"},
      {.type_id = 0x610, .byte_size = 8, .name = u8"signed char *"},
      {.type_id = 0x611, .byte_size = 8, .name = u8"short *"},
      {.type_id = 0x612, .byte_size = 8, .name = u8"long *"},
      {.type_id = 0x613, .byte_size = 8, .name = u8"long long *"},
      {.type_id = 0x620, .byte_size = 8, .name = u8"unsigned char *"},
      {.type_id = 0x621, .byte_size = 8, .name = u8"unsigned short *"},
      {.type_id = 0x622, .byte_size = 8, .name = u8"unsigned long *"},
      {.type_id = 0x623, .byte_size = 8, .name = u8"unsigned long long *"},
      {.type_id = 0x640, .byte_size = 8, .name = u8"float *"},
      {.type_id = 0x641, .byte_size = 8, .name = u8"double *"},
      {.type_id = 0x670, .byte_size = 8, .name = u8"char *"},
      {.type_id = 0x671, .byte_size = 8, .name = u8"wchar_t *"},
      {.type_id = 0x674, .byte_size = 8, .name = u8"int *"},
      {.type_id = 0x675, .byte_size = 8, .name = u8"unsigned *"},

      // Special types:
      {.type_id = 0x103, .byte_size = 8, .name = u8"std::nullptr_t"},
  };
  CodeView_Type_Table<Span_Reader>* type_table =
      nullptr;  // Primitive types do not need the type table.
  for (const Test_Case& test_case : test_cases) {
    SCOPED_TRACE("type_id = " + std::to_string(test_case.type_id));
    std::optional<CodeView_Type> actual_type =
        get_codeview_type(test_case.type_id, type_table);
    ASSERT_TRUE(actual_type.has_value());
    EXPECT_EQ(actual_type->byte_size, test_case.byte_size);
    EXPECT_EQ(actual_type->name, test_case.name);
  }
}
}
}
