#include "sorting.h"
#include <math.h>
int lr_compare_i32(const void *a, const void *b) {
  int32_t x = *(const int32_t *)a, y = *(const int32_t *)b;
  return (x > y) - (x < y);
}
int lr_compare_record(const void *a, const void *b) {
  const struct lr_record *x = a, *y = b;
#ifndef LR_MUTANT_NAN
  int xn = !!isnan(x->key), yn = !!isnan(y->key);
  if (xn != yn)
    return xn - yn;
#endif
  int key = (x->key > y->key) - (x->key < y->key);
  return key ? key : (x->identity > y->identity) - (x->identity < y->identity);
}
enum lr_status lr_sort(int32_t *a, size_t n, struct f_work *w) {
  if (!w || (n && !a) || n > 32)
    return LR_ARGUMENT;
  struct f_record staged[32];
  struct f_work work;
  for (size_t i = 0; i < n; ++i)
    staged[i] = (struct f_record){a[i], (uint32_t)i};
  if (f_sort(staged, n, &work) != F_OK)
    return LR_SOURCE;
  for (size_t i = 0; i < n; ++i)
    a[i] = staged[i].key;
  *w = work;
  return LR_OK;
}
enum lr_status lr_three(lr_draw f, void *ctx, unsigned budget, unsigned *out) {
  if (!f || !out)
    return LR_ARGUMENT;
  while (budget--) {
    int x = f(ctx);
    if (x < 0 || x > 7)
      return LR_SOURCE;
#ifndef LR_MUTANT_BIAS
    if (x >= 6)
      continue;
#endif
    *out = (unsigned)x % 3;
    return LR_OK;
  }
  return LR_RETRY;
}
enum lr_status lr_entropy(lr_bytes f, void *ctx, unsigned budget,
                          unsigned char out[32]) {
  if (!f || !out)
    return LR_ARGUMENT;
  unsigned char staged[32];
  size_t done = 0;
  while (done < sizeof staged && budget--) {
    ptrdiff_t n = f(ctx, staged + done, sizeof staged - done);
    if (n == -2)
      continue;
    if (n <= 0 || (size_t)n > sizeof staged - done)
      return LR_SOURCE;
    done += (size_t)n;
#ifdef LR_MUTANT_PARTIAL
    for (size_t i = 0; i < done; ++i)
      out[i] = staged[i];
#endif
  }
  if (done != sizeof staged)
    return LR_RETRY;
  for (size_t i = 0; i < done; ++i)
    out[i] = staged[i];
  return LR_OK;
}
