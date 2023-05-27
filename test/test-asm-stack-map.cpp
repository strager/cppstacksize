#include <cppstacksize/asm-stack-map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::IsEmpty;

#define CHECK_TOUCHES(code, ...)                               \
  do {                                                         \
    static constexpr U8 _code_bytes[] = code;                  \
    EXPECT_THAT(analyze_x86_64_stack_map(_code_bytes).touches, \
                ::testing::ElementsAreArray(__VA_ARGS__));     \
  } while (false)

#define ASM_X86_64(...) \
  { __VA_ARGS__ }

namespace cppstacksize {
namespace {
TEST(Test_ASM_Stack_Map, function_not_touching_stack) {
  // clang-format off
  static const U8 code[] = ASM_X86_64(
      // mov %rax, %rbx
      // nop
      // add %rax, %rax
      // ret
      0x48, 0x89, 0xc3,
      0x90,
      0x48, 0x01, 0xc0,
      0xc3,
  );
  // clang-format on
  Stack_Map sm = analyze_x86_64_stack_map(code);
  EXPECT_THAT(sm.touches, IsEmpty());
}

TEST(Test_ASM_Stack_Map, rsp_relative_store_touches) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // mov %rax, 0x30(%rsp)
    0x48, 0x89, 0x44, 0x24, 0x30,
  ), {
    Stack_Map_Touch::write(0, 0x30, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // movq $69, 0x30(%rsp)
    0x48, 0xc7, 0x44, 0x24, 0x30, 0x45, 0x00, 0x00, 0x00,
  ), {
    Stack_Map_Touch::write(0, 0x30, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // mov %eax, 0x20(%rsp)
    0x89, 0x44, 0x24, 0x20,
  ), {
    Stack_Map_Touch::write(0, 0x20, 4),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // movl $69, 0x20(%rsp)
    0xc7, 0x44, 0x24, 0x20, 0x45, 0x00, 0x00, 0x00,
  ), {
    Stack_Map_Touch::write(0, 0x20, 4),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // mov %ax, 0x20(%rsp)
    0x66, 0x89, 0x44, 0x24, 0x20,
  ), {
    Stack_Map_Touch::write(0, 0x20, 2),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // movw $69, 0x20(%rsp)
    0x66, 0xc7, 0x44, 0x24, 0x20, 0x45, 0x00,
  ), {
    Stack_Map_Touch::write(0, 0x20, 2),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // mov %ah, 0x20(%rsp)
    0x88, 0x64, 0x24, 0x20,
  ), {
    Stack_Map_Touch::write(0, 0x20, 1),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // movb $69, 0x20(%rsp)
    0xc6, 0x44, 0x24, 0x20, 0x45,
  ), {
    Stack_Map_Touch::write(0, 0x20, 1),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, rsp_relative_load_touches) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // mov 0x30(%rsp), %rax
    0x48, 0x8b, 0x44, 0x24, 0x30,
  ), {
    Stack_Map_Touch::read(0, 0x30, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // mov 0x20(%rsp), %eax
    0x8b, 0x44, 0x24, 0x20,
  ), {
    Stack_Map_Touch::read(0, 0x20, 4),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // mov 0x20(%rsp), %ax
    0x66, 0x8b, 0x44, 0x24, 0x20,
  ), {
    Stack_Map_Touch::read(0, 0x20, 2),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // mov 0x20(%rsp), %ah
    0x8a, 0x64, 0x24, 0x20,
  ), {
    Stack_Map_Touch::read(0, 0x20, 1),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, offset_is_byte_of_start_of_instruction) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // nop
    // mov (%rsp), %rax
    0x90,
    0x48, 0x8b, 0x04, 0x24,
  ), {
    Stack_Map_Touch::read(1, 0, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // test %rax, %rax
    // mov (%rsp), %rax
    0x48, 0x85, 0xc0,
    0x48, 0x8b, 0x04, 0x24,
  ), {
    Stack_Map_Touch::read(3, 0, 8),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, push_touches_stack) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // pushq %rax
    0x50,
  ), {
    Stack_Map_Touch::write(0, -8, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // pushw %bx
    0x66, 0x53,
  ), {
    Stack_Map_Touch::write(0, -2, 2),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // pushw (%rbp)
    0x66, 0xff, 0x75, 0x00,
  ), {
    Stack_Map_Touch::write(0, -2, 2),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, push_updates_rsp) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // pushq %rax
    // movq $69, (%rsp)
    0x50,
    0x48, 0xc7, 0x04, 0x24, 0x45, 0x00, 0x00, 0x00,
  ), {
    Stack_Map_Touch::write(0, -8, 8),
    Stack_Map_Touch::write(1, -8, 8),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, pop_updates_rsp) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // pop %rax
    // movq $69, (%rsp)
    0x58,
    0x48, 0xc7, 0x04, 0x24, 0x45, 0x00, 0x00, 0x00,
  ), {
    Stack_Map_Touch::write(1, 8, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // pop %di
    // movq $69, (%rsp)
    0x66, 0x5f,
    0x48, 0xc7, 0x04, 0x24, 0x45, 0x00, 0x00, 0x00,
  ), {
    Stack_Map_Touch::write(2, 2, 8),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, rsp_relative_mov_after_stack_adjustment) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // sub $0x50, %rsp
    // mov %rax, 0x30(%rsp)
    0x48, 0x83, 0xec, 0x50,
    0x48, 0x89, 0x44, 0x24, 0x30,
  ), {
    Stack_Map_Touch::write(4, -0x50 + 0x30, 8),
  });

  CHECK_TOUCHES(ASM_X86_64(
    // add $0x50, %rsp
    // mov %rax, 0x30(%rsp)
    0x48, 0x83, 0xc4, 0x50,
    0x48, 0x89, 0x44, 0x24, 0x30,
  ), {
    Stack_Map_Touch::write(4, +0x50 + 0x30, 8),
  });

  // clang-format on
}

TEST(Test_ASM_Stack_Map, push_after_stack_adjustment) {
  // clang-format off

  CHECK_TOUCHES(ASM_X86_64(
    // sub $0x50, %rsp
    // pushq (%rdi)
    0x48, 0x83, 0xec, 0x50,
    0xff, 0x37,
  ), {
    Stack_Map_Touch::write(4, -0x50 - 8, 8),
  });

  // clang-format on
}
}
}
