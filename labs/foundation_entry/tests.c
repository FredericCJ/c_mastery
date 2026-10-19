#include "entry.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "FE_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x);          \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
typedef struct {
  int32_t storage[64];
  unsigned fail, live, takes, releases;
} Pool;
static int32_t *take(void *p, size_t n) {
  Pool *a = p;
  ++a->takes;
  if (a->fail || a->live || n > 64)
    return NULL;
  a->live = 1;
  return a->storage;
}
static void release(void *p, int32_t *v) {
  Pool *a = p;
  CHECK(v == a->storage && a->live);
  a->live = 0;
  ++a->releases;
}
static FeAllocator allocator(Pool *p) {
  return (FeAllocator){p, take, release};
}
static void trace(void) {
  const int32_t in[] = {3, -1, 4};
  int32_t a = 91, b = 91;
  FeTrace x, y;
  CHECK(fe_recursive(in, 3, &a, &x) == FE_OK && a == 6);
  CHECK(fe_explicit_stack(in, 3, &b, &y) == FE_OK && b == 6);
  CHECK(x.count == 8 && y.count == 8 && x.maximum_live == 4 &&
        y.maximum_live == 4);
  const unsigned enters[] = {1, 1, 1, 1, 0, 0, 0, 0};
  const size_t indices[] = {0, 1, 2, 3, 3, 2, 1, 0};
  const int32_t values[] = {0, 0, 0, 0, 0, 4, 3, 6};
  for (size_t i = 0; i < 8; ++i) {
    CHECK(x.event[i].entering == enters[i] && x.event[i].index == indices[i] &&
          x.event[i].value == values[i]);
    CHECK(y.event[i].entering == enters[i] && y.event[i].index == indices[i] &&
          y.event[i].value == values[i]);
  }
  CHECK(fe_recursive(NULL, 0, &a, &x) == FE_OK && a == 0 && x.count == 2 &&
        x.maximum_live == 1);
  CHECK(fe_explicit_stack(NULL, 0, &a, &y) == FE_OK && a == 0 && y.count == 2 &&
        y.maximum_live == 1);
  int32_t full[17] = {0};
  CHECK(fe_recursive(full, 16, &a, &x) == FE_OK && x.count == 34 &&
        x.maximum_live == 17);
  CHECK(fe_explicit_stack(full, 16, &a, &y) == FE_OK && y.count == 34 &&
        y.maximum_live == 17);
  a = 91;
  CHECK(fe_recursive(full, 17, &a, &x) == FE_FULL && a == 91);
  CHECK(fe_explicit_stack(full, 17, &a, &y) == FE_FULL && a == 91);
  puts("FE_TRACE_PASS normal8 empty2 bounded34 "
       "explicit_activation_correspondence");
}
static void prefixes(void) {
  const int32_t bad[] = {INT32_MAX, 1, -1};
  int32_t out[] = {91, 92, 93};
  Pool pool = {0};
  FeAllocator a = allocator(&pool);
  CHECK(fe_fixed(bad, 3, out, 3) == FE_RANGE && out[0] == 91 && out[1] == 92 &&
        out[2] == 93);
  CHECK(fe_two_pass(bad, 3, out, 3) == FE_RANGE && out[0] == 91 &&
        out[1] == 92 && out[2] == 93);
  CHECK(fe_staged(bad, 3, out, 3, a) == FE_RANGE && out[0] == 91 &&
        out[1] == 92 && out[2] == 93);
  CHECK(pool.live == 0 && pool.releases == 1);
  const int32_t good[] = {3, -1, 4};
  CHECK(fe_fixed(good, 3, out, 3) == FE_OK && out[0] == 3 && out[1] == 2 &&
        out[2] == 6);
  CHECK(fe_fixed(NULL, 0, NULL, 0) == FE_OK);
  CHECK(fe_two_pass(NULL, 0, NULL, 0) == FE_OK);
  CHECK(fe_staged(NULL, 0, NULL, 0, (FeAllocator){0}) == FE_OK);
  pool.fail = 1;
  out[0] = 91;
  CHECK(fe_staged(good, 3, out, 3, a) == FE_ALLOC && out[0] == 91 &&
        pool.live == 0);
  pool.fail = 0;
  int32_t inplace[] = {3, -1, 4};
  CHECK(fe_two_pass(inplace, 3, inplace, 3) == FE_OK && inplace[0] == 3 &&
        inplace[1] == 2 && inplace[2] == 6);
  int32_t overlap[] = {3, -1, 4, 99};
  CHECK(fe_staged(overlap, 3, overlap + 1, 3, a) == FE_OK && overlap[0] == 3 &&
        overlap[1] == 3 && overlap[2] == 2 && overlap[3] == 6 &&
        pool.live == 0);
  int32_t seventeen[17], result[17];
  for (size_t i = 0; i < 17; ++i) {
    seventeen[i] = 1;
    result[i] = 91;
  }
  CHECK(fe_fixed(seventeen, 17, result, 17) == FE_FULL && result[0] == 91);
  CHECK(fe_two_pass(seventeen, 17, result, 17) == FE_OK && result[16] == 17);
  CHECK(fe_staged(seventeen, 17, result, 17, a) == FE_OK && result[16] == 17);
  unsigned before = pool.takes;
  CHECK(fe_staged(good, SIZE_MAX, out, SIZE_MAX, a) == FE_RANGE &&
        pool.takes == before);
  CHECK(fe_maximum(NULL, 0, &out[0]) == FE_EMPTY && out[0] == 91);
  CHECK(fe_maximum(good, 3, &out[0]) == FE_OK && out[0] == 4);
  const int32_t negative[] = {-5, -2, -9};
  CHECK(fe_maximum(negative, 3, &out[0]) == FE_OK && out[0] == -2);
  const int32_t minimum[] = {INT32_MIN};
  CHECK(fe_maximum(minimum, 1, &out[0]) == FE_OK && out[0] == INT32_MIN);
  CHECK(fe_maximum(NULL, 0, NULL) == FE_INVALID);
  int32_t small[2] = {81, 82};
  CHECK(fe_fixed(good, 3, small, 2) == FE_FULL && small[0] == 81 &&
        small[1] == 82);
  CHECK(fe_two_pass(good, 3, small, 2) == FE_FULL && small[0] == 81);
  CHECK(fe_staged(good, 3, small, 2, a) == FE_FULL && small[0] == 81);
  before = pool.takes;
  CHECK(fe_staged(NULL, 0, NULL, 0, a) == FE_OK && pool.takes == before);
  puts(
      "FE_PREFIX_PASS atomic_failure dynamic_rollback overlap17 maximum_empty");
}
static void values(void) {
  unsigned char bytes[] = {91, 92, 93, 94};
  CHECK(fe_encode24(UINT32_C(0x1234ad), bytes, 4) == FE_OK &&
        bytes[0] == 0x12 && bytes[1] == 0x34 && bytes[2] == 173 &&
        bytes[3] == 94);
  CHECK(fe_encode24(UINT32_C(0x1000000), bytes, 4) == FE_RANGE &&
        bytes[0] == 0x12);
  CHECK(fe_encode24(173, bytes, 2) == FE_FULL && bytes[0] == 0x12);
  CHECK(fe_encode24(0xffffff, bytes, 3) == FE_OK && bytes[0] == 255 &&
        bytes[1] == 255 && bytes[2] == 255);
  CHECK(fe_encode24(0, bytes, 3) == FE_OK && bytes[0] == 0 && bytes[1] == 0 &&
        bytes[2] == 0);
  for (unsigned b = 0; b < 256; ++b)
    for (unsigned m = 0; m < 256; ++m) {
      unsigned selected = 0, present = 0;
      for (unsigned i = 0; i < 8; ++i)
        if ((m / (1u << i)) % 2) {
          ++selected;
          present += (b / (1u << i)) % 2;
        }
      CHECK(fe_any((uint8_t)b, (uint8_t)m) == (present != 0));
      CHECK(fe_all((uint8_t)b, (uint8_t)m) == (present == selected));
      CHECK(fe_forbidden_clear((uint8_t)b, (uint8_t)m) == (present == 0));
    }
  CHECK(fe_saturating_add(fe_saturating_add(INT32_MAX, 1), -1) ==
        INT32_MAX - 1);
  CHECK(fe_saturating_add(INT32_MAX, fe_saturating_add(1, -1)) == INT32_MAX);
  CHECK(fe_saturating_add(INT32_MIN, -1) == INT32_MIN);
  CHECK(fe_saturating_add(-7, 4) == -3);
  uint32_t wrapped = UINT32_MAX;
  wrapped += 1;
  CHECK(wrapped == 0);
  puts("FE_VALUE_PASS BE24 masks65536 saturation_nonassociative "
       "unsigned_modulo");
}
static void oracle(void) {
  const int32_t choices[] = {INT32_MIN, -2, -1, 0, 1, 2, INT32_MAX};
  Pool pool = {0};
  int32_t out[3], other[3];
  unsigned cases = 0;
  for (unsigned a = 0; a < 7; ++a)
    for (unsigned b = 0; b < 7; ++b)
      for (unsigned c = 0; c < 7; ++c) {
        int32_t in[] = {choices[a], choices[b], choices[c]};
        int64_t sums[3] = {(int64_t)in[0], (int64_t)in[0] + in[1],
                           (int64_t)in[0] + in[1] + in[2]};
        int valid = 1;
        for (unsigned i = 0; i < 3; ++i)
          if (sums[i] < INT32_MIN || sums[i] > INT32_MAX)
            valid = 0;
        for (unsigned i = 0; i < 3; ++i)
          out[i] = other[i] = 91;
        CHECK(fe_two_pass(in, 3, out, 3) == (valid ? FE_OK : FE_RANGE));
        CHECK(fe_staged(in, 3, other, 3, allocator(&pool)) ==
              (valid ? FE_OK : FE_RANGE));
        CHECK(pool.live == 0);
        for (unsigned i = 0; i < 3; ++i)
          CHECK(out[i] == (valid ? (int32_t)sums[i] : 91) &&
                other[i] == out[i]);
        int64_t suffix[3] = {(int64_t)in[2], (int64_t)in[1] + in[2], sums[2]};
        valid = 1;
        for (unsigned i = 0; i < 3; ++i)
          if (suffix[i] < INT32_MIN || suffix[i] > INT32_MAX)
            valid = 0;
        FeTrace x, y;
        int32_t r = 91, s = 91;
        CHECK(fe_recursive(in, 3, &r, &x) == (valid ? FE_OK : FE_RANGE));
        CHECK(fe_explicit_stack(in, 3, &s, &y) == (valid ? FE_OK : FE_RANGE));
        CHECK(r == (valid ? (int32_t)sums[2] : 91) && s == r &&
              x.count == y.count);
        for (size_t i = 0; i < x.count; ++i)
          CHECK(x.event[i].index == y.event[i].index &&
                x.event[i].entering == y.event[i].entering &&
                x.event[i].value == y.event[i].value);
        ++cases;
      }
  printf("FE_ORACLE_PASS direct_integer_formula_cases=%u\n", cases);
}
int main(void) {
  trace();
  prefixes();
  values();
  oracle();
  puts("FE_PASS entry_algorithms_values");
  return 0;
}
