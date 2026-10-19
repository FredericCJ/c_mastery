#include "semantics.h"
int fs_wrong(void) {
  const int a[3] = {1, 2, 3};
  int out;
  return fs_row(a, &out);
}
