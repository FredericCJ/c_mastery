#include "service.h"
#ifndef PL_BIG_ENDIAN
#include <stdio.h>
#else
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("PL_CHECK:%u\n", (unsigned)__LINE__);                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  const uint8_t *borrow;
  uint32_t token;
  unsigned accepted, cancelled, done, fail, attempts;
} Script;
static int send(void *c, const uint8_t *p, unsigned n, uint32_t t, uint32_t now,
                uint32_t budget) {
  Script *m = c;
  m->attempts++;
  (void)now;
  (void)budget;
  if (m->borrow || n != 8)
    return PL_BUSY;
  if (m->fail)
    return PL_IO;
  m->borrow = p;
  m->token = t;
  m->accepted++;
  return PL_OK;
}
static int poll(void *c, uint32_t t, uint32_t now) {
  Script *m = c;
  (void)now;
  if (t != m->token)
    return PL_STALE;
  if (!m->done)
    return PL_BUSY;
  m->borrow = 0;
  return m->cancelled ? PL_CANCELLED : PL_OK;
}
static int cancel(void *c, uint32_t t) {
  Script *m = c;
  if (t != m->token)
    return PL_STALE;
  m->cancelled++;
  return PL_OK;
}
int main(void) {
  uint8_t bytes[8];
  CHECK(pl_encode(0, 0, bytes) == PL_OK);
  for (unsigned i = 1; i < 7; i++)
    CHECK(bytes[i] == 0x10);
  CHECK(bytes[0] == 0xf0 && bytes[7] == 0xf1);
  for (unsigned seq = 0; seq < 256; seq++)
    for (unsigned value = 0; value < 256; value++) {
      PlParser p = {0};
      PlCommand out = {99, 99};
      CHECK(pl_encode((uint8_t)seq, (uint8_t)value, bytes) == PL_OK);
      for (unsigned k = 0; k < 8; k++)
        CHECK(pl_feed(&p, bytes[k], &out) == (k == 7 ? PL_OK : PL_BUSY));
      CHECK(out.sequence == seq && out.value == value);
    }
  for (unsigned cut = 0; cut < 8; cut++) {
    PlParser p = {0};
    PlCommand out = {99, 99};
    CHECK(pl_encode(0xf0, 0xf1, bytes) == PL_OK);
    for (unsigned k = 0; k < cut; k++)
      CHECK(pl_feed(&p, bytes[k], &out) == PL_BUSY);
    CHECK(out.value == 99);
    CHECK(pl_encode(4, 5, bytes) == PL_OK);
    for (unsigned k = 0; k < 8; k++)
      CHECK(pl_feed(&p, bytes[k], &out) == (k == 7 ? PL_OK : PL_BUSY));
    CHECK(out.value == 5);
  }
  PlParser p = {0};
  PlCommand out = {99, 99};
  CHECK(pl_encode(4, 5, bytes) == PL_OK);
  bytes[6] ^= 1;
  for (unsigned k = 0; k < 8; k++)
    CHECK(pl_feed(&p, bytes[k], &out) == (k == 7 ? PL_FRAME : PL_BUSY));
  CHECK(out.value == 99);
  Script m = {0};
  PlTransport t = {&m, send, poll, cancel};
  PlService s = {0};
  PlResult r = {99, 99, 99};
  CHECK(pl_send(&s, t, 1, 2, 1, 0, 100) == PL_OK && m.borrow == s.bytes);
  CHECK(pl_send(&s, t, 3, 4, 2, 0, 100) == PL_BUSY && m.accepted == 1 &&
        m.attempts == 1 && s.bytes[2] == 0x11);
  CHECK(pl_cancel(&s, t, 1) == PL_OK && s.active && m.borrow);
  CHECK(pl_poll(&s, t, 1, 1, &r) == PL_BUSY && r.token == 99);
  m.done = 1;
  CHECK(pl_poll(&s, t, 1, 2, &r) == PL_OK && r.status == PL_CANCELLED &&
        r.may_have_committed == 1 && !s.active);
  CHECK(pl_poll(&s, t, 1, 3, &r) == PL_STALE);
  m.fail = 1;
  CHECK(pl_send(&s, t, 8, 9, 2, 3, 100) == PL_IO && s.token == 1 &&
        s.bytes[2] == 0x11);
  CHECK(pl_send(&s, t, 8, 9, 1, 3, 100) == PL_STALE);
  s.token = UINT32_MAX;
  CHECK(pl_send(&s, t, 8, 9, 1, 3, 100) == PL_STALE);
  CHECK(pl_send(&s, t, 8, 9, 0, 3, 100) == PL_INVALID);
  CHECK(pl_send(&s, t, 8, 9, 2, 3, 0x80000000u) == PL_INVALID);
  /* Local synthetic attack: valid CRC recomputation and replay are accepted.
   * This demonstrates absent authenticity/deduplication, not memory corruption.
   */
  unsigned effects = 0;
  for (unsigned replay = 0; replay < 2; replay++) {
    PlParser attacker_parser = {0};
    PlCommand synthetic = {0};
    CHECK(pl_encode(4, 222, bytes) == PL_OK);
    for (unsigned k = 0; k < 8; k++)
      effects += pl_feed(&attacker_parser, bytes[k], &synthetic) == PL_OK;
    CHECK(synthetic.value == 222);
  }
  CHECK(effects == 2);
  for (unsigned n = 0; n < 7; n++) {
    PlParser short_frame = {0};
    PlCommand sentinel = {99, 99};
    CHECK(pl_feed(&short_frame, 0xf0, &sentinel) == PL_BUSY);
    for (unsigned k = 0; k < n; k++)
      CHECK(pl_feed(&short_frame, 0x10, &sentinel) == PL_BUSY);
    CHECK(pl_feed(&short_frame, 0xf1, &sentinel) ==
          (n == 6 ? PL_OK : PL_FRAME));
    CHECK(sentinel.value == (n == 6 ? 0u : 99u));
  }
  {
    PlParser overflow = {0};
    PlCommand sentinel = {99, 99};
    CHECK(pl_feed(&overflow, 0xf0, &sentinel) == PL_BUSY);
    for (unsigned k = 0; k < 6; k++)
      CHECK(pl_feed(&overflow, 0x10, &sentinel) == PL_BUSY);
    CHECK(pl_feed(&overflow, 0x10, &sentinel) == PL_FRAME &&
          sentinel.value == 99);
    CHECK(pl_feed(&overflow, 0xf1, &sentinel) == PL_BUSY &&
          sentinel.value == 99);
  }
  puts("PL_PASS");
  return 0;
}
