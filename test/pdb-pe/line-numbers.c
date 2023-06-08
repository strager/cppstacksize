void callee() {}

void foo() {
  callee();
  callee();
#include "line-numbers.inc"
  callee();
}

// clang-format off
void bar() {
  callee(); callee();
}
// clang-format on

void _start() {}
