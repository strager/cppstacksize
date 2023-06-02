#include <cppstacksize/example-file.h>
#include <cppstacksize/pe.h>
#include <cppstacksize/reader.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_COFF, find_coff_sections_by_name_in_small_obj) {
  Example_File file("coff/small.obj");
  PE_File pe = parse_pe_file(&file.reader());
  std::vector<Sub_File_Reader<Span_Reader>> section_readers =
      pe.find_sections_by_name(u8".debug$S");

  // Data according to: dumpbin.exe /HEADERS
  ASSERT_EQ(section_readers.size(), 1);
  EXPECT_EQ(section_readers[0].base_reader(), &file.reader());
  EXPECT_EQ(section_readers[0].sub_file_offset(), 0x10b);
  EXPECT_EQ(section_readers[0].size(), 0x1e8);
}

TEST(Test_COFF, small_obj_sections) {
  Example_File file("coff/small.obj");
  PE_File pe = parse_pe_file(&file.reader());

  // Data according to: dumpbin.exe /HEADERS
  ASSERT_EQ(pe.sections.size(), 5);
  EXPECT_EQ(pe.sections[0].name, u8".drectve");
  EXPECT_EQ(pe.sections[0].data_size, 0x2f);
  EXPECT_EQ(pe.sections[0].data_file_offset, 0xdc);
  EXPECT_EQ(pe.sections[1].name, u8".debug$S");
  EXPECT_EQ(pe.sections[1].data_size, 0x1e8);
  EXPECT_EQ(pe.sections[1].data_file_offset, 0x10b);
  EXPECT_EQ(pe.sections[2].name, u8".text$mn");
  EXPECT_EQ(pe.sections[2].data_size, 0x3);
  EXPECT_EQ(pe.sections[2].data_file_offset, 0x31b);
  EXPECT_EQ(pe.sections[3].name, u8".debug$T");
  EXPECT_EQ(pe.sections[3].data_size, 0x68);
  EXPECT_EQ(pe.sections[3].data_file_offset, 0x31e);
  EXPECT_EQ(pe.sections[4].name, u8".chks64");
  EXPECT_EQ(pe.sections[4].data_size, 0x28);
  EXPECT_EQ(pe.sections[4].data_file_offset, 0x386);
}
}
}
