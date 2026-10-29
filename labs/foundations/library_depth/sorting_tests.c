#include "sorting.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SORTING-CHECK:%s line%d\n", #x, __LINE__);              \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int one(void *ctx) { return *(int *)ctx; }
struct bytes {
  unsigned calls, position, fail;
};
static ptrdiff_t source(void *ctx, unsigned char *out, size_t n) {
  struct bytes *s = ctx;
  ++s->calls;
  if (s->calls == 1)
    return -2;
  if (s->position >= s->fail)
    return -1;
  if (n > 3)
    n = 3;
  if (n > s->fail - s->position)
    n = s->fail - s->position;
  for (size_t i = 0; i < n; ++i)
    out[i] = (unsigned char)(s->position + i);
  s->position += (unsigned)n;
  return (ptrdiff_t)n;
}
int main(void) {
  const int32_t alphabet[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
  unsigned cases = 0;
  for (unsigned a = 0; a < 5; ++a)
    for (unsigned b = 0; b < 5; ++b)
      for (unsigned c = 0; c < 5; ++c) {
        int ab = lr_compare_i32(alphabet + a, alphabet + b),
            ba = lr_compare_i32(alphabet + b, alphabet + a),
            bc = lr_compare_i32(alphabet + b, alphabet + c),
            ac = lr_compare_i32(alphabet + a, alphabet + c);
        CHECK(ab == -ba && (!ab) == (a == b));
        CHECK(!(ab <= 0 && bc <= 0) || ac <= 0);
      }
  for (unsigned n = 0, limit = 1; n <= 5; ++n, limit *= 5)
    for (unsigned code = 0; code < limit; ++code) {
      int32_t a[32] = {0}, b[32] = {0};
      unsigned counts[5] = {0}, q = code;
      struct f_work work;
      for (unsigned i = 0; i < n; ++i) {
        unsigned digit = q % 5;
        q /= 5;
        a[i] = b[i] = alphabet[digit];
        ++counts[digit];
      }
      CHECK(lr_sort(a, n, &work) == LR_OK);
      qsort(b, n, sizeof *b, lr_compare_i32);
      unsigned offset = 0;
      for (unsigned k = 0; k < 5; ++k)
        for (unsigned i = 0; i < counts[k]; ++i, ++offset)
          CHECK(a[offset] == alphabet[k] && b[offset] == alphabet[k]);
      CHECK(work.comparisons <= n * (n ? n - 1 : 0) / 2);
      ++cases;
    }
  CHECK(cases == 3906);
  int32_t sample[] = {INT32_MIN, INT32_MAX, 0, 0, 7}, zero = 0, eight = 8;
  qsort(sample, 5, sizeof *sample, lr_compare_i32);
  CHECK(sample[0] == INT32_MIN && sample[1] == 0 && sample[2] == 0 &&
        sample[3] == 7 && sample[4] == INT32_MAX);
  CHECK(bsearch(&zero, sample, 5, sizeof *sample, lr_compare_i32) != NULL &&
        bsearch(&eight, sample, 5, sizeof *sample, lr_compare_i32) == NULL);
  CHECK(bsearch(&zero, sample, 0, sizeof *sample, lr_compare_i32) == NULL);
  struct lr_record records[] = {{NAN, 0},      {0, 4},         {-0.0, 3},
                                {INFINITY, 8}, {-INFINITY, 9}, {NAN, 2},
                                {1, 5},        {-1, 6}};
  for (unsigned a = 0; a < 8; ++a)
    for (unsigned b = 0; b < 8; ++b)
      for (unsigned c = 0; c < 8; ++c) {
        int ab = lr_compare_record(records + a, records + b),
            ba = lr_compare_record(records + b, records + a),
            bc = lr_compare_record(records + b, records + c),
            ac = lr_compare_record(records + a, records + c);
        CHECK(ab == -ba && (!ab) == (a == b));
        CHECK(!(ab <= 0 && bc <= 0) || ac <= 0);
      }
  qsort(records, 8, sizeof *records, lr_compare_record);
  const unsigned order[] = {9, 6, 3, 4, 5, 8, 0, 2};
  for (unsigned i = 0; i < 8; ++i)
    CHECK(records[i].identity == order[i]);
  unsigned raw[3] = {0}, accepted[3] = {0}, rejected = 0;
  for (int draw = 0; draw < 8; ++draw) {
    unsigned value = 99;
    enum lr_status result = lr_three(one, &draw, 1, &value);
    ++raw[draw % 3];
    if (draw >= 6) {
      CHECK(result == LR_RETRY && value == 99);
      ++rejected;
    } else {
      CHECK(result == LR_OK && value == (unsigned)draw % 3);
      ++accepted[value];
    }
  }
  CHECK(raw[0] == 3 && raw[1] == 3 && raw[2] == 2 && rejected == 2 &&
        accepted[0] == 2 && accepted[1] == 2 && accepted[2] == 2);
  int bad = 8;
  unsigned value = 99;
  CHECK(lr_three(one, &bad, 1, &value) == LR_SOURCE && value == 99);
  CHECK(lr_three(one, &bad, 0, &value) == LR_RETRY && value == 99);
  for (unsigned fail = 0; fail <= 32; ++fail) {
    unsigned char out[32];
    memset(out, 0xaa, 32);
    struct bytes s = {0, 0, fail};
    enum lr_status r = lr_entropy(source, &s, 20, out);
    if (fail < 32) {
      CHECK(r == LR_SOURCE);
      for (unsigned i = 0; i < 32; ++i)
        CHECK(out[i] == 0xaa);
    } else {
      CHECK(r == LR_OK);
      for (unsigned i = 0; i < 32; ++i)
        CHECK(out[i] == i);
    }
  }
  unsigned char out[32];
  memset(out, 0xaa, 32);
  struct bytes s = {0, 0, 32};
  CHECK(lr_entropy(source, &s, 2, out) == LR_RETRY && s.position == 3);
  for (unsigned i = 0; i < 32; ++i)
    CHECK(out[i] == 0xaa);
  int first[32];
  srand(120617);
  for (unsigned i = 0; i < 32; ++i)
    first[i] = rand();
  srand(120617);
  printf("rand RAND_MAX=%d seed=120617 sequence=", RAND_MAX);
  for (unsigned i = 0; i < 32; ++i) {
    CHECK(first[i] == rand());
    printf("%s%d", i ? "," : "", first[i]);
  }
  puts("");
  /* A deliberately weak original teaching recurrence; observation identifies
     its complete three-bit state. Never a cryptographic implementation. */
  unsigned state = 3;
  for (unsigned i = 0; i < 8; ++i) {
    state = (5 * state + 1) % 8;
    unsigned prediction = (5 * state + 1) % 8;
    state = (5 * state + 1) % 8;
    CHECK(prediction == state);
  }
  printf("SORTING-PASS:%u histogram-oracle arrays;125 integer/512 record "
         "triples;33 failure cuts;bias3,3,2 to2,2,2;8 toy predictions\n",
         cases);
  return 0;
}
