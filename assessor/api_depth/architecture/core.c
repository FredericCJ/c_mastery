#include "arch.h"
#include <limits.h>
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_double(const int32_t *in, size_t n, int32_t *out) {
  if (n > 8 || (!in && n) || (!out && n))
    return AR_INVALID;
  int32_t tmp[8];
  for (size_t i = 0; i < n; ++i) {
    if (in[i] > INT32_MAX / 2 || in[i] < INT32_MIN / 2)
      return AR_RANGE;
    tmp[i] = in[i] * (AR_MUTANT == 1 ? 1 : 2);
  }
  for (size_t i = 0; i < n; ++i)
    out[i] = tmp[i];
  return AR_OK;
}
int ar_window(ArWindow *s, int32_t v, int32_t *out) {
  if (!s || !out)
    return AR_INVALID;
  int64_t sum = (int64_t)v + (s->present ? s->previous : 0);
  if (sum < INT32_MIN || sum > INT32_MAX)
    return AR_RANGE;
  s->previous = v;
  s->present = 1;
  *out = (int32_t)sum;
  return AR_OK;
}
int ar_encode(unsigned v, ArRecord r, unsigned char *p, size_t n) {
  size_t width = v == 1 ? 2 : 4;
  if ((v != 1 && v != 2) || !p || n != width + 1)
    return AR_INVALID;
  if (v == 1 && (r.value < -32768 || r.value > 32767))
    return AR_RANGE;
  uint32_t u = (uint32_t)r.value;
  unsigned char check = 0;
  for (size_t i = 0; i < width; ++i) {
    p[width - i - 1] = (unsigned char)(u >> (i * 8));
    check ^= p[width - i - 1];
  }
  p[width] = check;
  return AR_OK;
}
int ar_decode(unsigned v, const unsigned char *p, size_t n, ArRecord *out) {
  size_t w = v == 1 ? 2 : 4;
  if ((v != 1 && v != 2) || !p || !out || n != w + 1)
    return AR_INVALID;
  unsigned char check = 0;
  uint32_t u = 0;
  for (size_t i = 0; i < w; ++i) {
    u = (u << 8) | p[i];
    check ^= p[i];
  }
  if (check != p[w])
    return AR_INVALID;
  int64_t value = u;
  if (p[0] & 128u)
    value -= v == 1 ? INT64_C(65536) : INT64_C(4294967296);
  out->value = (int32_t)value;
  return AR_OK;
}
int ar_feed(ArReader *r, const unsigned char *p, size_t n, ArRecord *out) {
  if (!r || !out || (!p && n) || (r->version != 1 && r->version != 2))
    return AR_INVALID;
  size_t want = r->version == 1 ? 3 : 5;
  if (r->used > want || n > want - r->used)
    return AR_RANGE;
  ArReader t = *r;
  for (size_t i = 0; i < n; ++i)
    t.bytes[t.used++] = p[i];
  if (t.used < want) {
    *r = t;
    return AR_PENDING;
  }
  ArRecord value;
  int result = ar_decode(t.version, t.bytes, want, &value);
  if (result != AR_OK)
    return result;
  t.used = 0;
  *r = t;
  *out = value;
  return AR_OK;
}
int ar_process(ArStorage s, size_t n, int32_t *out) {
  if (!s.get || n > 8 || (!out && n))
    return AR_INVALID;
  int32_t in[8];
  for (size_t i = 0; i < n; ++i) {
    ArRecord r;
    int result = s.get(s.context, i, &r);
    if (result != AR_OK)
      return result;
    in[i] = r.value;
  }
  return ar_double(in, n, out);
}
