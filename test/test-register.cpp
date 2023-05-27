#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/asm.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_Register, mov_immediate_to_register_changes_register) {
  {
    static constexpr U8 code[] = ASM_X86_64(
        // mov $0x50, %rax
        0x48, 0xc7, 0xc0, 0x50, 0x00, 0x00, 0x00, );
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x50));
  }

  {
    static constexpr U8 code[] = ASM_X86_64(
        // mov $0x123456789abcdef0, %rax
        0x48, 0xb8, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, );
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x123456789abcdef0));
  }
}

TEST(Test_Register, mov32_sets_entire_64_bits) {
  {
    static constexpr U8 code[] = ASM_X86_64(
        // mov $0x123456789abcdef0, %rax
        // mov $0x69, %eax
        0x48, 0xb8, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0xb8, 0x69,
        0x00, 0x00, 0x00, );
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x69));
  }
}
}
}