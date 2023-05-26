#include <malloc.h>

int *f() {
  int *x = (int *)_alloca(69);
  return x;
}
