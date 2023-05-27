#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/asm.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_Register, mov_immediate_to_register_changes_register) {
  {
    std::span<const U8> code = ASM_X86_64("mov $0x50, %rax");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x50));
  }

  {
    // clang-format off
    std::span<const U8> code = ASM_X86_64(
        "mov $1, %rax"
        "mov $2, %rbx"
        "mov $3, %rcx"
        "mov $4, %rdx"
        "mov $5, %rsi"
        "mov $6, %rdi"
        "mov $7, %rbp"
        "mov $8, %r8"
        "mov $9, %r9"
        "mov $10, %r10"
        "mov $11, %r11"
        "mov $12, %r12"
        "mov $13, %r13"
        "mov $14, %r14"
        "mov $15, %r15"
        );
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax], Register_Value::make_literal(1));
    EXPECT_EQ(sm.registers.values[Register_Name::rbx], Register_Value::make_literal(2));
    EXPECT_EQ(sm.registers.values[Register_Name::rcx], Register_Value::make_literal(3));
    EXPECT_EQ(sm.registers.values[Register_Name::rdx], Register_Value::make_literal(4));
    EXPECT_EQ(sm.registers.values[Register_Name::rsi], Register_Value::make_literal(5));
    EXPECT_EQ(sm.registers.values[Register_Name::rdi], Register_Value::make_literal(6));
    EXPECT_EQ(sm.registers.values[Register_Name::rbp], Register_Value::make_literal(7));
    EXPECT_EQ(sm.registers.values[Register_Name::r8],  Register_Value::make_literal(8));
    EXPECT_EQ(sm.registers.values[Register_Name::r9],  Register_Value::make_literal(9));
    EXPECT_EQ(sm.registers.values[Register_Name::r10], Register_Value::make_literal(10));
    EXPECT_EQ(sm.registers.values[Register_Name::r11], Register_Value::make_literal(11));
    EXPECT_EQ(sm.registers.values[Register_Name::r12], Register_Value::make_literal(12));
    EXPECT_EQ(sm.registers.values[Register_Name::r13], Register_Value::make_literal(13));
    EXPECT_EQ(sm.registers.values[Register_Name::r14], Register_Value::make_literal(14));
    EXPECT_EQ(sm.registers.values[Register_Name::r15], Register_Value::make_literal(15));
    // clang-format on
  }

  {
    // clang-format off
    std::span<const U8> code = ASM_X86_64(
        "mov $1, %eax"
        "mov $2, %ebx"
        "mov $3, %ecx"
        "mov $4, %edx"
        "mov $5, %esi"
        "mov $6, %edi"
        "mov $7, %ebp"
        "mov $8, %r8d"
        "mov $9, %r9d"
        "mov $10, %r10d"
        "mov $11, %r11d"
        "mov $12, %r12d"
        "mov $13, %r13d"
        "mov $14, %r14d"
        "mov $15, %r15d"
        );
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax], Register_Value::make_literal(1));
    EXPECT_EQ(sm.registers.values[Register_Name::rbx], Register_Value::make_literal(2));
    EXPECT_EQ(sm.registers.values[Register_Name::rcx], Register_Value::make_literal(3));
    EXPECT_EQ(sm.registers.values[Register_Name::rdx], Register_Value::make_literal(4));
    EXPECT_EQ(sm.registers.values[Register_Name::rsi], Register_Value::make_literal(5));
    EXPECT_EQ(sm.registers.values[Register_Name::rdi], Register_Value::make_literal(6));
    EXPECT_EQ(sm.registers.values[Register_Name::rbp], Register_Value::make_literal(7));
    EXPECT_EQ(sm.registers.values[Register_Name::r8],  Register_Value::make_literal(8));
    EXPECT_EQ(sm.registers.values[Register_Name::r9],  Register_Value::make_literal(9));
    EXPECT_EQ(sm.registers.values[Register_Name::r10], Register_Value::make_literal(10));
    EXPECT_EQ(sm.registers.values[Register_Name::r11], Register_Value::make_literal(11));
    EXPECT_EQ(sm.registers.values[Register_Name::r12], Register_Value::make_literal(12));
    EXPECT_EQ(sm.registers.values[Register_Name::r13], Register_Value::make_literal(13));
    EXPECT_EQ(sm.registers.values[Register_Name::r14], Register_Value::make_literal(14));
    EXPECT_EQ(sm.registers.values[Register_Name::r15], Register_Value::make_literal(15));
    // clang-format on
  }

  {
    std::span<const U8> code = ASM_X86_64("mov $0x123456789abcdef0, %rax");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x123456789abcdef0));
  }
}

