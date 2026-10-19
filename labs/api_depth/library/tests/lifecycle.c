#include "lc.h"
#include "support.h"
#include "test.h"
#include <string.h>
typedef struct {
  unsigned fail, events[16], n, live;
} Log;
static int acquire(void *v, unsigned id) {
  Log *l = v;
  l->events[l->n++] = id;
  if (id == l->fail)
    return 0;
  l->live |= 1u << id;
  return 1;
}
static void release(void *v, unsigned id) {
  Log *l = v;
  l->events[l->n++] = 10 + id;
  l->live &= ~(1u << id);
}
static void done(void *v, LCStatus s) {
  unsigned *p = v;
  p[0]++;
  p[1] = (unsigned)s;
}
int main(void) {
  for (int optional = 0; optional <= 1; optional++)
    for (unsigned fail = 0; fail <= 3; fail++) {
      Log before = {fail, {0}, 0, 0}, after = before, helper = before;
      LCPair a = {99, {0}}, b = a, c = a;
      int x =
          lc_construct_nested(&a, (LCOps){acquire, release, &before}, optional);
      int y = lc_construct(&b, (LCOps){acquire, release, &after}, optional);
      int z = lc_construct_helpers(&c, (LCOps){acquire, release, &helper},
                                   optional);
      CHECK(x == y && y == z);
      CHECK(x == (fail == 0 || (!optional && fail == 2)));
      CHECK(before.n == after.n && before.n == helper.n &&
            !memcmp(before.events, after.events, before.n * sizeof(unsigned)) &&
            !memcmp(before.events, helper.events, before.n * sizeof(unsigned)));
      if (x) {
        lc_destroy(&a);
        lc_destroy(&b);
        lc_destroy(&c);
      } else
        CHECK(a.mask == 99 && b.mask == 99 && c.mask == 99);
      CHECK(!before.live && !after.live && !helper.live);
      CHECK(before.n == after.n &&
            !memcmp(before.events, after.events, before.n * sizeof(unsigned)));
    }
  unsigned log[2] = {0};
  CHECK(lc_sync(done, log) == LC_OK && log[0] == 1);
  log[0] = 0;
  LCOperation *p = lc_start(done, log);
  CHECK(p && log[0] == 0);
  CHECK(lc_release_operation(p) == LC_BUSY);
  CHECK(lc_cancel(p) == LC_PENDING);
  CHECK(log[0] == 0);
  CHECK(lc_release_operation(p) == LC_BUSY);
  CHECK(lc_poll(p) == LC_PENDING);
  CHECK(lc_ack(p) == LC_OK && log[0] == 1 && log[1] == LC_CANCELLED);
  CHECK(lc_ack(p) == LC_OK && log[0] == 1);
  CHECK(lc_release_operation(p) == LC_OK);
  log[0] = 0;
  p = lc_start(done, log);
  CHECK(p);
  CHECK(lc_poll(p) == LC_OK && log[0] == 1 && log[1] == LC_OK);
  CHECK(lc_poll(p) == LC_OK && log[0] == 1);
  CHECK(lc_release_operation(p) == LC_OK);
  LCBackend host = {0, 3}, board = {0, 7};
  CHECK(lc_backend_run((LCBackendOps){lc_host_backend, &host}, 10) == 13);
  CHECK(lc_backend_run((LCBackendOps){lc_board_backend, &board}, 10) == 3);
  CHECK(host.calls == 1 && board.calls == 1);
  puts("LC_LIFECYCLE all failure prefixes, optional resource, acknowledged "
       "cancellation, separate backend contexts passed");
  return 0;
}
