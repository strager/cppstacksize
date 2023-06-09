#include <cppstacksize/codeview.h>
#include <cppstacksize/example-file.h>
#include <cppstacksize/line-tables.h>
#include <cppstacksize/project.h>
#include <cppstacksize/util.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

namespace cppstacksize {
namespace {
TEST(Test_Project, loads_complete_obj_file) {
  Example_File obj_file("coff/parameters-int-0-callee.obj");
  Project project;
  project.add_file("parameters-int-0-callee.obj",
                   std::move(obj_file).loaded_file());

  // Function table should load.
  std::span<const CodeView_Function> funcs = project.get_all_functions();
  ASSERT_GT(funcs.size(), 0);
  EXPECT_EQ(funcs[0].name, u8"callee");
  // Type table should load.
  CodeView_Type_Table* type_table = project.get_type_table();
  ASSERT_NE(type_table, nullptr);
  CodeView_Type_Table* type_index_table = project.get_type_index_table();
  ASSERT_NE(type_index_table, nullptr);
  EXPECT_EQ(funcs[0].get_caller_stack_size(*type_table, *type_index_table), 32);
}

TEST(Test_Project, loads_complete_pdb_file) {
  Example_File pdb_file("pdb/example.pdb");
  Project project;
  project.add_file("example.pdb", std::move(pdb_file).loaded_file());

  // Function table should load.
  std::span<const CodeView_Function> funcs = project.get_all_functions();
  ASSERT_GT(funcs.size(), 0);
  EXPECT_EQ(funcs[0].name, u8"callee");
  // Type tables should load.
  CodeView_Type_Table* type_table = project.get_type_table();
  ASSERT_NE(type_table, nullptr);
  CodeView_Type_Table* type_index_table = project.get_type_index_table();
  ASSERT_NE(type_index_table, nullptr);
  EXPECT_EQ(funcs[0].get_caller_stack_size(*type_table, *type_index_table), 40);
}

TEST(Test_Project, loads_unlinked_pdb_and_obj) {
  Example_File pdb_file("coff-pdb/example.pdb");
  Example_File obj_file("coff-pdb/example.obj");
  Project project;
  project.add_file("example.pdb", std::move(pdb_file).loaded_file());
  project.add_file("example.obj", std::move(obj_file).loaded_file());

  // Function table should load from .obj.
  std::span<const CodeView_Function> funcs = project.get_all_functions();
  ASSERT_GT(funcs.size(), 0);
  EXPECT_EQ(funcs[0].name, u8"callee");
  // Type tables should load from .pdb.
  CodeView_Type_Table* type_table = project.get_type_table();
  ASSERT_NE(type_table, nullptr);
  CodeView_Type_Table* type_index_table = project.get_type_index_table();
  ASSERT_NE(type_index_table, nullptr);
  EXPECT_EQ(funcs[0].get_caller_stack_size(*type_table, *type_index_table), 40);
}

TEST(Test_Project, loads_unlinked_obj_and_pdb) {
  Example_File obj_file("coff-pdb/example.obj");
  Example_File pdb_file("coff-pdb/example.pdb");
  Project project;
  project.add_file("example.obj", std::move(obj_file).loaded_file());
  project.add_file("example.pdb", std::move(pdb_file).loaded_file());

  // Function table should load from .obj.
  std::span<const CodeView_Function> funcs = project.get_all_functions();
  ASSERT_GT(funcs.size(), 0);
  EXPECT_EQ(funcs[0].name, u8"callee");
  // Type tables should load from .pdb.
  CodeView_Type_Table* type_table = project.get_type_table();
  ASSERT_NE(type_table, nullptr);
  CodeView_Type_Table* type_index_table = project.get_type_index_table();
  ASSERT_NE(type_index_table, nullptr);
  EXPECT_EQ(funcs[0].get_caller_stack_size(*type_table, *type_index_table), 40);
}

TEST(Test_Project, loads_functions_from_obj_after_loading_functionless_pdb) {
  Example_File obj_file("coff-pdb/example.obj");
  Example_File pdb_file("coff-pdb/example.pdb");
  Project project;
  project.add_file("example.pdb", std::move(pdb_file).loaded_file());

  // This .pdb file has no functions, only types.
  std::span<const CodeView_Function> funcs = project.get_all_functions();
  EXPECT_THAT(funcs, ::testing::IsEmpty());

  project.add_file("example.obj", std::move(obj_file).loaded_file());
  funcs = project.get_all_functions();
  EXPECT_GT(funcs.size(), 0)
      << "loading example.obj should have added functions";
}

TEST(Test_Project, loads_types_from_pdb_after_loading_typeless_obj) {
  Example_File obj_file("coff-pdb/example.obj");
  Example_File pdb_file("coff-pdb/example.pdb");
  Project project;
  project.add_file("example.obj", std::move(obj_file).loaded_file());

  // This .obj file has no types, only functions.
  EXPECT_EQ(project.get_type_table(), nullptr);
  EXPECT_EQ(project.get_type_index_table(), nullptr);

  project.add_file("example.pdb", std::move(pdb_file).loaded_file());
  EXPECT_NE(project.get_type_table(), nullptr);
  EXPECT_NE(project.get_type_index_table(), nullptr);
}

TEST(Test_Project, pdb_functions_link_to_loaded_dll) {
  for (U32 i = 0; i < 2; ++i) {
    Example_File pdb_reader("pdb-pe/temporary.pdb");
    Example_File dll_reader("pdb-pe/temporary.dll");
    Project project;
    if (i == 0) {
      project.add_file("temporary.pdb", std::move(pdb_reader).loaded_file());
      project.add_file("temporary.dll", std::move(dll_reader).loaded_file());
    } else {
      project.add_file("temporary.dll", std::move(dll_reader).loaded_file());
      project.add_file("temporary.pdb", std::move(pdb_reader).loaded_file());
    }

    // Function table should load from .pdb.
    std::span<const CodeView_Function> funcs = project.get_all_functions();
    std::map<std::u8string, const CodeView_Function*> funcs_by_name;
    for (const CodeView_Function& func : funcs) {
      funcs_by_name[func.name] = &func;
    }

    std::optional local_variable_bytes_reader =
        funcs_by_name[u8"local_variable"]->get_instruction_bytes_reader();
    ASSERT_TRUE(local_variable_bytes_reader.has_value());
    // TODO(strager): Assert that the reader is for the .dll file, not the .pdb
    // file or an unrelated file.
    EXPECT_EQ(local_variable_bytes_reader->locate(0).file_offset, 0x0400);
    EXPECT_EQ(local_variable_bytes_reader->size(), 0x39);

    std::optional temporary_reader =
        funcs_by_name[u8"temporary"]->get_instruction_bytes_reader();
    ASSERT_TRUE(temporary_reader.has_value());
    // TODO(strager): Assert that the reader is for the .dll file, not the .pdb
    // file or an unrelated file.
    EXPECT_EQ(temporary_reader->locate(0).file_offset, 0x0440);
    EXPECT_EQ(temporary_reader->size(), 0x39);
  }
}

TEST(Test_Project, map_function_bytes_to_source_lines) {
  Example_File pdb_reader("pdb-pe/line-numbers.pdb");
  Project project;
  project.add_file("line-numbers.pdb", std::move(pdb_reader).loaded_file());

  std::span<const CodeView_Function> funcs = project.get_all_functions();
  std::map<std::u8string, const CodeView_Function*> funcs_by_name;
  for (const CodeView_Function& func : funcs) {
    funcs_by_name[func.name] = &func;
  }

  Line_Tables* line_tables = project.get_line_tables();

  struct Test_Case {
    const char8_t* function_name;
    U32 instruction_offset;
    U32 line_number;
  };
  static constexpr Test_Case test_cases[] = {
      // clang-format off
      {.function_name = u8"foo", .instruction_offset = 0x00, .line_number = 3},
      {.function_name = u8"foo", .instruction_offset = 0x01, .line_number = 3},
      {.function_name = u8"foo", .instruction_offset = 0x04, .line_number = 4},
      {.function_name = u8"foo", .instruction_offset = 0x08, .line_number = 4},
      {.function_name = u8"foo", .instruction_offset = 0x09, .line_number = 5},
      {.function_name = u8"foo", .instruction_offset = 0x0d, .line_number = 5},
      {.function_name = u8"foo", .instruction_offset = 0x0e, .line_number = 2},  // line-numbers.inc
      {.function_name = u8"foo", .instruction_offset = 0x12, .line_number = 2},  // line-numbers.inc
      {.function_name = u8"foo", .instruction_offset = 0x13, .line_number = 7},
      {.function_name = u8"foo", .instruction_offset = 0x1c, .line_number = 8},
      // clang-format on
  };
  for (const Test_Case& test_case : test_cases) {
    SCOPED_TRACE(fmt::format("function name: {}",
                             u8string_to_string(test_case.function_name)));
    SCOPED_TRACE(
        fmt::format("instruction offset: {:#x}", test_case.instruction_offset));
    const CodeView_Function* func = funcs_by_name[test_case.function_name];
    ASSERT_NE(func, nullptr);
    ASSERT_FALSE(func->line_tables_handle.is_null());
    Line_Source_Info info = line_tables->source_info_for_offset(
        func->line_tables_handle, func->code_section_index,
        func->code_offset + test_case.instruction_offset);
    EXPECT_EQ(info.line_number, test_case.line_number);
  }
}
}
}
