#include "machine.h"
void *mi_move(void *d, const void *s, size_t n) {
  unsigned char *a = d;
  const unsigned char *b = s;
  if ((uintptr_t)a < (uintptr_t)b) {
    for (size_t i = 0; i < n; i++)
      a[i] = b[i];
  } else {
    while (n) {
      --n;
      a[n] = b[n];
    }
  }
  return d;
}
int mi_compare(const void *a, const void *b, size_t n) {
  const unsigned char *x = a, *y = b;
  for (size_t i = 0; i < n; i++) {
    if (x[i] != y[i])
      return x[i] < y[i] ? -1 : 1;
  }
  return 0;
}
