#include <cppstacksize/codeview.h>
#include <cppstacksize/example-file.h>
#include <cppstacksize/pdb.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/util.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_CodeView, primitives_obj_has_one_function) {
  Example_File file("coff/primitives.obj");
  PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;
  Reader section_reader = pe.find_sections_by_name(u8".debug$S").at(0);

  std::vector<CodeView_Function> functions;
  find_all_codeview_functions(&section_reader, functions);
  ASSERT_EQ(functions.size(), 1);
  EXPECT_EQ(functions[0].name, u8"primitives");
  EXPECT_EQ(functions[0].location().file_offset, 0x237);
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

  CodeView_Function func = find_all_codeview_functions(&section_reader).at(0);
  std::vector<CodeView_Function_Local> locals =
      func.get_locals(func.byte_offset);

  std::map<std::u8string, CodeView_Function_Local*> locals_by_name;
  std::vector<std::u8string> local_names;
  for (CodeView_Function_Local& local : locals) {
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
  CodeView_Type_Table* type_table =
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

  CodeView_Function func = find_all_codeview_functions(&section_reader).at(0);
  std::vector<CodeView_Function_Local> locals =
      func.get_locals(func.byte_offset);

  std::vector<std::u8string> local_names;
  for (CodeView_Function_Local& local : locals) {
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

TEST(Test_CodeView, int_parameters) {
  struct Test_Case {
    const char* description;
    const char* obj_name;
    U64 expected_caller_stack_size;
  };

  static constexpr Test_Case test_cases[] = {
      {
          .description =
              "void callee(void) has shadow space for four registers",
          .obj_name = "coff/parameters-int-0-callee.obj",
          .expected_caller_stack_size = 32,
      },
      {
          .description = "void callee(int) has shadow space for four registers",
          .obj_name = "coff/parameters-int-1-callee.obj",
          .expected_caller_stack_size = 32,
      },
      {
          .description =
              "void callee(int, int) has shadow space for four registers",
          .obj_name = "coff/parameters-int-2-callee.obj",
          .expected_caller_stack_size = 32,
      },
      {
          .description = "void callee(int, int, int, int) has shadow space for "
                         "four registers",
          .obj_name = "coff/parameters-int-4-callee.obj",
          .expected_caller_stack_size = 32,
      },
      {
          .description = "void callee(int, int, int, int, int) has shadow "
                         "space for four registers plus space for int on stack",
          .obj_name = "coff/parameters-int-5-callee.obj",
          .expected_caller_stack_size = 40,
      },
      {
          .description =
              "void callee(int, int, int, int, int, int) has shadow space for "
              "four registers plus space for two ints on stack",
          .obj_name = "coff/parameters-int-6-callee.obj",
          .expected_caller_stack_size = 48,
      },
  };

  for (const Test_Case& test_case : test_cases) {
    SCOPED_TRACE(test_case.description);
    Example_File file(test_case.obj_name);
    PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
    using Reader = Sub_File_Reader<Span_Reader>;

    Reader types_section_reader = pe.find_sections_by_name(u8".debug$T").at(0);
    CodeView_Type_Table type_table =
        parse_codeview_types(&types_section_reader);
    Reader symbols_section_reader =
        pe.find_sections_by_name(u8".debug$S").at(1);
    CodeView_Function func =
        find_all_codeview_functions(&symbols_section_reader).at(0);

    EXPECT_EQ(func.get_caller_stack_size(type_table),
              test_case.expected_caller_stack_size);
  }
}

TEST(Test_CodeView, member_function_parameters) {
  Example_File file("coff/member-function.obj");
  PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;

  Reader types_section_reader = pe.find_sections_by_name(u8".debug$T").at(0);
  CodeView_Type_Table type_table = parse_codeview_types(&types_section_reader);
  Reader symbols_section_reader = pe.find_sections_by_name(u8".debug$S").at(0);
  CodeView_Function func =
      find_all_codeview_functions(&symbols_section_reader).at(0);

  std::vector<CodeView_Function> functions;
  find_all_codeview_functions(&symbols_section_reader, functions);
  std::map<std::u8string, CodeView_Function*> func_by_name;
  for (CodeView_Function& func : functions) {
    func_by_name[func.name] = &func;
  }

  EXPECT_EQ(func_by_name[u8"S::v"]->get_caller_stack_size(type_table), 32)
      << "non-static void f() has shadow space for four registers";
  EXPECT_EQ(func_by_name[u8"S::v_i"]->get_caller_stack_size(type_table), 32)
      << "non-static void f(int) has shadow space for four registers";
  EXPECT_EQ(func_by_name[u8"S::v_iii"]->get_caller_stack_size(type_table), 32)
      << "non-static void f(int, int, int) has shadow space for four registers";
  EXPECT_EQ(func_by_name[u8"S::v_iiii"]->get_caller_stack_size(type_table), 40)
      << "non-static void f(int, int, int, int) has shadow space for four "
         "registers plus space for int on stack (implicit 'this')";
  EXPECT_EQ(
      func_by_name[u8"S::static_v_iiii"]->get_caller_stack_size(type_table), 32)
      << "static void f(int, int, int, int) has shadow space for four "
         "registers (no implicit 'this')";
}

TEST(Test_CodeView, split_coff_and_pdb_fails_to_load_type_info_from_coff) {
  Example_File file("coff-pdb/example.obj");
  PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;
  Reader section_reader = pe.find_sections_by_name(u8".debug$T").at(0);

  try {
    parse_codeview_types(&section_reader);
    ADD_FAILURE() << "parse_code_view_types should have thrown";
  } catch (CodeView_Types_In_Separate_PDB_File& error) {
    EXPECT_EQ(
        error.pdb_path,
        u8"C:\\Users\\strager\\Documents\\Projects\\cppstacksize\\test\\coff-"
        u8"pdb\\example.pdb");
    EXPECT_EQ(error.pdb_guid.to_string(),
              "015182d6-09fa-4590-89e2-5abf55ea3c33");
  }
}

TEST(Test_CodeView, coff_can_load_type_info_from_pdb_tpi_and_ipi) {
  Example_File obj_file("coff-pdb/example.obj");
  Example_File pdb_file("coff-pdb/example.pdb");

  auto pdb_streams = parse_pdb_stream_directory(
      &pdb_file.reader(), parse_pdb_header(pdb_file.reader()));
  auto pdb_tpi_header = parse_pdb_tpi_stream_header(&pdb_streams[2]);
  auto pdb_type_table =
      parse_codeview_types_without_header(&pdb_tpi_header.type_reader);
  auto pdb_ipi_header = parse_pdb_tpi_stream_header(&pdb_streams[4]);
  auto pdb_type_index_table =
      parse_codeview_types_without_header(&pdb_ipi_header.type_reader);

  PE_File<Span_Reader> obj = parse_pe_file(&obj_file.reader());
  using Reader = Sub_File_Reader<Span_Reader>;
  Reader coff_section_reader = obj.find_sections_by_name(u8".debug$S").at(0);
  std::vector<CodeView_Function> coff_funcs;
  find_all_codeview_functions(&coff_section_reader, coff_funcs);

  EXPECT_EQ(coff_funcs.at(0).get_caller_stack_size(pdb_type_table,
                                                   pdb_type_index_table),
            40);
}

TEST(Test_CodeView, function_code_offset_and_size_from_pdb) {
  Example_File pdb_file("pdb-pe/temporary.pdb");
  using Reader = PDB_Blocks_Reader<Span_Reader>;
  Reader codeview_reader(&pdb_file.reader(), {9},
                         /*block_size=*/4096,
                         /*byte_size=*/648,
                         /*stream_index=*/10);
  std::vector<CodeView_Function> functions;
  find_all_codeview_functions_2(&codeview_reader, functions);
  std::map<std::u8string, CodeView_Function*> functions_by_name;
  for (CodeView_Function& func : functions) {
    functions_by_name[func.name] = &func;
  }

  CodeView_Function* local_variable_func =
      functions_by_name[u8"local_variable"];
  EXPECT_EQ(local_variable_func->code_section_index, 0);
  EXPECT_EQ(local_variable_func->code_offset, 0x0000);
  EXPECT_EQ(local_variable_func->code_size, 57);

  CodeView_Function* temporary_func = functions_by_name[u8"temporary"];
  EXPECT_EQ(temporary_func->code_section_index, 0);
  EXPECT_EQ(temporary_func->code_offset, 0x0040);
  EXPECT_EQ(temporary_func->code_size, 57);

  CodeView_Function* start_func = functions_by_name[u8"_start"];
  EXPECT_EQ(start_func->code_section_index, 0);
  EXPECT_EQ(start_func->code_offset, 0x0080);
  EXPECT_EQ(start_func->code_size, 3);
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
  CodeView_Type_Table* type_table =
      nullptr;  // Primitive types do not need the type table.
  for (const Test_Case& test_case : test_cases) {
    SCOPED_TRACE("type_id = " + std::to_string(test_case.type_id));
    std::optional<CodeView_Type> actual_type =
        type_table->get_type(test_case.type_id);
    ASSERT_TRUE(actual_type.has_value());
    EXPECT_EQ(actual_type->byte_size, test_case.byte_size);
    EXPECT_EQ(actual_type->name, test_case.name);
  }
}

TEST(Test_CodeView, codeview_types) {
  struct Test_Case {
    U32 type_id;
    U32 byte_size;
    const char8_t* name;
  };

  struct Test_Case_Group {
    const char* obj_name;
    std::vector<Test_Case> test_cases;
  };

  static const Test_Case_Group test_case_groups[] = {
      {"coff/pointer.obj",
       {
           {.type_id = 0x1004,
            .byte_size = 8,
            .name = u8"const volatile int *"},
           {.type_id = 0x1005, .byte_size = 8, .name = u8"int **"},
           {.type_id = 0x1008, .byte_size = 8, .name = u8"const int *const *"},
           {.type_id = 0x1009, .byte_size = 8, .name = u8"const int *"},
           {.type_id = 0x100b, .byte_size = 8, .name = u8"volatile int *"},
           {.type_id = 0x100d, .byte_size = 8, .name = u8"int *const *"},
           {.type_id = 0x100e, .byte_size = 8, .name = u8"const int **"},
           {.type_id = 0x1010, .byte_size = 8, .name = u8"const void *"},
           {.type_id = 0x1011, .byte_size = 8, .name = u8"int ***"},
       }},

      {"coff/struct.obj",
       {
           // Pointers:
           {.type_id = 0x101e, .byte_size = 8, .name = u8"Empty_Struct *"},
           {.type_id = 0x1028,
            .byte_size = 8,
            .name = u8"Forward_Declared_Struct *"},
           {.type_id = 0x102d,
            .byte_size = 8,
            .name = u8"Struct_With_One_Int *"},
           {.type_id = 0x102e,
            .byte_size = 8,
            .name = u8"Struct_With_Vtable *"},
           {.type_id = 0x1030,
            .byte_size = 8,
            .name = u8"Struct_With_Bit_Field *"},
           {.type_id = 0x1032,
            .byte_size = 8,
            .name = u8"Struct_With_Two_Ints *"},

           // Non-pointers:
           {.type_id = 0x1015, .byte_size = 8, .name = u8"Struct_With_Vtable"},
           {.type_id = 0x101c, .byte_size = 1, .name = u8"Empty_Struct"},
           {.type_id = 0x1020,
            .byte_size = 8,
            .name = u8"Struct_With_Two_Ints"},
           {.type_id = 0x1025,
            .byte_size = 1,
            .name = u8"Struct_With_Bit_Field"},
           {.type_id = 0x102b, .byte_size = 4, .name = u8"Struct_With_One_Int"},
       }},

      {"coff/enum.obj",
       {
           {.type_id = 0x1004, .byte_size = 4, .name = u8"Basic_Enum"},
           {.type_id = 0x1008, .byte_size = 4, .name = u8"Enum_Class"},
           {.type_id = 0x100b, .byte_size = 1, .name = u8"Enum_Class_One_Byte"},
           {.type_id = 0x100d, .byte_size = 8, .name = u8"Basic_Enum *"},
           {.type_id = 0x100f, .byte_size = 4, .name = u8"Empty_Enum"},
       }},

      {"coff/union.obj",
       {
           // Pointers:
           {.type_id = 0x1004,
            .byte_size = 8,
            .name = u8"Forward_Declared_Union *"},
           {.type_id = 0x100a, .byte_size = 8, .name = u8"Empty_Union *"},
           {.type_id = 0x100f, .byte_size = 8, .name = u8"Union_With_Int *"},
           {.type_id = 0x1014,
            .byte_size = 8,
            .name = u8"Union_With_Int_And_Double *"},

           // Non-pointers:
           {.type_id = 0x1007, .byte_size = 1, .name = u8"Empty_Union"},
           {.type_id = 0x100d, .byte_size = 4, .name = u8"Union_With_Int"},
           {.type_id = 0x1012,
            .byte_size = 8,
            .name = u8"Union_With_Int_And_Double"},
       }},

      {"coff/function-type.obj",
       {
           {.type_id = 0x1003, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x1005, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x1006, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x1009, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x100b, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x100d, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x100e, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x100f, .byte_size = 8, .name = u8"<func> *"},
           {.type_id = 0x1010, .byte_size = 8, .name = u8"<func> *"},
       }},

      {"coff/array.obj",
       {
           {.type_id = 0x1004, .byte_size = 4 * 2 * 3, .name = u8"int[][]"},
           {.type_id = 0x1005, .byte_size = 8, .name = u8"int[]"},
           {.type_id = 0x1006, .byte_size = 4, .name = u8"int[]"},
       }},
  };

  for (const Test_Case_Group& test_case_group : test_case_groups) {
    SCOPED_TRACE(test_case_group.obj_name);
    Example_File file(test_case_group.obj_name);
    PE_File<Span_Reader> pe = parse_pe_file(&file.reader());
    using Reader = Sub_File_Reader<Span_Reader>;
    Reader types_section_reader = pe.find_sections_by_name(u8".debug$T").at(0);
    CodeView_Type_Table type_table =
        parse_codeview_types(&types_section_reader);

    for (const Test_Case& test_case : test_case_group.test_cases) {
      SCOPED_TRACE(test_case.type_id);
      SCOPED_TRACE(u8string_to_string(test_case.name));

      std::optional<CodeView_Type> actual_type =
          type_table.get_type(test_case.type_id);
      ASSERT_TRUE(actual_type.has_value());
      EXPECT_EQ(actual_type->byte_size, test_case.byte_size);
      EXPECT_EQ(actual_type->name, test_case.name);
    }
  }
}

TEST(
    Test_CodeView,
    find_all_codeview_functions_doesnt_crash_if_byte_after_last_entry_is_not_4_byte_aligned) {
  static constexpr U8 data[] = {
      // CV_SIGNATURE_C13
      0x04,
      0x00,
      0x00,
      0x00,

      // DEBUG_S_SYMBOLS type
      0xf1,
      0x00,
      0x00,
      0x00,
      // DEBUG_S_SYMBOLS size
      0x09,
      0x00,
      0x00,
      0x00,

      // S_COMPILE size
      0x07,
      0x00,
      // S_COMPILE type
      0x01,
      0x00,
      // S_COMPILE flags
      0x00,
      0x00,
      0x00,
      0x00,
      // S_COMPILE version (0-terminated string)
      0x00,

      // Padding
      0x00,
      0x00,
      0x00,
  };
  Span_Reader reader(data);
  std::vector<CodeView_Function> funcs = find_all_codeview_functions(&reader);
  EXPECT_THAT(funcs, ::testing::IsEmpty());
}
}
}
