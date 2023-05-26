#include <cppstacksize/asm-stack-map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ElementsAreArray;
using ::testing::IsEmpty;

namespace cppstacksize {
namespace {
TEST(Test_ASM_Stack_Map, function_not_touching_stack) {
  static const U8 code[] = {
      // @asm-begin x86_64
      0x48, 0x89, 0xc3,  // @asm mov %rax, %rbx
      0x90,              // @asm nop
      0x48, 0x01, 0xc0,  // @asm add %rax, %rax
      0xc3,              // @asm ret
  };                     // @asm-end
  Stack_Map sm = analyze_x86_64_stack_map(code);
  EXPECT_THAT(sm.touches, IsEmpty());
}

TEST(Test_ASM_Stack_Map, rsp_relative_store_touches) {
  {
    static const U8 code[] = {
        // @asm-begin x86_64
        0x48, 0x89, 0x44, 0x24, 0x30,  // @asm mov %rax, 0x30(%rsp)
    };                                 // @asm-end
    EXPECT_THAT(analyze_x86_64_stack_map(code).touches,
                ElementsAreArray({
                    Stack_Map_Touch(0, 0x30, 8),
                }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        // clang-format off
        0x48, 0xc7, 0x44, 0x24, 0x30, 0x45, 0x00, 0x00, 0x00,  // @asm movq $69, 0x30(%rsp)
        // clang-format on
    };  // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x30, 8),
                            }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        0x89, 0x44, 0x24, 0x20,  // @asm mov %eax, 0x20(%rsp)
    };                           // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x20, 4),
                            }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        // clang-format off
        0xc7, 0x44, 0x24, 0x20, 0x45, 0x00, 0x00, 0x00,  // @asm movl $69, 0x20(%rsp)
        // clang-format on
    };  // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x20, 4),
                            }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        0x66, 0x89, 0x44, 0x24, 0x20,  // @asm mov %ax, 0x20(%rsp)
    };                                 // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x20, 2),
                            }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        0x66, 0xc7, 0x44, 0x24, 0x20, 0x45, 0x00,  // @asm movw $69, 0x20(%rsp)
    };                                             // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x20, 2),
                            }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        0x88, 0x64, 0x24, 0x20,  // @asm mov %ah, 0x20(%rsp)
    };                           // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x20, 1),
                            }));
  }

  {
    static const U8 code[] = {
        // @asm-begin x86_64
        0xc6, 0x44, 0x24, 0x20, 0x45,  // @asm movb $69, 0x20(%rsp)
    };                                 // @asm-end
    Stack_Map sm = analyze_x86_64_stack_map(code);
    EXPECT_THAT(sm.touches, ElementsAreArray({
                                Stack_Map_Touch(0, 0x20, 1),
                            }));
  }
}
}
}
