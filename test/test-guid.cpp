#include <cppstacksize/base.h>
#include <cppstacksize/guid.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_GUID, parses_binary_then_stringifies) {
  static const U8 bytes[] = {
      0x8d, 0x05, 0x7c, 0x59, 0xfe, 0xaf, 0xbf, 0x4a,
      0xa0, 0xea, 0x76, 0xa2, 0xe3, 0xa3, 0xd0, 0x99,
  };
  GUID guid(bytes);
  EXPECT_EQ(guid.to_string(), "597c058d-affe-4abf-a0ea-76a2e3a3d099");
}

TEST(Test_GUID, string_includes_0_padding) {
  static const U8 bytes[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  GUID guid(bytes);
  EXPECT_EQ(guid.to_string(), "00000000-0000-0000-0000-000000000000");
}
}
}
