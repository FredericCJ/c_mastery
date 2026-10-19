#include "semantics.h"
int fs_wrong(void) {
  int a[3] = {1, 2, 3};
  int *rows[2] = {a, a};
  int out;
  return fs_matrix(rows, 2, &out);
}
