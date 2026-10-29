#include "bridge.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "MB_CHECK:%s:%d:%s\n", __func__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct mb_cleanup {
  struct mb_region *region;
  unsigned value;
  unsigned *trace, *count;
  int *busy;
};
static void cleanup(void *v) {
  struct mb_cleanup *c = v;
  struct mb_span s;
  *c->busy = mb_region_alloc(c->region, 1, 1, &s);
  c->trace[(*c->count)++] = c->value;
}
static int region(void) {
  struct mb_region r;
  mb_region_init(&r);
  struct mb_span a, b, sentinel = {0};
  unsigned char out = 99;
  unsigned trace[4] = {0}, count = 0;
  int busy = 0;
  struct mb_cleanup c[4];
  CHECK(mb_region_alloc(&r, 3, 1, &a) == MB_OK);
  CHECK(a.offset == 0 && a.length == 3);
  r.bytes[a.offset] = 42;
  CHECK(mb_region_alloc(&r, 5, 4, &b) == MB_OK);
  CHECK(b.offset == 4 && r.cursor == 9 && r.payload == 8 && r.high == 9);
  r.bytes[b.offset] = 7;
  CHECK(mb_region_read(&r, a, 0, &out) == MB_OK && out == 42);
  CHECK(mb_region_alloc(&r, (size_t)-1, 8, &sentinel) == MB_FULL);
  CHECK(!sentinel.owner && r.cursor == 9);
  CHECK(mb_region_alloc(&r, 0, 1, &sentinel) == MB_INVALID);
  CHECK(mb_region_alloc(&r, 1, 3, &sentinel) == MB_INVALID);
  for (unsigned i = 0; i < 4; i++) {
    c[i] = (struct mb_cleanup){&r, i + 1, trace, &count, &busy};
    CHECK(mb_region_cleanup(&r, cleanup, &c[i]) == MB_OK);
  }
  CHECK(mb_region_cleanup(&r, cleanup, &c[0]) == MB_FULL);
  CHECK(mb_region_reset(&r) == MB_OK);
  CHECK(count == 4 && trace[0] == 4 && trace[1] == 3 && trace[2] == 2 &&
        trace[3] == 1 && busy == MB_BUSY);
  CHECK(r.cursor == 0 && r.payload == 0 && r.high == 9);
  CHECK(mb_region_read(&r, a, 0, &out) == MB_STALE);
  CHECK(mb_region_alloc(&r, 32, 1, &b) == MB_OK && r.high == 32);
  CHECK(mb_region_alloc(&r, 1, 1, &sentinel) == MB_FULL);
  CHECK(mb_region_reset(&r) == MB_OK);
  CHECK(mb_region_reset(&r) == MB_OK);
  CHECK(mb_region_alloc(&r, 1, 1, &b) == MB_FULL);
  CHECK(r.generation == 4);
  return 0;
}
struct thread_arg {
  struct mb_once *once;
  unsigned id;
  atomic_uint *attempts;
  int result;
};
static void *once_worker(void *v) {
  struct thread_arg *t = v;
  unsigned out[2] = {0};
  t->result = 1;
  for (unsigned i = 0; i < 1000000; i++) {
    int rc = mb_once_enter(t->once, t->id, out);
    if (rc == MB_INIT) {
      atomic_fetch_add(t->attempts, 1);
      if (mb_once_finish(t->once, t->id, true, 17, 34) != MB_OK)
        return NULL;
    } else if (rc == MB_OK) {
      t->result = (out[0] != 17 || out[1] != 34);
      return NULL;
    } else if (rc != MB_BUSY && rc != MB_RETRY)
      return NULL;
    sched_yield();
  }
  return NULL;
}
static int once(void) {
  struct mb_once o;
  unsigned out[2] = {91, 92};
  mb_once_init(&o, false);
  CHECK(mb_once_enter(&o, 0, out) == MB_INIT);
  CHECK(mb_once_enter(&o, 1, out) == MB_BUSY);
  CHECK(mb_once_enter(&o, 0, out) == MB_RECURSIVE);
  CHECK(out[0] == 91 && out[1] == 92);
  CHECK(mb_once_finish(&o, 1, true, 5, 6) == MB_STALE);
  CHECK(mb_once_finish(&o, 0, false, 0, 0) == MB_OK);
  CHECK(mb_once_enter(&o, 1, out) == MB_FAILED);
  CHECK(mb_once_enter(&o, 2, out) == MB_FAILED);
  mb_once_init(&o, true);
  CHECK(mb_once_enter(&o, 0, out) == MB_INIT);
  CHECK(mb_once_finish(&o, 0, false, 0, 0) == MB_OK);
  CHECK(mb_once_enter(&o, 1, out) == MB_INIT);
  CHECK(mb_once_finish(&o, 1, true, 17, 34) == MB_OK);
  CHECK(mb_once_enter(&o, 2, out) == MB_OK && out[0] == 17 && out[1] == 34);
  CHECK(mb_once_finish(&o, 1, true, 7, 8) == MB_STALE);
  for (unsigned round = 0; round < 100; round++) {
    mb_once_init(&o, false);
    atomic_uint attempts;
    atomic_init(&attempts, 0);
    pthread_t threads[3];
    struct thread_arg args[3];
    for (unsigned i = 0; i < 3; i++) {
      args[i] = (struct thread_arg){&o, i, &attempts, 1};
      CHECK(pthread_create(&threads[i], NULL, once_worker, &args[i]) == 0);
    }
    for (unsigned i = 0; i < 3; i++) {
      CHECK(pthread_join(threads[i], NULL) == 0);
      CHECK(args[i].result == 0);
    }
    CHECK(atomic_load(&attempts) == 1);
  }
  return 0;
}
static int hazard(void) {
  struct mb_hazards h;
  unsigned out = 999;
  mb_hp_init(&h);
  CHECK(mb_hp_publish(&h, 11) == MB_OK);
  unsigned old = mb_hp_observe(&h);
  CHECK(old != 0);
  CHECK(mb_hp_retire(&h) == MB_OK);
  CHECK(mb_hp_collect(&h) == 1);
  CHECK(mb_hp_publish(&h, 22) == MB_OK);
  CHECK(mb_hp_observe(&h) != old);
  CHECK(mb_hp_protect(&h, 0, old, &out) == MB_RETRY && out == 999);
  CHECK(atomic_load(&h.hazard[0]) == 0);
  unsigned current = mb_hp_observe(&h);
  CHECK(mb_hp_protect(&h, 1, current, &out) == MB_OK && out == 22);
  CHECK(mb_hp_retire(&h) == MB_OK);
  CHECK(mb_hp_collect(&h) == 0);
  mb_hp_clear(&h, 1);
  CHECK(mb_hp_collect(&h) == 1);
  mb_hp_init(&h);
  for (unsigned i = 0; i < 9; i++) {
    CHECK(mb_hp_publish(&h, i) == MB_OK);
    CHECK(mb_hp_retire(&h) == MB_OK);
    CHECK(mb_hp_collect(&h) == 1);
  }
  CHECK(mb_hp_publish(&h, 10) == MB_FULL);
  mb_hp_init(&h);
  CHECK(mb_hp_publish(&h, 1) == MB_OK);
  CHECK(mb_hp_protect(&h, 0, mb_hp_observe(&h), &out) == MB_OK);
  CHECK(mb_hp_retire(&h) == MB_OK);
  CHECK(mb_hp_publish(&h, 2) == MB_OK);
  CHECK(mb_hp_protect(&h, 1, mb_hp_observe(&h), &out) == MB_OK);
  CHECK(mb_hp_retire(&h) == MB_OK);
  CHECK(mb_hp_collect(&h) == 0);
  CHECK(mb_hp_publish(&h, 3) == MB_OK);
  CHECK(mb_hp_retire(&h) == MB_OK);
  CHECK(mb_hp_collect(&h) == 1);
  mb_hp_clear(&h, 0);
  mb_hp_clear(&h, 1);
  CHECK(mb_hp_collect(&h) == 2);
  return 0;
}
static int epoch(void) {
  struct mb_epoch e;
  mb_epoch_init(&e);
  CHECK(mb_epoch_enter(&e, 0) == MB_OK);
  CHECK(mb_epoch_enter(&e, 0) == MB_BUSY);
  CHECK(mb_epoch_retire(&e, 0) == MB_OK);
  CHECK(mb_epoch_advance(&e) == MB_OK);
  CHECK(mb_epoch_advance(&e) == MB_BUSY);
  CHECK(mb_epoch_collect(&e) == 0);
  CHECK(mb_epoch_leave(&e, 0) == MB_OK);
  CHECK(mb_epoch_advance(&e) == MB_OK);
  CHECK(mb_epoch_collect(&e) == 1);
  CHECK(mb_epoch_retire(&e, 0) == MB_STALE);
  CHECK(mb_epoch_advance(&e) == MB_FULL);
  mb_epoch_init(&e);
  CHECK(mb_epoch_retire(&e, 1) == MB_OK);
  CHECK(mb_epoch_advance(&e) == MB_OK);
  CHECK(mb_epoch_collect(&e) == 0);
  CHECK(mb_epoch_enter(&e, 1) == MB_OK);
  CHECK(mb_epoch_advance(&e) == MB_OK);
  CHECK(mb_epoch_collect(&e) == 1);
  CHECK(mb_epoch_leave(&e, 1) == MB_OK);
  CHECK(mb_epoch_leave(&e, 1) == MB_STALE);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "region"))
    return region();
  if (!strcmp(argv[1], "once"))
    return once();
  if (!strcmp(argv[1], "hazard"))
    return hazard();
  if (!strcmp(argv[1], "epoch"))
    return epoch();
  return 2;
}
