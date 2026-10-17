#include "messaging.h"
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SC_CHECK:%s:%d: %s\n", __func__, __LINE__, #x);         \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int task(void *p, const atomic_bool *cancel) {
  (void)cancel;
  return *(int *)p;
}
static int nursery(void) {
  MsgNursery n;
  int fail = 5, ok = 0, result = 99;
  msg_nursery_init(&n, 0);
  CHECK(msg_spawn(&n, task, &fail) == MSG_OK);
  CHECK(msg_spawn(&n, task, &ok) == MSG_OK);
  CHECK(msg_join(&n, &result) == MSG_OK && result == 5);
  CHECK(atomic_load(&n.cancel));
  CHECK(n.closed && n.child[0].joined && n.child[1].joined);
  CHECK(msg_spawn(&n, task, &ok) == MSG_CLOSED);
  msg_nursery_init(&n, 2);
  CHECK(msg_spawn(&n, task, &ok) == MSG_OK);
  CHECK(msg_spawn(&n, task, &ok) == MSG_FAILED);
  CHECK(msg_join(&n, &result) == MSG_OK && result == 0 && n.count == 1 &&
        atomic_load(&n.cancel));
  return 0;
}
typedef struct {
  MsgHub *hub;
  MsgStatus result;
} Rendezvous;
static void *offer(void *p) {
  Rendezvous *r = p;
  r->result = msg_send(r->hub, 0, 42, true);
  return NULL;
}
static int channels(void) {
  MsgHub h;
  unsigned k = 99, v = 99;
  CHECK(msg_hub_init(&h, 2, 2) == MSG_OK);
  CHECK(msg_send(&h, 0, 7, false) == MSG_OK);
  CHECK(msg_close(&h, 0) == MSG_OK);
  CHECK(msg_send(&h, 0, 8, false) == MSG_CLOSED);
  CHECK(msg_select(&h, 1, false, &k, &v) == MSG_OK && k == 0 && v == 7);
  CHECK(msg_select(&h, 1, false, &k, &v) == MSG_CLOSED);
  CHECK(msg_select(&h, 2, false, &k, &v) == MSG_EMPTY);
  CHECK(msg_send(&h, 1, 8, false) == MSG_OK);
  CHECK(msg_send(&h, 1, 9, false) == MSG_OK);
  CHECK(msg_send(&h, 1, 10, false) == MSG_FULL);
  CHECK(msg_select(&h, 2, false, &k, &v) == MSG_OK && v == 8);
  msg_hub_cancel(&h);
  CHECK(msg_select(&h, 2, false, &k, &v) == MSG_CANCELLED);
  msg_hub_destroy(&h);
  CHECK(msg_hub_init(&h, 0, 1) == MSG_OK);
  Rendezvous r = {&h, MSG_INVALID};
  pthread_t t;
  CHECK(pthread_create(&t, NULL, offer, &r) == 0);
  CHECK(msg_select(&h, 1, true, &k, &v) == MSG_OK && v == 42);
  CHECK(pthread_join(t, NULL) == 0 && r.result == MSG_OK);
  msg_hub_destroy(&h);
  return 0;
}
typedef struct {
  unsigned calls, value;
  MsgStatus status;
} Received;
static void receive(void *p, MsgStatus status, unsigned v) {
  Received *r = p;
  ++r->calls;
  r->status = status;
  r->value = v;
}
static void release_operation(void *p, MsgStatus status, unsigned v) {
  (void)status;
  (void)v;
  free(p);
}
static int operations(void) {
  MsgOperation o;
  Received r = {0};
  msg_connect(&o, receive, &r);
  CHECK(msg_complete(&o, MSG_OK, 1) == MSG_INVALID);
  CHECK(msg_start(&o) == MSG_OK);
  CHECK(msg_start(&o) == MSG_INVALID);
  CHECK(msg_stop_operation(&o) == MSG_OK && o.state == MSG_STARTED);
  CHECK(msg_complete(&o, MSG_CANCELLED, 3) == MSG_OK && r.calls == 1 &&
        r.value == 3);
  CHECK(msg_complete(&o, MSG_OK, 5) == MSG_INVALID && r.calls == 1);
  MsgOperation *heap = malloc(sizeof *heap);
  CHECK(heap != NULL);
  msg_connect(heap, release_operation, heap);
  CHECK(msg_start(heap) == MSG_OK);
  CHECK(msg_complete(heap, MSG_OK, 1) == MSG_OK);
  return 0;
}
static bool handle(void *p, unsigned v) {
  unsigned *sum = p;
  if (v == 99)
    return false;
  *sum += v;
  return true;
}
static int actors(void) {
  MsgActor a;
  unsigned sum = 0;
  msg_actor_init(&a, handle, &sum);
  CHECK(msg_actor_send(&a, 0, 1) == MSG_STALE);
  CHECK(msg_actor_send(&a, 1, 2) == MSG_OK);
  CHECK(msg_actor_send(&a, 1, 99) == MSG_OK);
  CHECK(msg_actor_send(&a, 1, 3) == MSG_OK);
  CHECK(msg_actor_step(&a) == MSG_OK && sum == 2);
  CHECK(msg_actor_step(&a) == MSG_FAILED && a.count == 0);
  CHECK(msg_actor_restart(&a) == MSG_OK && a.generation == 2);
  CHECK(msg_actor_send(&a, 1, 5) == MSG_STALE);
  CHECK(msg_actor_step(&a) == MSG_EMPTY);
  CHECK(msg_actor_restart(&a) == MSG_BUSY);
  return 0;
}
static int supervision(void) {
  MsgSupervisor s;
  int a[8] = {0};
  size_t n = 0;
  CHECK(msg_supervisor_init(&s, 2, 10) == MSG_OK);
  CHECK(msg_restart_plan(&s, 0, MSG_REST, 1, a, &n) == MSG_OK && n == 6);
  const int expected[] = {-4, -3, -2, 2, 3, 4};
  CHECK(memcmp(a, expected, sizeof expected) == 0);
  CHECK(msg_restart_plan(&s, 1, MSG_ONE, 2, a, &n) == MSG_OK && n == 2 &&
        a[0] == -3 && a[1] == 3);
  CHECK(msg_restart_plan(&s, 2, MSG_ALL, 0, a, &n) == MSG_FAILED && n == 4 &&
        s.escalated);
  CHECK(msg_restart_plan(&s, 20, MSG_ONE, 0, a, &n) == MSG_CLOSED);
  CHECK(msg_supervisor_init(&s, 1, 10) == MSG_OK);
  CHECK(msg_restart_plan(&s, 1, MSG_ONE, 0, a, &n) == MSG_OK);
  CHECK(msg_restart_plan(&s, 11, MSG_ONE, 0, a, &n) == MSG_OK);
  CHECK(msg_restart_plan(&s, 10, MSG_ONE, 0, a, &n) == MSG_INVALID);
  return 0;
}
static int stealing(void) {
  MsgDeque q;
  unsigned v = 99;
  msg_deque_init(&q);
  CHECK(msg_pop(&q, &v) == MSG_EMPTY && v == 99);
  CHECK(msg_push(&q, 7) == MSG_OK);
  CHECK(msg_push(&q, 8) == MSG_OK);
  CHECK(msg_pop(&q, &v) == MSG_OK && v == 8);
  CHECK(msg_steal(&q, &v) == MSG_OK && v == 7);
  CHECK(msg_steal(&q, &v) == MSG_EMPTY);
  for (unsigned i = 0; i < 8; i++)
    CHECK(msg_push(&q, i) == MSG_OK);
  CHECK(msg_push(&q, 9) == MSG_FULL);
  for (unsigned i = 0; i < 8; i++)
    CHECK(msg_steal(&q, &v) == MSG_OK && v == i);
  atomic_store(&q.top, SIZE_MAX - 1);
  atomic_store(&q.bottom, SIZE_MAX - 1);
  CHECK(msg_push(&q, 1) == MSG_OVERFLOW);
  return 0;
}
static int transactions(void) {
  MsgStore s;
  MsgTxn a, b;
  unsigned v = 99;
  msg_store_init(&s);
  msg_begin(&s, &a);
  msg_begin(&s, &b);
  CHECK(msg_write(&b, 0, 7) == MSG_OK && msg_commit(&b) == MSG_OK);
  CHECK(msg_read(&a, 0, &v) == MSG_CONFLICT && v == 99);
  msg_begin(&s, &a);
  CHECK(msg_read(&a, 0, &v) == MSG_OK && v == 7);
  msg_begin(&s, &b);
  CHECK(msg_write(&b, 0, 8) == MSG_OK && msg_commit(&b) == MSG_OK);
  CHECK(msg_write(&a, 1, 9) == MSG_OK);
  CHECK(msg_commit(&a) == MSG_CONFLICT && atomic_load(&s.cells[1].value) == 0 &&
        !(atomic_load(&s.cells[1].stamp) & 1));
  msg_begin(&s, &a);
  CHECK(msg_write(&a, 2, 12) == MSG_OK && msg_read(&a, 2, &v) == MSG_OK &&
        v == 12 && msg_commit(&a) == MSG_OK);
  atomic_store(&s.clock, UINT_MAX / 2);
  msg_begin(&s, &a);
  CHECK(msg_write(&a, 3, 4) == MSG_OK && msg_commit(&a) == MSG_OVERFLOW);
  CHECK(atomic_load(&s.cells[3].value) == 0 &&
        atomic_load(&s.cells[3].stamp) == 0);
  return 0;
}
static int snapshots(void) {
  MsgVersions s;
  MsgSnapshot a, b;
  unsigned v[2] = {99, 99};
  CHECK(msg_versions_init(&s) == MSG_OK);
  CHECK(msg_snapshot(&s, &a) == MSG_OK);
  CHECK(msg_publish(&s, 1, 2) == MSG_OK);
  CHECK(msg_snapshot_read(&s, a, v) == MSG_OK && v[0] == 0 && v[1] == 0);
  CHECK(msg_publish(&s, 3, 4) == MSG_OK);
  CHECK(msg_snapshot_read(&s, a, v) == MSG_OK && v[0] == 0);
  CHECK(msg_snapshot_drop(&s, a) == MSG_OK);
  CHECK(msg_snapshot(&s, &b) == MSG_OK);
  CHECK(msg_snapshot_read(&s, a, v) == MSG_STALE);
  CHECK(msg_snapshot_read(&s, b, v) == MSG_OK && v[0] == 3 && v[1] == 4);
  CHECK(msg_snapshot_drop(&s, b) == MSG_OK);
  msg_versions_destroy(&s);
  return 0;
}
static int sequences(void) {
  MsgSequence s;
  unsigned v[2] = {99, 99};
  msg_sequence_init(&s);
  atomic_store(&s.sequence, 1);
  CHECK(msg_sequence_read(&s, 3, v) == MSG_BUSY && v[0] == 99 && v[1] == 99);
  atomic_store(&s.sequence, 0);
  CHECK(msg_sequence_write(&s, 7, 14) == MSG_OK);
  CHECK(msg_sequence_read(&s, 1, v) == MSG_OK && v[0] == 7 && v[1] == 14);
  atomic_store(&s.sequence, UINT_MAX - 1);
  CHECK(msg_sequence_write(&s, 9, 18) == MSG_OVERFLOW);
  return 0;
}
typedef struct {
  unsigned count, last, ends;
  MsgStatus end;
  MsgStream *stream;
  bool nested_end;
} Sink;
static void next(void *p, unsigned v) {
  Sink *s = p;
  ++s->count;
  s->last = v;
  if (v == 99)
    (void)msg_request(s->stream, 1);
  if (v == 100 || v == 101) {
    (void)msg_request(s->stream, 0);
    if (v == 101)
      msg_stream_cancel(s->stream);
  }
}
static void terminal(void *p, MsgStatus status) {
  Sink *s = p;
  if (s->stream->delivering)
    s->nested_end = true;
  ++s->ends;
  s->end = status;
}
static int streams(void) {
  MsgStream s;
  Sink sink = {.stream = &s};
  msg_stream_init(&s, next, terminal, &sink);
  CHECK(msg_emit(&s, 1) == MSG_EMPTY && sink.count == 0);
  CHECK(msg_request(&s, 1) == MSG_OK);
  CHECK(msg_emit(&s, 99) == MSG_OK && s.demand == 1);
  CHECK(msg_emit(&s, 7) == MSG_OK && sink.count == 2 && s.demand == 0);
  CHECK(msg_request(&s, UINT64_MAX) == MSG_OK && msg_request(&s, 1) == MSG_OK &&
        s.demand == UINT64_MAX);
  CHECK(msg_stream_end(&s, MSG_OK) == MSG_OK && sink.ends == 1);
  CHECK(msg_stream_end(&s, MSG_FAILED) == MSG_CLOSED && sink.ends == 1);
  CHECK(msg_emit(&s, 8) == MSG_CLOSED);
  msg_stream_init(&s, next, terminal, &sink);
  CHECK(msg_request(&s, 0) == MSG_OK && s.terminal && sink.end == MSG_INVALID);
  msg_stream_init(&s, next, terminal, &sink);
  CHECK(msg_request(&s, 1) == MSG_OK);
  CHECK(msg_emit(&s, 100) == MSG_OK && s.terminal && sink.ends == 3 &&
        !sink.nested_end);
  msg_stream_init(&s, next, terminal, &sink);
  CHECK(msg_request(&s, 1) == MSG_OK);
  CHECK(msg_emit(&s, 101) == MSG_OK && s.cancelled && sink.ends == 3);
  return 0;
}
typedef struct {
  MsgDeque q;
  atomic_bool done;
  atomic_uint seen[4000];
} RaceDeque;
static void *thief(void *p) {
  RaceDeque *r = p;
  unsigned v;
  for (;;) {
    MsgStatus s = msg_steal(&r->q, &v);
    if (s == MSG_OK) {
      if (v < 4000)
        atomic_fetch_add(&r->seen[v], 1);
      else
        abort();
    } else if (atomic_load(&r->done) && s == MSG_EMPTY)
      break;
    else
      sched_yield();
  }
  return NULL;
}
typedef struct {
  MsgSequence sequence;
  atomic_bool done, bad;
} RaceSeq;
static void *seqwriter(void *p) {
  RaceSeq *r = p;
  for (unsigned i = 1; i <= 10000; i++)
    (void)msg_sequence_write(&r->sequence, i, 2 * i);
  atomic_store(&r->done, true);
  return NULL;
}
static void *seqreader(void *p) {
  RaceSeq *r = p;
  unsigned v[2];
  do {
    if (msg_sequence_read(&r->sequence, 3, v) == MSG_OK && v[1] != 2 * v[0])
      atomic_store(&r->bad, true);
  } while (!atomic_load(&r->done));
  return NULL;
}
typedef struct {
  MsgStore store;
  atomic_bool bad;
} RaceTxn;
static void *transactor(void *p) {
  RaceTxn *r = p;
  for (unsigned i = 0; i < 1000;) {
    MsgTxn t;
    unsigned a, b;
    msg_begin(&r->store, &t);
    if (msg_read(&t, 0, &a) != MSG_OK || msg_read(&t, 1, &b) != MSG_OK)
      continue;
    if (a != b)
      atomic_store(&r->bad, true);
    (void)msg_write(&t, 0, a + 1);
    (void)msg_write(&t, 1, b + 1);
    if (msg_commit(&t) == MSG_OK)
      i++;
  }
  return NULL;
}
static int threads(void) {
  RaceDeque r;
  msg_deque_init(&r.q);
  atomic_init(&r.done, false);
  for (unsigned i = 0; i < 4000; i++)
    atomic_init(&r.seen[i], 0);
  pthread_t a, b;
  CHECK(pthread_create(&a, NULL, thief, &r) == 0);
  CHECK(pthread_create(&b, NULL, thief, &r) == 0);
  for (unsigned i = 0; i < 4000; i++) {
    while (msg_push(&r.q, i) == MSG_FULL)
      sched_yield();
    if (i % 3 == 0) {
      unsigned v;
      MsgStatus s = msg_pop(&r.q, &v);
      if (s == MSG_OK) {
        CHECK(v < 4000);
        atomic_fetch_add(&r.seen[v], 1);
      }
    }
  }
  atomic_store(&r.done, true);
  CHECK(pthread_join(a, NULL) == 0 && pthread_join(b, NULL) == 0);
  for (unsigned i = 0; i < 4000; i++)
    CHECK(atomic_load(&r.seen[i]) == 1);
  RaceSeq sq;
  msg_sequence_init(&sq.sequence);
  atomic_init(&sq.done, false);
  atomic_init(&sq.bad, false);
  CHECK(pthread_create(&a, NULL, seqwriter, &sq) == 0 &&
        pthread_create(&b, NULL, seqreader, &sq) == 0);
  CHECK(pthread_join(a, NULL) == 0 && pthread_join(b, NULL) == 0 &&
        !atomic_load(&sq.bad));
  RaceTxn tx;
  msg_store_init(&tx.store);
  atomic_init(&tx.bad, false);
  CHECK(pthread_create(&a, NULL, transactor, &tx) == 0 &&
        pthread_create(&b, NULL, transactor, &tx) == 0);
  CHECK(pthread_join(a, NULL) == 0 && pthread_join(b, NULL) == 0 &&
        !atomic_load(&tx.bad));
  CHECK(atomic_load(&tx.store.cells[0].value) == 2000 &&
        atomic_load(&tx.store.cells[1].value) == 2000);
  printf("thread cases: deque4000 exactly once, sequence10000 coherent writes, "
         "STM2000 commits; atomics lock-free size=%d uint=%d\n",
         atomic_is_lock_free(&r.q.top),
         atomic_is_lock_free(&sq.sequence.first));
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  const char *names[] = {"nursery",     "channels", "operations",   "actors",
                         "supervision", "stealing", "transactions", "snapshots",
                         "sequences",   "streams",  "threads"};
  int (*cases[])(void) = {nursery,     channels, operations,   actors,
                          supervision, stealing, transactions, snapshots,
                          sequences,   streams,  threads};
  for (size_t i = 0; i < sizeof cases / sizeof *cases; i++)
    if (!strcmp(argv[1], names[i]))
      return cases[i]();
  return 2;
}
