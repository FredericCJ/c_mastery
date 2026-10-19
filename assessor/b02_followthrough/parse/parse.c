#include "parse.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
static unsigned digit(unsigned char b) {
  return b >= 48 && b <= 57   ? b - 48u
         : b >= 65 && b <= 90 ? b - 65u + 10u
                              : 36u;
}
int bf_parse_reset(BfParse *s, unsigned base) {
  if (!s || base < 2 || base > 36)
    return B2_INVALID;
  s->base = base;
  s->phase = 0;
  s->negative = 0;
  s->seen = 0;
  s->zero = 0;
  if (BF_MUTANT != 5)
    s->invalid = 0;
  s->range = 0;
  s->magnitude = 0;
  return B2_OK;
}
int bf_parse_feed(BfParse *s, const unsigned char *p, size_t n) {
  if (!s || (!p && n))
    return B2_INVALID;
  if (s->phase)
    return BF_CLOSED;
  for (size_t i = 0; i < n; i++) {
    unsigned char b = p[i];
    if (!s->seen && !s->negative && b == 45) {
      s->negative = 1;
      continue;
    }
    unsigned d = digit(b);
    if (d >= s->base ||
        (BF_MUTANT != 2 && (s->zero || (!s->seen && s->negative && !d))))
      s->invalid = 1;
    if (!s->seen)
      s->zero = d == 0;
    s->seen = 1;
    if (d >= s->base || s->range)
      continue;
    uint32_t limit = s->negative ? UINT32_C(2147483648) : UINT32_C(2147483647);
    if (s->magnitude > (limit - d) / s->base) {
      s->range = 1;
      if (BF_MUTANT != 3)
        continue;
    }
    s->magnitude = s->magnitude * s->base + d;
    if (BF_MUTANT == 3)
      s->range = 0;
  }
  return B2_OK;
}
int bf_parse_finish(BfParse *s, int32_t *out) {
  if (!s || !out)
    return B2_INVALID;
  if (s->phase == 2)
    return BF_CANCELLED;
  s->phase = 1;
  if (BF_MUTANT == 1 && s->range)
    return B2_RANGE;
  if (!s->seen || s->invalid)
    return B2_INVALID;
  if (s->range)
    return B2_RANGE;
  if (s->negative)
    *out = s->magnitude == UINT32_C(2147483648) ? INT32_MIN
                                                : -(int32_t)s->magnitude;
  else
    *out = (int32_t)s->magnitude;
  return B2_OK;
}
int bf_parse_cancel(BfParse *s) {
  if (!s)
    return B2_INVALID;
  s->phase = 2;
  s->magnitude = 0;
  return B2_OK;
}
int bf_parse_radix(const unsigned char *p, size_t n, unsigned base,
                   int32_t *out) {
  BfParse s;
  s.invalid = 0;
  if (!out || (!p && n))
    return B2_INVALID;
  int r = bf_parse_reset(&s, base);
  if (r)
    return r;
  r = bf_parse_feed(&s, p, n);
  return r ? r : bf_parse_finish(&s, out);
}
