#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/asm.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <span>

using ::testing::ElementsAreArray;
using ::testing::IsEmpty;

#define CHECK_TOUCHES(code, ...)                                              \
  do {                                                                        \
    EXPECT_THAT(analyze_x86_64_stack_map((code)).touches,                     \
                ::testing::ElementsAreArray<Stack_Map_Touch>({__VA_ARGS__})); \
  } while (false)

namespace cppstacksize {
namespace {
TEST(Test_ASM_Stack_Map, function_not_touching_stack) {
  std::span<const U8> code = ASM_X86_64(
      "mov %rax, %rbx"
      "nop"
      "add %rax, %rax");
  Stack_Map sm = analyze_x86_64_stack_map(code);
  EXPECT_THAT(sm.touches, IsEmpty());
}

TEST(Test_ASM_Stack_Map, rsp_relative_store_touches) {
  CHECK_TOUCHES(ASM_X86_64("mov %rax, 0x30(%rsp)"),
                Stack_Map_Touch::write(0, 0x30, 8));
  CHECK_TOUCHES(ASM_X86_64("movq $69, 0x30(%rsp)"),
                Stack_Map_Touch::write(0, 0x30, 8));

  CHECK_TOUCHES(ASM_X86_64("mov %eax, 0x20(%rsp)"),
                Stack_Map_Touch::write(0, 0x20, 4));
  CHECK_TOUCHES(ASM_X86_64("movl $69, 0x20(%rsp)"),
                Stack_Map_Touch::write(0, 0x20, 4));

  CHECK_TOUCHES(ASM_X86_64("mov %ax, 0x20(%rsp)"),
                Stack_Map_Touch::write(0, 0x20, 2));
  CHECK_TOUCHES(ASM_X86_64("movw $69, 0x20(%rsp)"),
                Stack_Map_Touch::write(0, 0x20, 2));

  CHECK_TOUCHES(ASM_X86_64("mov %ah, 0x20(%rsp)"),
                Stack_Map_Touch::write(0, 0x20, 1));
  CHECK_TOUCHES(ASM_X86_64("movb $69, 0x20(%rsp)"),
                Stack_Map_Touch::write(0, 0x20, 1));
}

TEST(Test_ASM_Stack_Map, rsp_relative_load_touches) {
  CHECK_TOUCHES(ASM_X86_64("mov 0x30(%rsp), %rax"),
                Stack_Map_Touch::read(0, 0x30, 8));
  CHECK_TOUCHES(ASM_X86_64("mov 0x20(%rsp), %eax"),
                Stack_Map_Touch::read(0, 0x20, 4));
  CHECK_TOUCHES(ASM_X86_64("mov 0x20(%rsp), %ax"),
                Stack_Map_Touch::read(0, 0x20, 2));
  CHECK_TOUCHES(ASM_X86_64("mov 0x20(%rsp), %ah"),
                Stack_Map_Touch::read(0, 0x20, 1));

  CHECK_TOUCHES(ASM_X86_64("movzbl 0x20(%rsp), %eax"),
                Stack_Map_Touch::read(0, 0x20, 1));
  CHECK_TOUCHES(ASM_X86_64("movzbq 0x20(%rsp), %rax"),
                Stack_Map_Touch::read(0, 0x20, 1));
  CHECK_TOUCHES(ASM_X86_64("movzwq 0x20(%rsp), %rax"),
                Stack_Map_Touch::read(0, 0x20, 2));

  CHECK_TOUCHES(ASM_X86_64("movslq 0x20(%rsp), %rax"),
                Stack_Map_Touch::read(0, 0x20, 4));
  CHECK_TOUCHES(ASM_X86_64("movsbq 0x20(%rsp), %rbx"),
                Stack_Map_Touch::read(0, 0x20, 1));
  CHECK_TOUCHES(ASM_X86_64("movswq 0x20(%rsp), %rbx"),
                Stack_Map_Touch::read(0, 0x20, 2));

  CHECK_TOUCHES(ASM_X86_64("cmp 0x20(%rsp), %rbx"),
                Stack_Map_Touch::read(0, 0x20, 8));
  CHECK_TOUCHES(ASM_X86_64("cmp 0x20(%rsp), %bh"),
                Stack_Map_Touch::read(0, 0x20, 1));
}

TEST(Test_ASM_Stack_Map, rsp_relative_load_modify_store_touches) {
  CHECK_TOUCHES(ASM_X86_64("add %rax, 0x28(%rsp)"),
                Stack_Map_Touch::read_and_write(0, 0x28, 8));
  CHECK_TOUCHES(ASM_X86_64("addq $69, 0x28(%rsp)"),
                Stack_Map_Touch::read_and_write(0, 0x28, 8));
}

TEST(Test_ASM_Stack_Map, offset_is_byte_of_start_of_instruction) {
  CHECK_TOUCHES(ASM_X86_64("nop"
                           "mov (%rsp), %rax"),
                Stack_Map_Touch::read(1, 0, 8));

  CHECK_TOUCHES(ASM_X86_64("test %rax, %rax"
                           "mov (%rsp), %rax"),
                Stack_Map_Touch::read(3, 0, 8));
}

TEST(Test_ASM_Stack_Map, push_touches_stack) {
  CHECK_TOUCHES(ASM_X86_64("pushq %rax"), Stack_Map_Touch::write(0, -8, 8));
  CHECK_TOUCHES(ASM_X86_64("pushw %bx"), Stack_Map_Touch::write(0, -2, 2));
  CHECK_TOUCHES(ASM_X86_64("pushw (%rbp)"), Stack_Map_Touch::write(0, -2, 2));
}

TEST(Test_ASM_Stack_Map, push_updates_rsp) {
  CHECK_TOUCHES(ASM_X86_64("pushq %rax"
                           "movq $69, (%rsp)"),
                Stack_Map_Touch::write(0, -8, 8),
                Stack_Map_Touch::write(1, -8, 8));
}

TEST(Test_ASM_Stack_Map, pop_updates_rsp) {
  CHECK_TOUCHES(ASM_X86_64("pop %rax"
                           "movq $69, (%rsp)"),
                Stack_Map_Touch::read(0, 0, 8),
                Stack_Map_Touch::write(1, 8, 8));

  CHECK_TOUCHES(ASM_X86_64("pop %di"
                           "movq $69, (%rsp)"),
                Stack_Map_Touch::read(0, 0, 2),
                Stack_Map_Touch::write(2, 2, 8));
}

TEST(Test_ASM_Stack_Map, rsp_relative_mov_after_stack_adjustment) {
  CHECK_TOUCHES(ASM_X86_64("sub $0x50, %rsp"
                           "mov %rax, 0x30(%rsp)"),
                Stack_Map_Touch::write(4, -0x50 + 0x30, 8));

  CHECK_TOUCHES(ASM_X86_64("add $0x50, %rsp"
                           "mov %rax, 0x30(%rsp)"),
                Stack_Map_Touch::write(4, +0x50 + 0x30, 8));
}

TEST(Test_ASM_Stack_Map, push_after_stack_adjustment) {
  CHECK_TOUCHES(ASM_X86_64("sub $0x50, %rsp"
                           "pushq (%rdi)"),
                Stack_Map_Touch::write(4, -0x50 - 8, 8));
}

TEST(Test_ASM_Stack_Map,
     mov_immediate_to_register_then_adjust_rsp_by_register) {
  CHECK_TOUCHES(ASM_X86_64("mov $0x50, %rax"
                           "sub %rax, %rsp"
                           "mov %rbx, (%rsp)"),
                Stack_Map_Touch::write(10, -0x50, 8));
}

TEST(Test_ASM_Stack_Map, ret_reads_return_address_and_adjusts_stack) {
  {
    std::span<const U8> code = ASM_X86_64("ret");
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_EQ(sm.registers.values[Register_Name::rsp],
              Register_Value::make_entry_rsp_relative(8));
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch::read(0, 0, 8),
                            }));
  }
}

TEST(Test_ASM_Stack_Map, lea_with_rsp_alone_does_not_use_stack) {
  CHECK_TOUCHES(
      ASM_X86_64("lea 0x50(%rsp), %rax"
                 "lea (%rax), %rbx"));
}

TEST(Test_ASM_Stack_Map, lea_then_call_attributes_stack_usage_to_lea) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x50(%rsp), %rax"
                           "call 0x1234"),
                Stack_Map_Touch::read_or_write(0, 0x50, -1));
}
}
}
