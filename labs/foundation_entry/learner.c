#include "entry.h"
/* Intentionally unfinished learner implementation; it must compile and fail. */
FeStatus fe_fixed(const int32_t *p, size_t n, int32_t *o, size_t c) {
  (void)p;
  (void)n;
  (void)o;
  (void)c;
  return FE_NOT_IMPLEMENTED;
}
FeStatus fe_recursive(const int32_t *p, size_t n, int32_t *o, FeTrace *t) {
  (void)p;
  (void)n;
  (void)o;
  (void)t;
  return FE_NOT_IMPLEMENTED;
}
FeStatus fe_explicit_stack(const int32_t *p, size_t n, int32_t *o, FeTrace *t) {
  (void)p;
  (void)n;
  (void)o;
  (void)t;
  return FE_NOT_IMPLEMENTED;
}
FeStatus fe_two_pass(const int32_t *p, size_t n, int32_t *o, size_t c) {
  (void)p;
  (void)n;
  (void)o;
  (void)c;
  return FE_NOT_IMPLEMENTED;
}
FeStatus fe_staged(const int32_t *p, size_t n, int32_t *o, size_t c,
                   FeAllocator a) {
  (void)p;
  (void)n;
  (void)o;
  (void)c;
  (void)a;
  return FE_NOT_IMPLEMENTED;
}
FeStatus fe_maximum(const int32_t *p, size_t n, int32_t *o) {
  (void)p;
  (void)n;
  (void)o;
  return FE_NOT_IMPLEMENTED;
}
FeStatus fe_encode24(uint32_t v, unsigned char *o, size_t c) {
  (void)v;
  (void)o;
  (void)c;
  return FE_NOT_IMPLEMENTED;
}
int fe_any(uint8_t b, uint8_t m) {
  (void)b;
  (void)m;
  return -1;
}
int fe_all(uint8_t b, uint8_t m) {
  (void)b;
  (void)m;
  return -1;
}
int fe_forbidden_clear(uint8_t b, uint8_t m) {
  (void)b;
  (void)m;
  return -1;
}
int32_t fe_saturating_add(int32_t a, int32_t b) {
  (void)a;
  (void)b;
  return 0;
}
