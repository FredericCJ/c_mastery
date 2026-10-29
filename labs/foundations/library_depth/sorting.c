#include "sorting.h"
int lr_compare_i32(const void *a, const void *b) {
  (void)a;
  (void)b;
  return 0;
}
int lr_compare_record(const void *a, const void *b) {
  (void)a;
  (void)b;
  return 0;
}
enum lr_status lr_sort(int32_t *a, size_t n, struct f_work *w) {
  (void)a;
  (void)n;
  (void)w;
  return LR_NOT_IMPLEMENTED;
}
enum lr_status lr_three(lr_draw f, void *ctx, unsigned n, unsigned *out) {
  (void)f;
  (void)ctx;
  (void)n;
  (void)out;
  return LR_NOT_IMPLEMENTED;
}
enum lr_status lr_entropy(lr_bytes f, void *ctx, unsigned n,
                          unsigned char out[32]) {
  (void)f;
  (void)ctx;
  (void)n;
  (void)out;
  return LR_NOT_IMPLEMENTED;
}
