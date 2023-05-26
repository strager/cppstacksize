#include <cstddef>

void f() {
  int *p_i = nullptr;
  const int *p_ci = nullptr;
  volatile int *p_vi = nullptr;
  const volatile int *p_cvi = nullptr;

  void *p_v = nullptr;
  const void *p_cv = nullptr;

  int **p_p_i = nullptr;
  int *const *p_cp_i = nullptr;
  const int *const *p_cp_ci = nullptr;
  const int **p_p_ci = nullptr;

  int ***p_p_p_i = nullptr;

  std::nullptr_t np = nullptr;
}
