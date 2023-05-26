#include <cppstacksize/hello.h>
#include <gtest/gtest.h>

using U8 = std::uint8_t;

TEST(Test_Hello, hello) {
  const U8 code[] = {
      // @asm-begin x86_64
      0x89, 0xcb,  // @asm mov %ecx, %ebx
      0x90,        // @asm nop
  };               // @asm-end
  ASSERT_EQ(2 + 2, hello());
}
