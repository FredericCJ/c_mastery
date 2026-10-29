#include "hashing.h"
#include "replay_depth.h"
#include "replay_vectors.h"
#include <string.h>
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ || _CALL_ELF != 2
#error Requires actual big-endian ELFv2 translation
#endif
/* Explicit audited memory providers for compiler-generated aggregate copies. */
void *memcpy(void *d, const void *s, size_t n) {
  volatile unsigned char *a = d;
  const volatile unsigned char *b = s;
  for (size_t i = 0; i < n; ++i)
    a[i] = b[i];
  return d;
}
void *memset(void *d, int v, size_t n) {
  volatile unsigned char *a = d;
  for (size_t i = 0; i < n; ++i)
    a[i] = (unsigned char)v;
  return d;
}
int memcmp(const void *a, const void *b, size_t n) {
  const unsigned char *x = a, *y = b;
  for (size_t i = 0; i < n; ++i)
    if (x[i] != y[i])
      return x[i] < y[i] ? -1 : 1;
  return 0;
}
extern long sa_raw_write(const char *, size_t);
static int capture(const struct rpr_session *s, struct rph_sha *h) {
  unsigned char b[512];
  struct rpr_session got;
  if (rpr_encode(s, b) || rpr_decode(9, b, &got))
    return 1;
  return rph_update(h, b, 512);
}
static int cases(void) {
  const struct rp_state initial[2] = {{0, 0, 0, 1}, {0, 7, 0, 9}};
  for (unsigned row = 0; row < sizeof vectors / sizeof vectors[0]; ++row) {
    struct rpr_session s;
    struct rpr_notice notice;
    struct rpr_effect effect;
    struct rph_sha h;
    unsigned char digest[32];
    int local[8], remote[8];
    unsigned n = vectors[row].n, lc = vectors[row].local,
             rc = vectors[row].remote, order = vectors[row].order;
    if (rpr_init(9, 8, initial, &s) || rph_init(&h) || capture(&s, &h))
      return 1;
    for (unsigned i = 0; i < n; ++i) {
      local[i] = (int)(lc % 3) - 1;
      lc /= 3;
      remote[i] = (int)(rc % 3) - 1;
      rc /= 3;
    }
    for (unsigned i = 0; i < n; ++i)
      if (rpr_advance(&s, local[i], &notice) || capture(&s, &h))
        return 1;
    for (unsigned i = 0; i < n; ++i) {
      unsigned tick = order % 9;
      order /= 9;
      if (rpr_correct(&s, 9, tick, remote[tick - 1], &notice) ||
          capture(&s, &h))
        return 1;
    }
    for (unsigned i = 0; i < n; ++i)
      if (rpr_prepare(&s, &effect) || capture(&s, &h) ||
          rpr_feedback(&s, effect, AR_OK) || capture(&s, &h))
        return 1;
    if (rph_final(&h, digest) || memcmp(digest, vectors[row].digest, 32))
      return 1;
  }
  return 0;
}
int main(void) {
  int r = cases();
  const char pass[] = "REPLAY_BE_PASS traces128 canonical-full-state\n",
             fail[] = "RP_CHECK big-endian canonical-state\n";
  const char *s = r ? fail : pass;
  size_t n = r ? sizeof fail - 1 : sizeof pass - 1;
  if (sa_raw_write(s, n) != (long)n)
    return 2;
  return r;
}
