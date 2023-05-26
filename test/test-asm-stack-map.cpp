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
}
}
