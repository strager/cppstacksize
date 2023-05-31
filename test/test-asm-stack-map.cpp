#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/asm.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <span>

using ::testing::ElementsAreArray;
using ::testing::IsEmpty;

#define CHECK_TOUCHES(code, ...)                                               \
  do {                                                                         \
    EXPECT_THAT(                                                               \
        analyze_x86_64_stack_map((code)).touches,                              \
        ::testing::UnorderedElementsAreArray<Stack_Map_Touch>({__VA_ARGS__})); \
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
              Register_Value::make_entry_rsp_relative(8, 0));
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

  CHECK_TOUCHES(ASM_X86_64("lea 0x50(%rsp), %rax"
                           "lea 0x51(%rsp), %rdx"
                           "call 0x1234"),
                Stack_Map_Touch::read_or_write(0, 0x50, -1),
                Stack_Map_Touch::read_or_write(5, 0x51, -1));
}

TEST(Test_ASM_Stack_Map,
     lea_then_rename_then_call_attributes_stack_usage_to_only_rename) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x50(%rsp), %r15"
                           "mov %r15, %rax"
                           "call 0x1234"),
                Stack_Map_Touch::read_or_write(5, 0x50, -1));
}

TEST(Test_ASM_Stack_Map,
     lea_then_two_calls_attributes_stack_usage_to_lea_once) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x50(%rsp), %rax"
                           "call 0x1234"
                           "call 0x5678"),
                Stack_Map_Touch::read_or_write(0, 0x50, -1));
}

TEST(Test_ASM_Stack_Map, lea_then_store_attributes_stack_usage_to_store) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rax"
                           "mov %rbx, (%rax)"),
                Stack_Map_Touch::write(5, 0x20, 8));
}

TEST(Test_ASM_Stack_Map, lea_then_load_attributes_stack_usage_to_load) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rax"
                           "mov 0x30(%rax), %rbx"),
                Stack_Map_Touch::read(5, 0x20 + 0x30, 8));
}

TEST(Test_ASM_Stack_Map, unrolled_stos) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov $12345, %ecx"
                           "stosq"
                           "stosq"
                           "stosq"),
                Stack_Map_Touch::write(10, 0x20, 8),
                Stack_Map_Touch::write(12, 0x28, 8),
                Stack_Map_Touch::write(14, 0x30, 8));
}

TEST(Test_ASM_Stack_Map, memset_with_rep_stos) {
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov $12345, %ecx"
                           "rep; stosq"),
                Stack_Map_Touch::write(10, 0x20, 12345 * 8));
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov $12345, %rcx"
                           "rep; stosl"),
                Stack_Map_Touch::write(12, 0x20, 12345 * 4));
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov $12345, %ecx"
                           "rep; stosw"),
                Stack_Map_Touch::write(10, 0x20, 12345 * 2));
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov $12345, %ecx"
                           "rep; stosb"),
                Stack_Map_Touch::write(10, 0x20, 12345));

  // Unknown size:
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov (%rax), %ecx"
                           "rep; stosb"),
                Stack_Map_Touch::write(7, 0x20, -1));
}

TEST(Test_ASM_Stack_Map, unrolled_movs) {
  // Destination on stack:
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov 0x40(%rcx), %rsi"
                           "mov $12345, %ecx"
                           "movsq"
                           "movsq"
                           "movsq"),
                Stack_Map_Touch::write(14, 0x20, 8),
                Stack_Map_Touch::write(16, 0x28, 8),
                Stack_Map_Touch::write(18, 0x30, 8));

  // Source on stack:
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rsi"
                           "mov 0x40(%rcx), %rdi"
                           "mov $12345, %ecx"
                           "movsq"
                           "movsq"
                           "movsq"),
                Stack_Map_Touch::read(14, 0x20, 8),
                Stack_Map_Touch::read(16, 0x28, 8),
                Stack_Map_Touch::read(18, 0x30, 8));

  // Source and destination on stack:
  CHECK_TOUCHES(
      ASM_X86_64("lea 0x20(%rsp), %rsi"
                 "lea 0x40(%rsp), %rdi"
                 "mov $12345, %ecx"
                 "movsq"
                 "movsq"
                 "movsq"),
      Stack_Map_Touch::read(15, 0x20, 8), Stack_Map_Touch::write(15, 0x40, 8),
      Stack_Map_Touch::read(17, 0x28, 8), Stack_Map_Touch::write(17, 0x48, 8),
      Stack_Map_Touch::read(19, 0x30, 8), Stack_Map_Touch::write(19, 0x50, 8));
}

TEST(Test_ASM_Stack_Map, memcpy_with_rep_movs) {
  // Destination on stack:
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rdi"
                           "mov 0x40(%rcx), %rsi"
                           "mov $12345, %ecx"
                           "rep; movsq"),
                Stack_Map_Touch::write(14, 0x20, 12345 * 8));

  // Source on stack:
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rsi"
                           "mov 0x40(%rcx), %rdi"
                           "mov $12345, %ecx"
                           "rep; movsq"),
                Stack_Map_Touch::read(14, 0x20, 12345 * 8));

  // Source and destination on stack:
  CHECK_TOUCHES(ASM_X86_64("lea 0x20(%rsp), %rsi"
                           "lea 0x40(%rsp), %rdi"
                           "mov $12345, %ecx"
                           "rep; movsl"),
                Stack_Map_Touch::read(15, 0x20, 12345 * 4),
                Stack_Map_Touch::write(15, 0x40, 12345 * 4));
}
}
}
