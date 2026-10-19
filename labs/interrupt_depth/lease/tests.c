#include "lease.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("XD_CHECK:%d:%s\n", __LINE__, #x);                                \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int equal(const XdLease *a, const XdLease *b) {
  return a->epoch == b->epoch && a->generation == b->generation &&
         a->phase == b->phase && a->terminal == b->terminal &&
         a->terminal_count == b->terminal_count && a->length == b->length &&
         a->complete == b->complete && a->possible == b->possible &&
         a->quiescent == b->quiescent;
}
int main(void) {
  XdLease s = {0};
  XdFence fence = {0, 1, 1, 1};
  XdTicket t = {9, 9};
  XdResult r = {.terminal = 99};
  CHECK(xd_submit(&s, 32, &t) == XD_BUSY && t.epoch == 9);
  CHECK(xd_recover(&s, &fence) == XD_OK);
  CHECK(xd_submit(&s, 0, &t) == XD_INVALID && t.epoch == 9);
  CHECK(xd_submit(&s, 16385, &t) == XD_INVALID && t.epoch == 9);
  CHECK(xd_submit(&s, 32, &t) == XD_OK && t.epoch == 1 && t.generation == 1);
  XdLease unchanged = s;
  XdProof invalid[] = {{t, 2, 0, 1, 32, 32},
                       {t, 0, 0, 2, 32, 32},
                       {t, 0, 0, 1, 33, 32},
                       {t, 0, 0, 1, 32, 33},
                       {t, 0, 0, 1, 31, 32}};
  for (unsigned i = 0; i < sizeof invalid / sizeof invalid[0]; ++i)
    CHECK(xd_observe(&s, t, XD_COMPLETE, &invalid[i]) == XD_INVALID &&
          equal(&s, &unchanged));
  XdProof unavailable = {t, 1, 0, 1, 32, 32};
  CHECK(xd_observe(&s, t, XD_COMPLETE, &unavailable) == XD_BUSY &&
        equal(&s, &unchanged));
  unavailable.enabled = 0;
  unavailable.visibility_done = 0;
  CHECK(xd_observe(&s, t, XD_COMPLETE, &unavailable) == XD_BUSY &&
        equal(&s, &unchanged));
  CHECK(xd_observe(&s, t, XD_NONE, &unavailable) == XD_INVALID &&
        equal(&s, &unchanged));
  CHECK(xd_observe(&s, t, XD_STOPPED, &unavailable) == XD_INVALID &&
        equal(&s, &unchanged));
  CHECK(xd_cancel(&s, t) == XD_OK && s.phase == XD_STOPPING);
  CHECK(xd_reclaim(&s, t, &r) == XD_BUSY && r.terminal == 99);
  XdProof p = {t, 0, 1, 1, 32, 32};
  CHECK(xd_observe(&s, t, XD_COMPLETE, &p) == XD_BUSY && s.terminal_count == 0);
  p.writes_pending = 0;
  CHECK(xd_observe(&s, t, XD_COMPLETE, &p) == XD_OK && s.phase == XD_SERVICE);
  CHECK(xd_observe(&s, t, XD_STOPPED, &p) == XD_DUPLICATE &&
        s.terminal_count == 1);
  CHECK(xd_reclaim(&s, t, &r) == XD_OK && r.terminal == XD_COMPLETE &&
        s.phase == XD_CPU);
  CHECK(xd_reclaim(&s, t, &r) == XD_STALE);
  for (unsigned gen = 2; gen <= 7; ++gen) {
    CHECK(xd_submit(&s, 32, &t) == XD_OK && t.generation == gen);
    CHECK(xd_cancel(&s, t) == XD_OK);
    p = (XdProof){t, 0, 0, 1, 8, 8};
    CHECK(xd_observe(&s, t, XD_STOPPED, &p) == XD_OK);
    p.complete = p.possible = 32;
    CHECK(xd_observe(&s, t, XD_COMPLETE, &p) == XD_STALE &&
          s.terminal == XD_STOPPED && s.terminal_count == 1);
    CHECK(xd_reclaim(&s, t, &r) == XD_OK && r.complete == 8);
  }
  XdTicket old = t;
  CHECK(xd_submit(&s, 32, &t) == XD_OK && t.generation == 8);
  p = (XdProof){old, 0, 0, 1, 32, 32};
  CHECK(xd_observe(&s, old, XD_COMPLETE, &p) == XD_STALE &&
        s.phase == XD_DEVICE && s.terminal_count == 0);
  /* Software reset cannot manufacture a hardware quiescence observation. */
  xd_quarantine(&s);
  fence = (XdFence){1, 2, 1, 0};
  CHECK(xd_recover(&s, &fence) == XD_BUSY && s.phase == XD_QUARANTINE);
  fence.writes_drained = 1;
  CHECK(xd_recover(&s, &fence) == XD_OK && s.epoch == 2 && s.generation == 0);
  CHECK(xd_submit(&s, 32, &t) == XD_OK && t.epoch == 2 && t.generation == 1);
  CHECK(xd_observe(&s, old, XD_COMPLETE, &p) == XD_STALE &&
        s.phase == XD_DEVICE);
  p = (XdProof){t, 0, 0, 1, 0, 32};
  CHECK(xd_observe(&s, t, XD_ERROR, &p) == XD_OK);
  CHECK(xd_reclaim(&s, t, &r) == XD_OK && r.terminal == XD_ERROR &&
        r.possible == 32);
  s.generation = UINT32_MAX;
  CHECK(xd_submit(&s, 32, &t) == XD_EXHAUSTED);
  xd_quarantine(&s);
  fence = (XdFence){UINT32_MAX, 0, 1, 1};
  CHECK(xd_recover(&s, &fence) == XD_EXHAUSTED);
  puts("XD_PASS cancellation_generation_reset");
  return 0;
}
