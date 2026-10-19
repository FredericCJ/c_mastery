#define _POSIX_C_SOURCE 200809L
#include "atomics.h"
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "TA_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x);         \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static void graph(void) {
  unsigned publication[8] = {2, 4, 8, 16, 32, 64, 128, 0};
  CHECK(ta_closure(publication, 8) == TA_SUCCESS);
  CHECK(publication[0] & (1u << 3));
  CHECK(publication[3] & (1u << 6));
  unsigned initial[4] = {2, 0, 8, 0};
  CHECK(ta_closure(initial, 4) == TA_SUCCESS);
  CHECK(!(initial[0] & (1u << 2)));
  /* Independent worklist traversal, including cyclic input; no memory-model
     classification is inferred just from a graph's transitive closure. */
  for (unsigned bits = 0; bits < 65536; bits++) {
    unsigned original[4], closed[4];
    for (unsigned i = 0; i < 4; i++)
      original[i] = closed[i] = (bits >> (4 * i)) & 15u;
    CHECK(ta_closure(closed, 4) == TA_SUCCESS);
    for (unsigned start = 0; start < 4; start++) {
      unsigned pending = original[start], seen = 0;
      while (pending) {
        unsigned i = 0;
        while (!(pending & (1u << i)))
          i++;
        pending &= ~(1u << i);
        if (!(seen & (1u << i))) {
          seen |= 1u << i;
          pending |= original[i] & ~seen;
        }
      }
      CHECK(closed[start] == seen);
    }
  }
  unsigned invalid[2] = {4, 0};
  CHECK(ta_closure(invalid, 2) == TA_INVALID && invalid[0] == 4 &&
        invalid[1] == 0);
  puts("OBS graph65536 closures; release read-from edge; initial read no "
       "publication; reverse reuse");
}
static void conflict(void) {
  /* Named semantic cases: ordinary writes, representation write versus atomic
     write, disjoint elements, adjacent nonzero bit-fields, zero-width split. */
  const unsigned cases[][6] = {{1, 1, 1, 0, 0, 1}, {1, 1, 1, 0, 1, 0},
                               {1, 1, 1, 1, 0, 0}, {0, 1, 1, 0, 0, 0},
                               {1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 0},
                               {1, 0, 0, 0, 0, 0}};
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    CHECK(ta_data_race(cases[i][0], cases[i][1], cases[i][2], cases[i][3],
                       cases[i][4]) == (int)cases[i][5]);
  CHECK(ta_data_race(2, 0, 0, 0, 0) == -1);
  puts("OBS conflict7 analysis-only access cases; adjacent bit-field location "
       "differs from array elements");
}
typedef struct {
  TASnapshot *snapshot;
  atomic_int stop;
  unsigned successes, limits;
} Snapshot;
typedef struct {
  atomic_int a, b;
  pthread_barrier_t entered, observed;
  pthread_mutex_t mutex;
  int locked, plain_a, plain_b;
} SplitBalance;
static void *split_writer(void *p) {
  SplitBalance *s = p;
  if (s->locked) {
    CHECK(pthread_mutex_lock(&s->mutex) == 0);
    s->plain_a = 40;
  }
  atomic_store(&s->a, 40);
  int e = pthread_barrier_wait(&s->entered);
  CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
  e = pthread_barrier_wait(&s->observed);
  CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
  atomic_store(&s->b, 60);
  if (s->locked) {
    s->plain_b = 60;
    CHECK(pthread_mutex_unlock(&s->mutex) == 0);
  }
  return NULL;
}
static void split_balance(void) {
  SplitBalance s;
  s.locked = 0;
  s.plain_a = s.plain_b = 50;
  CHECK(pthread_mutex_init(&s.mutex, NULL) == 0);
  atomic_init(&s.a, 50);
  atomic_init(&s.b, 50);
  CHECK(pthread_barrier_init(&s.entered, NULL, 2) == 0);
  CHECK(pthread_barrier_init(&s.observed, NULL, 2) == 0);
  pthread_t t;
  CHECK(pthread_create(&t, NULL, split_writer, &s) == 0);
  int e = pthread_barrier_wait(&s.entered);
  CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
  CHECK(atomic_load(&s.a) + atomic_load(&s.b) == 90);
  e = pthread_barrier_wait(&s.observed);
  CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
  CHECK(pthread_join(t, NULL) == 0);
  CHECK(atomic_load(&s.a) + atomic_load(&s.b) == 100);
  s.locked = 1;
  CHECK(pthread_create(&t, NULL, split_writer, &s) == 0);
  e = pthread_barrier_wait(&s.entered);
  CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
  CHECK(pthread_mutex_trylock(&s.mutex) == EBUSY);
  e = pthread_barrier_wait(&s.observed);
  CHECK(e == 0 || e == PTHREAD_BARRIER_SERIAL_THREAD);
  CHECK(pthread_mutex_lock(&s.mutex) == 0);
  CHECK(s.plain_a + s.plain_b == 100);
  CHECK(pthread_mutex_unlock(&s.mutex) == 0);
  CHECK(pthread_join(t, NULL) == 0);
  CHECK(pthread_mutex_destroy(&s.mutex) == 0);
  CHECK(pthread_barrier_destroy(&s.entered) == 0);
  CHECK(pthread_barrier_destroy(&s.observed) == 0);
  puts("BALANCE atomic fields permit actual forced total90; final total100; no "
       "C data race");
}
static void *snapshot_reader(void *arg) {
  Snapshot *s = arg;
  do {
    int n = 77;
    int e = ta_snapshot_try(s->snapshot, 8, &n);
    CHECK(e == TA_SUCCESS || e == TA_LIMIT);
    if (e == TA_SUCCESS) {
      CHECK(n == 100);
      s->successes++;
    } else {
      CHECK(n == 77);
      s->limits++;
    }
  } while (!atomic_load(&s->stop));
  return NULL;
}
static void snapshot(void) {
  split_balance();
  TASnapshot s;
  ta_snapshot_init(&s);
  int n = 77;
  CHECK(ta_snapshot_try(&s, 1, &n) == TA_SUCCESS && n == 100);
  n = 77;
  atomic_store(&s.sequence, 1);
  CHECK(ta_snapshot_try(&s, 16, &n) == TA_LIMIT && n == 77);
  atomic_store(&s.sequence, 0);
  CHECK(ta_snapshot_try(&s, 0, &n) == TA_INVALID && n == 77);
  Snapshot run = {.snapshot = &s};
  atomic_init(&run.stop, 0);
  pthread_t thread;
  CHECK(pthread_create(&thread, NULL, snapshot_reader, &run) == 0);
  for (int i = 0; i < 10000; i++)
    CHECK(ta_snapshot_write(&s, i % 101) == TA_SUCCESS);
  atomic_store(&run.stop, 1);
  CHECK(pthread_join(thread, NULL) == 0);
  CHECK(run.successes + run.limits > 0);
  atomic_store(&s.sequence, UINT_MAX - 1u);
  CHECK(ta_snapshot_write(&s, 4) == TA_LIMIT);
  CHECK(ta_snapshot_write(&s, -1) == TA_INVALID);
  puts("OBS snapshot bounded16 attempts, output preservation,10000 writes, no "
       "sequence wrap");
}
typedef struct {
  TASlot *slot;
  unsigned id, rounds;
} Reader;
static void *reader(void *arg) {
  Reader *r = arg;
  for (unsigned i = 1; i <= r->rounds; i++) {
    int n = 0, e;
    do {
      e = ta_slot_read(r->slot, r->id, &n);
      CHECK(e == TA_SUCCESS || e == TA_LIMIT);
      if (e == TA_LIMIT)
        sched_yield();
    } while (e == TA_LIMIT);
    CHECK(n == (int)i);
  }
  return NULL;
}
static void handoff(void) {
  TASlot s;
  CHECK(ta_slot_init(&s, 0) == TA_INVALID);
  CHECK(ta_slot_init(&s, 2) == TA_SUCCESS);
  int n = 77;
  CHECK(ta_slot_read(&s, 2, &n) == TA_INVALID && n == 77);
  CHECK(ta_slot_read(&s, 0, &n) == TA_LIMIT && n == 77);
  CHECK(ta_slot_publish(&s, 42) == TA_SUCCESS);
  CHECK(ta_slot_read(&s, 0, &n) == TA_SUCCESS && n == 42);
  CHECK(ta_slot_publish(&s, 99) == TA_LIMIT);
  CHECK(ta_slot_read(&s, 1, &n) == TA_SUCCESS && n == 42);
  CHECK(ta_slot_publish(&s, 99) == TA_SUCCESS);
  CHECK(ta_slot_read(&s, 0, &n) == TA_SUCCESS && n == 99);
  CHECK(ta_slot_read(&s, 1, &n) == TA_SUCCESS && n == 99);
  for (unsigned readers = 1; readers <= 2; readers++) {
    CHECK(ta_slot_init(&s, readers) == TA_SUCCESS);
    Reader r[2];
    pthread_t t[2];
    for (unsigned i = 0; i < readers; i++) {
      r[i] = (Reader){.slot = &s, .id = i, .rounds = 2000};
      CHECK(pthread_create(&t[i], NULL, reader, &r[i]) == 0);
    }
    for (int value = 1; value <= 2000; value++) {
      int e;
      do {
        e = ta_slot_publish(&s, value);
        CHECK(e == TA_SUCCESS || e == TA_LIMIT);
        if (e == TA_LIMIT)
          sched_yield();
      } while (e == TA_LIMIT);
    }
    for (unsigned i = 0; i < readers; i++)
      CHECK(pthread_join(t[i], NULL) == 0);
    CHECK(atomic_load(&s.pending) == 0);
  }
  puts("OBS handoff2000 generations each one/two-reader; final reader release "
       "gates reuse");
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  if (!strcmp(argv[1], "graph"))
    graph();
  else if (!strcmp(argv[1], "conflict"))
    conflict();
  else if (!strcmp(argv[1], "snapshot"))
    snapshot();
  else if (!strcmp(argv[1], "handoff"))
    handoff();
  else
    return 2;
  return 0;
}
