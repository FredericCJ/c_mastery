#define _POSIX_C_SOURCE 200809L
#include "cmx_advanced.h"
#include "grace.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "AG_ASSERT:%d:%s\n", __LINE__, #x);                      \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static void epoch(void) {
  AGDomain d;
  AGLease l;
  ag_init(&d, 0);
  CHECK(ag_register(&d, &l) == AG_OK);
  for (unsigned e = 0; e < 4; e++)
    CHECK(ag_advance(&d) == AG_OK);
  CHECK(ag_enter(&d, l) == AG_OK && d.retained.seen[0] == 4);
  CHECK(ag_enter(&d, l) == AG_INVALID);
  CHECK(ag_advance(&d) == AG_OK);
  CHECK(ag_reclaimable(&d, 2) && !ag_reclaimable(&d, 5));
  CHECK(ag_advance(&d) == AG_PENDING && ag_depart(&d, l) == AG_BUSY);
  /* Deliberately injected conservative ledger, not a reachable tracker trace.
   */
  d.retained.active[1] = 1;
  d.retained.seen[1] = 1;
  CHECK(!ag_reclaimable(&d, 2) && !ag_reclaimable(&d, 5));
  d.retained.active[1] = 0;
  CHECK(ag_leave(&d, l) == AG_OK && ag_reclaimable(&d, 5));
  CHECK(ag_leave(&d, l) == AG_INVALID);
  CHECK(ag_depart(&d, l) == AG_OK);
  AGLease next;
  CHECK(ag_register(&d, &next) == AG_OK && next.generation != l.generation);
  CHECK(ag_enter(&d, l) == AG_INVALID);
  d.retained.epoch = UINT32_MAX;
  CHECK(ag_advance(&d) == AG_EXHAUSTED);
  cmx_epoch retained;
  cmx_epoch_init(&retained);
  CHECK(cmx_epoch_advance(&retained));
  CHECK(cmx_epoch_enter(&retained, 0) == CMX_OK);
  CHECK(cmx_epoch_advance(&retained));
  CHECK(!cmx_epoch_advance(&retained) && retained.epoch == 2);
  ag_init(&d, 1);
  CHECK(ag_register(&d, &l) == AG_OK);
  CHECK(ag_enter(&d, l) == AG_OK);
  CHECK(ag_enter(&d, l) == AG_OK);
  CHECK(ag_advance(&d) == AG_OK);
  CHECK(ag_leave(&d, l) == AG_OK);
  CHECK(d.retained.active[0] && d.depth[0] == 1 && !ag_reclaimable(&d, 0));
  CHECK(ag_leave(&d, l) == AG_OK && !d.retained.active[0] &&
        ag_reclaimable(&d, 0));
  d.section[0] = UINT_MAX;
  CHECK(ag_enter(&d, l) == AG_EXHAUSTED);
  puts("OBS epoch reachable_active4_retired2_5 injected_active1_unreachable "
       "nested_outermost saturation");
}
static void versions(void) {
  AGStore s;
  AGLease old, new;
  AGBorrow before, after;
  unsigned char data[32];
  ag_store_init(&s, 0, 1);
  CHECK(ag_register(&s.domain, &old) == AG_OK &&
        ag_register(&s.domain, &new) == AG_OK);
  CHECK(ag_read(&s, old, &before) == AG_OK);
  CHECK(ag_writer_acquire(&s, 7) == AG_OK);
  CHECK(ag_publish(&s, 7, 2) == AG_OK);
  CHECK(ag_writer_release(&s, 7) == AG_OK);
  CHECK(ag_read(&s, new, &after) == AG_OK &&
        after.version.index != before.version.index);
  CHECK(ag_copy(&s, before, data) == AG_OK && data[0] == 1);
  CHECK(ag_copy(&s, after, data) == AG_OK && data[0] == 2);
  CHECK(ag_collect(&s) == 0);
  CHECK(ag_read_end(&s, before) == AG_OK);
  CHECK(ag_collect(&s) == 1 && s.domain.retained.active[new.participant]);
  data[0] = 99;
  CHECK(ag_copy(&s, before, data) == AG_INVALID && data[0] == 99);
  CHECK(ag_read_end(&s, after) == AG_OK);
  cmx_rcu baseline;
  unsigned root;
  cmx_rcu_init(&baseline, 1);
  CHECK(cmx_rcu_read(&baseline, 0, &root) == CMX_OK && root == 1);
  CHECK(cmx_rcu_publish(&baseline, 2) == 1);
  CHECK(cmx_rcu_read(&baseline, 1, &root) == CMX_OK && root == 2);
  CHECK(cmx_rcu_leave(&baseline, 0) == CMX_OK);
  CHECK(!cmx_rcu_quiescent(&baseline));
  CHECK(cmx_rcu_leave(&baseline, 1) == CMX_OK && cmx_rcu_quiescent(&baseline));
  puts("OBS versions old1_new2 old_exit_reclaims_with_new_reader "
       "coarse_retained_waits_all borrow_rejected");
}
static void pressure(void) {
  AGStore s;
  AGLease l;
  AGBorrow b;
  ag_store_init(&s, 0, 0);
  CHECK(ag_register(&s.domain, &l) == AG_OK);
  CHECK(ag_read(&s, l, &b) == AG_OK);
  CHECK(ag_writer_acquire(&s, 1) == AG_OK);
  for (unsigned n = 1; n <= 16; n++) {
    CHECK(ag_publish(&s, 1, (unsigned char)n) == AG_OK);
    CHECK(ag_collect(&s) == 0);
  }
  CHECK(ag_publish(&s, 1, 17) == AG_FULL);
  unsigned retired = 0;
  for (unsigned i = 0; i < 17; i++)
    retired += s.objects[i].active && s.objects[i].retired;
  CHECK(retired == 16 && s.domain.retained.epoch == 1);
  CHECK(ag_depart(&s.domain, l) == AG_BUSY);
  CHECK(ag_read_end(&s, b) == AG_OK);
  CHECK(ag_collect(&s) == 16);
  CHECK(ag_publish(&s, 1, 17) == AG_OK);
  CHECK(ag_writer_release(&s, 1) == AG_OK);
  printf("OBS pressure retained_objects=16 payload_bytes=512 object_bytes=%zu "
         "capacity=17 full_then_recovery\n",
         16 * sizeof(AGObject));
}
typedef struct {
  char trace[8];
  unsigned length;
  ApiHandle other;
  int remove, destroy;
} Log;
static void callback(void *ctx, AGStore *s, ApiHandle self,
                     const unsigned char bytes[32]) {
  Log *log = ctx;
  for (unsigned i = 0; i < 32; i++)
    CHECK(bytes[i] == 1);
  log->trace[log->length++] = 'A';
  CHECK(ag_collect(s) == 0);
  if (log->remove) {
    CHECK(ag_remove(s, self) == API_PENDING);
    CHECK(ag_remove(s, log->other) == API_PENDING);
    CHECK(ag_collect(s) == 0);
    CHECK(s->drops == 0);
    log->remove = 0;
  }
  if (log->destroy)
    CHECK(ag_destroy_callbacks(s) == API_PENDING);
}
static void callbacks(void) {
  for (unsigned destroy = 0; destroy < 2; destroy++) {
    AGStore s;
    AGLease l;
    AGBorrow b;
    ApiHandle a, z;
    Log log = {{0}, 0, {0, 0}, !destroy, (int)destroy};
    ag_store_init(&s, 0, 1);
    CHECK(ag_register(&s.domain, &l) == AG_OK);
    CHECK(ag_read(&s, l, &b) == AG_OK);
    CHECK(ag_capture(&s, b, callback, &log, &a) == API_OK);
    CHECK(ag_capture(&s, b, callback, &log, &z) == API_OK);
    log.other = z;
    CHECK(s.objects[b.version.index].refs == 2);
    CHECK(ag_writer_acquire(&s, 0) == AG_OK && ag_publish(&s, 0, 2) == AG_OK &&
          ag_writer_release(&s, 0) == AG_OK);
    CHECK(ag_read_end(&s, b) == AG_OK);
    CHECK(ag_capture(&s, b, callback, &log, &z) == API_INVALID);
    CHECK(ag_collect(&s) == 0 && s.objects[b.version.index].active);
    /* First callback removes both; second still owns its snapshot reference. */
    ApiStatus r = ag_dispatch(&s);
    CHECK(r == (destroy ? API_CLOSED : API_OK));
    CHECK(log.length == (destroy ? 1u : 2u) && s.invocations == log.length);
    CHECK(s.drops == 2 && s.objects[b.version.index].refs == 0);
    CHECK(ag_collect(&s) == 1);
  }
  AGStore full;
  AGLease l;
  AGBorrow b;
  ApiHandle h[4], sentinel = {99, 99};
  Log log = {{0}, 0, {0, 0}, 0, 0};
  ag_store_init(&full, 0, 1);
  CHECK(ag_register(&full.domain, &l) == AG_OK &&
        ag_read(&full, l, &b) == AG_OK);
  for (unsigned i = 0; i < 4; i++)
    CHECK(ag_capture(&full, b, callback, &log, &h[i]) == API_OK);
  CHECK(ag_capture(&full, b, callback, &log, &sentinel) == API_FULL &&
        sentinel.index == 99 && full.objects[0].refs == 4);
  CHECK(ag_destroy_callbacks(&full) == API_OK && full.objects[0].refs == 0);
  CHECK(ag_capture(&full, b, callback, &log, &sentinel) == API_CLOSED &&
        full.objects[0].refs == 0);
  CHECK(ag_read_end(&full, b) == AG_OK);
  puts("OBS callbacks actual_registry refs_acquired_before_exit delayed_drop "
       "destroy_suppression full_rollback");
}
typedef struct {
  AGStore *s;
  pthread_mutex_t *mutex;
  unsigned id, *done;
} Writer;
static void *writer(void *ptr) {
  Writer *w = ptr;
  for (unsigned i = 0; i < 32; i++) {
    CHECK(!pthread_mutex_lock(w->mutex));
    CHECK(ag_writer_acquire(w->s, w->id) == AG_OK);
    CHECK(ag_publish(w->s, w->id, (unsigned char)(w->id * 32 + i)) == AG_OK);
    CHECK(ag_writer_release(w->s, w->id) == AG_OK);
    ag_collect(w->s);
    (*w->done)++;
    CHECK(!pthread_mutex_unlock(w->mutex));
  }
  return NULL;
}
static void writers(void) {
  AGStore s;
  ag_store_init(&s, 0, 0);
  CHECK(ag_writer_acquire(&s, 1) == AG_OK);
  CHECK(ag_writer_acquire(&s, 2) == AG_BUSY);
  CHECK(ag_publish(&s, 2, 99) == AG_INVALID);
  CHECK(ag_writer_release(&s, 2) == AG_INVALID);
  CHECK(ag_writer_release(&s, 1) == AG_OK);
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  unsigned done = 0;
  Writer args[2] = {{&s, &mutex, 1, &done}, {&s, &mutex, 2, &done}};
  pthread_t threads[2];
  for (unsigned i = 0; i < 2; i++)
    CHECK(!pthread_create(&threads[i], NULL, writer, &args[i]));
  for (unsigned i = 0; i < 2; i++)
    CHECK(!pthread_join(threads[i], NULL));
  CHECK(done == 64 && s.writer == AG_NONE);
  CHECK(!pthread_mutex_destroy(&mutex));
  puts("OBS writers two_actual_threads explicit_mutex_domain publications=64 "
       "unsynchronized_core_not_authorized");
}
typedef struct {
  AGStore *s;
  AGLease lease;
  AGBorrow borrow;
} Depart;
static void *depart(void *ptr) {
  Depart *d = ptr;
  CHECK(ag_read(d->s, d->lease, &d->borrow) == AG_OK);
  return NULL;
}
static void departure(void) {
  AGStore s;
  AGLease lease;
  ag_store_init(&s, 0, 1);
  CHECK(ag_register(&s.domain, &lease) == AG_OK);
  Depart d = {.s = &s, .lease = lease};
  pthread_t worker;
  CHECK(!pthread_create(&worker, NULL, depart, &d));
  CHECK(!pthread_join(worker, NULL));
  CHECK(ag_writer_acquire(&s, 0) == AG_OK && ag_publish(&s, 0, 2) == AG_OK);
  CHECK(ag_collect(&s) == 0);
  CHECK(ag_depart(&s.domain, lease) == AG_BUSY);
  CHECK(ag_read_end(&s, d.borrow) == AG_OK &&
        ag_depart(&s.domain, lease) == AG_OK);
  CHECK(ag_collect(&s) == 1);
  CHECK(ag_enter(&s.domain, lease) == AG_INVALID);
  puts("OBS departure joined_owner_external_fence active_departure_busy "
       "successor_unwinds_then_reclaims");
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "epoch"))
    epoch();
  else if (!strcmp(argv[1], "versions"))
    versions();
  else if (!strcmp(argv[1], "pressure"))
    pressure();
  else if (!strcmp(argv[1], "callbacks"))
    callbacks();
  else if (!strcmp(argv[1], "writers"))
    writers();
  else if (!strcmp(argv[1], "departure"))
    departure();
  else
    return 2;
  return 0;
}