TEST(Test_Register, mov32_sets_entire_64_bits) {
  {
    std::span<const U8> code = ASM_X86_64(
        "mov $0x123456789abcdef0, %rax"
        "mov $0x69, %eax");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x69));
  }

  // Should not sign extend:
  {
    std::span<const U8> code = ASM_X86_64(
        "mov $0x123456789abcdef0, %rax"
        "mov $0xffffffff, %eax");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0xffffffff));
  }
}

TEST(Test_Register, mov16_preserves_other_bits) {
  {
    std::span<const U8> code = ASM_X86_64(
        "mov $0x123456789abcdef0, %rax"
        "mov $0x0420, %ax");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x123456789abc0420));
  }

  {
    // clang-format off
    std::span<const U8> code = ASM_X86_64(
        "mov $0xffffffffffffffff, %rax"
        "mov $0xffffffffffffffff, %rbx"
        "mov $0xffffffffffffffff, %rcx"
        "mov $0xffffffffffffffff, %rdx"
        "mov $0xffffffffffffffff, %rsi"
        "mov $0xffffffffffffffff, %rdi"
        "mov $0xffffffffffffffff, %rbp"
        "mov $0xffffffffffffffff, %r8"
        "mov $0xffffffffffffffff, %r9"
        "mov $0xffffffffffffffff, %r10"
        "mov $0xffffffffffffffff, %r11"
        "mov $0xffffffffffffffff, %r12"
        "mov $0xffffffffffffffff, %r13"
        "mov $0xffffffffffffffff, %r14"
        "mov $0xffffffffffffffff, %r15"
        "mov $1, %ax"
        "mov $2, %bx"
        "mov $3, %cx"
        "mov $4, %dx"
        "mov $5, %si"
        "mov $6, %di"
        "mov $7, %bp"
        "mov $8, %r8w"
        "mov $9, %r9w"
        "mov $10, %r10w"
        "mov $11, %r11w"
        "mov $12, %r12w"
        "mov $13, %r13w"
        "mov $14, %r14w"
        "mov $15, %r15w"
        );
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax], Register_Value::make_literal(0xffffffffffff0001));
    EXPECT_EQ(sm.registers.values[Register_Name::rbx], Register_Value::make_literal(0xffffffffffff0002));
    EXPECT_EQ(sm.registers.values[Register_Name::rcx], Register_Value::make_literal(0xffffffffffff0003));
    EXPECT_EQ(sm.registers.values[Register_Name::rdx], Register_Value::make_literal(0xffffffffffff0004));
    EXPECT_EQ(sm.registers.values[Register_Name::rsi], Register_Value::make_literal(0xffffffffffff0005));
    EXPECT_EQ(sm.registers.values[Register_Name::rdi], Register_Value::make_literal(0xffffffffffff0006));
    EXPECT_EQ(sm.registers.values[Register_Name::rbp], Register_Value::make_literal(0xffffffffffff0007));
    EXPECT_EQ(sm.registers.values[Register_Name::r8],  Register_Value::make_literal(0xffffffffffff0008));
    EXPECT_EQ(sm.registers.values[Register_Name::r9],  Register_Value::make_literal(0xffffffffffff0009));
    EXPECT_EQ(sm.registers.values[Register_Name::r10], Register_Value::make_literal(0xffffffffffff000a));
    EXPECT_EQ(sm.registers.values[Register_Name::r11], Register_Value::make_literal(0xffffffffffff000b));
    EXPECT_EQ(sm.registers.values[Register_Name::r12], Register_Value::make_literal(0xffffffffffff000c));
    EXPECT_EQ(sm.registers.values[Register_Name::r13], Register_Value::make_literal(0xffffffffffff000d));
    EXPECT_EQ(sm.registers.values[Register_Name::r14], Register_Value::make_literal(0xffffffffffff000e));
    EXPECT_EQ(sm.registers.values[Register_Name::r15], Register_Value::make_literal(0xffffffffffff000f));
    // clang-format on
  }
}

TEST(Test_Register, mov8_preserves_other_bits) {
  {
    std::span<const U8> code = ASM_X86_64(
        "mov $0x123456789abcdef0, %rax"
        "mov $0x69, %al");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x123456789abcde69));
  }

  {
    std::span<const U8> code = ASM_X86_64(
        "mov $0x123456789abcdef0, %rax"
        "mov $0x69, %ah");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax],
              Register_Value::make_literal(0x123456789abc69f0));
  }
}

TEST(Test_Register, mov8_after_unknown_value_is_unknown) {
  {
    std::span<const U8> code = ASM_X86_64(
        "mov (%rbx), %rax"
        "mov $0x69, %al");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rax], Register_Value());
  }
}
}
}
