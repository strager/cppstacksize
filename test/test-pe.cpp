#include <cppstacksize/example-file.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/reader.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_PE, pe_file_sections_in_pdb_example_dll) {
  Example_File file("pdb/example.dll");
  PE_File pe = parse_pe_file(&file.reader());

  // Data according to: dumpbin.exe /HEADERS
  ASSERT_EQ(pe.sections.size(), 5);
  EXPECT_EQ(pe.sections[0].name, u8".text");
  EXPECT_EQ(pe.sections[0].data_size, 0xe00);
  EXPECT_EQ(pe.sections[0].data_file_offset, 0x400);
  EXPECT_EQ(pe.sections[1].name, u8".rdata");
  EXPECT_EQ(pe.sections[1].data_size, 0xe00);
  EXPECT_EQ(pe.sections[1].data_file_offset, 0x1200);
  EXPECT_EQ(pe.sections[2].name, u8".data");
  EXPECT_EQ(pe.sections[2].data_size, 0x200);
  EXPECT_EQ(pe.sections[2].data_file_offset, 0x2000);
  EXPECT_EQ(pe.sections[3].name, u8".pdata");
  EXPECT_EQ(pe.sections[3].data_size, 0x200);
  EXPECT_EQ(pe.sections[3].data_file_offset, 0x2200);
  EXPECT_EQ(pe.sections[4].name, u8".reloc");
  EXPECT_EQ(pe.sections[4].data_size, 0x200);
  EXPECT_EQ(pe.sections[4].data_file_offset, 0x2400);
}

TEST(Test_PE, pe_debug_directory_in_pdb_example_dll) {
  Example_File file("pdb/example.dll");
  PE_File pe = parse_pe_file(&file.reader());

  // Data according to: dumpbin.exe /HEADERS
  ASSERT_EQ(pe.debug_directory.size(), 3);
  EXPECT_EQ(pe.debug_directory[0].type, 2)
      << "IMAGE_DEBUG_TYPE_CODEVIEW ('cv')";
  EXPECT_EQ(pe.debug_directory[0].data_size, 0x5e);
  EXPECT_EQ(pe.debug_directory[0].data_rva, 0x23d0);
  EXPECT_EQ(pe.debug_directory[0].data_file_offset, 0x15d0);
  EXPECT_EQ(pe.debug_directory[1].type, 12) << "undocumented ('feat')";
  EXPECT_EQ(pe.debug_directory[1].data_size, 0x14);
  EXPECT_EQ(pe.debug_directory[1].data_rva, 0x2430);
  EXPECT_EQ(pe.debug_directory[1].data_file_offset, 0x1630);
  EXPECT_EQ(pe.debug_directory[2].type, 13) << "undocumented ('coffgrp')";
  EXPECT_EQ(pe.debug_directory[2].data_size, 0x230);
  EXPECT_EQ(pe.debug_directory[2].data_rva, 0x2444);
  EXPECT_EQ(pe.debug_directory[2].data_file_offset, 0x1644);
}

TEST(Test_PE, pe_pdb_reference_in_pdb_example_dll) {
  Example_File file("pdb/example.dll");
  PE_File pe = parse_pe_file(&file.reader());
  std::optional<External_PDB_File_Reference> reference =
      get_pe_pdb_reference(pe);
  ASSERT_TRUE(reference.has_value());
  EXPECT_EQ(reference->pdb_guid.to_string(),
            "597c058d-affe-4abf-a0ea-76a2e3a3d099");
  EXPECT_EQ(reference->pdb_path,
            u8"C:"
            u8"\\Users\\strager\\Documents\\Projects\\cppstacksize\\test\\pdb\\"
            u8"example.pdb");
}

TEST(Test_PE, corrupted_pdb_example_dll_with_invalid_dos_signature) {
  Example_File original_file("pdb/example.dll");
  for (U64 i = 0; i < 2; ++i) {
    std::vector<U8> corrupted_file_buffer(original_file.data().begin(),
                                          original_file.data().end());
    corrupted_file_buffer[i] ^= 0xcc;
    Span_Reader file(corrupted_file_buffer);
    EXPECT_THROW({ parse_pe_file(&file); }, PE_Magic_Mismatch_Error);
  }
}

TEST(Test_PE, corrupted_pdb_example_dll_with_invalid_pe_signature) {
  Example_File original_file("pdb/example.dll");
  for (U64 i = 0; i < 4; ++i) {
    std::vector<U8> corrupted_file_buffer(original_file.data().begin(),
                                          original_file.data().end());
    corrupted_file_buffer[0x0100 + i] ^= 0xcc;
    Span_Reader file(corrupted_file_buffer);
    EXPECT_THROW({ parse_pe_file(&file); }, PE_Magic_Mismatch_Error);
  }
}
}
}
