#include "entry.h"
#include <limits.h>
static void emit(const char *s) {
  register unsigned r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}
__attribute__((noreturn)) static void stop(unsigned rc) {
  unsigned args[2] = {0x20026, rc};
  register unsigned r0 __asm__("r0") = 0x20;
  register unsigned *r1 __asm__("r1") = args;
  __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
  for (;;) {
  }
}
#define T(x)                                                                   \
  do {                                                                         \
    if (!(x)) {                                                                \
      emit("FE_TARGET_FAIL\n");                                                \
      stop(1);                                                                 \
    }                                                                          \
  } while (0)
static int32_t stage[32];
static unsigned live, releases;
static int32_t *take(void *context, size_t n) {
  (void)context;
  if (live || n > 32)
    return 0;
  live = 1;
  return stage;
}
static void release(void *context, int32_t *p) {
  (void)context;
  T(live && p == stage);
  live = 0;
  ++releases;
}
void fe_fault(void) {
  emit("FE_TARGET_FAULT\n");
  stop(2);
}
void fe_target_main(void) {
  const int32_t in[3] = {3, -1, 4}, bad[3] = {INT32_MAX, 1, -1};
  int32_t out[3] = {91, 92, 93}, value = 91, zeros[16] = {0};
  FeTrace a, b;
  FeAllocator allocator = {0, take, release};
  T(fe_fixed(in, 3, out, 3) == FE_OK && out[0] == 3 && out[1] == 2 &&
    out[2] == 6);
  T(fe_fixed(bad, 3, out, 3) == FE_RANGE && out[0] == 3 && out[1] == 2 &&
    out[2] == 6);
  T(fe_recursive(in, 3, &value, &a) == FE_OK && value == 6 && a.count == 8 &&
    a.maximum_live == 4);
  T(fe_explicit_stack(in, 3, &value, &b) == FE_OK && value == 6 &&
    b.count == 8 && b.maximum_live == 4);
  T(fe_recursive(zeros, 16, &value, &a) == FE_OK && a.count == 34 &&
    a.maximum_live == 17);
  T(fe_explicit_stack(zeros, 16, &value, &b) == FE_OK && b.count == 34 &&
    b.maximum_live == 17);
  T(fe_two_pass(in, 3, out, 3) == FE_OK && out[2] == 6);
  T(fe_staged(in, 3, out, 3, allocator) == FE_OK && out[2] == 6 && !live &&
    releases == 1);
  T(fe_staged(bad, 3, out, 3, allocator) == FE_RANGE && !live && releases == 2);
  T(fe_maximum(in, 3, &value) == FE_OK && value == 4);
  T(fe_maximum(0, 0, &value) == FE_EMPTY && value == 4);
  unsigned char bytes[3];
  T(fe_encode24(0x1234ad, bytes, 3) == FE_OK && bytes[0] == 0x12 &&
    bytes[1] == 0x34 && bytes[2] == 0xad);
  T(fe_any(4, 12) && !fe_all(4, 12) && !fe_forbidden_clear(4, 12));
  T(fe_saturating_add(fe_saturating_add(INT32_MAX, 1), -1) == INT32_MAX - 1);
  emit("FE_TARGET_PASS all_functions explicit_prefix_provider recursion16\n");
  stop(0);
}
