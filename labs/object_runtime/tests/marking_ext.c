#include "object_runtime_concurrent.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static const ORCHandle nil = {0, 0};
static int drain(ORCHeap *h, uint16_t *dead) {
  unsigned scanned;
  ORCFinish token;
  ORStatus s;
  do {
    s = orcm_step(h, 1, &scanned);
    CHECK((s == OR_OK || s == OR_DONE) && scanned <= 1);
  } while (s != OR_DONE);
  CHECK(orcm_prepare(h, &token) == OR_OK);
  CHECK(orcm_commit(h, token, dead) == OR_OK);
  return 0;
}
static int allocation(void) {
  unsigned policy, i, scanned;
  uint16_t dead;
  ORCHeap h, before;
  ORCHandle a, b, n, out;
  for (policy = 0; policy < 3; policy++) {
    orcm_init(&h, (ORCPolicy)policy);
    CHECK(orcm_allocate(&h, nil, nil, &a) == OR_OK);
    CHECK(orcm_allocate(&h, nil, nil, &b) == OR_OK);
    CHECK(orcm_release(&h, b) == OR_OK);
    CHECK(orcm_begin(&h) == OR_OK);
    CHECK(orcm_step(&h, 0, &scanned) == OR_OK && scanned == 0);
    CHECK(orcm_step(&h, 1, &scanned) == OR_DONE && scanned == 1);
    CHECK(orcm_allocate(&h, b, nil, &n) == OR_OK && orcm_invariant(&h));
    CHECK(drain(&h, &dead) == 0 && dead == 0 && h.graph.live == 7 &&
          orcm_invariant(&h));
    CHECK(h.scans == (policy == ORC_BLACK ? 2u : policy == ORC_GRAY ? 3u : 4u));
    CHECK(h.restarts == (policy == ORC_RESTART));
    CHECK(orcm_release(&h, a) == OR_OK && orcm_release(&h, n) == OR_OK);
    CHECK(orcm_begin(&h) == OR_OK && drain(&h, &dead) == 0 && dead == 7);
    out = (ORCHandle){99, 99};
    CHECK(orcm_allocate(&h, a, nil, &out) == OR_INVALID && out.slot == 99);
    CHECK(orcm_invariant(&h));
  }
  orcm_init(&h, ORC_GRAY);
  CHECK(orcm_allocate(&h, nil, nil, &a) == OR_OK);
  for (i = 1; i < 255; i++)
    CHECK(orcm_retain(&h, a) == OR_OK);
  memcpy(&before, &h, sizeof h);
  CHECK(orcm_retain(&h, a) == OR_FULL && !memcmp(&before, &h, sizeof h));
  CHECK(orcm_store(&h, a, 2, nil) == OR_INVALID &&
        !memcmp(&before, &h, sizeof h));
  for (i = 0; i < 255; i++)
    CHECK(orcm_release(&h, a) == OR_OK);
  memcpy(&before, &h, sizeof h);
  CHECK(orcm_release(&h, a) == OR_STATE && !memcmp(&before, &h, sizeof h));
  CHECK(orcm_begin(&h) == OR_OK && drain(&h, &dead) == 0 && dead == 1);
  orcm_init(&h, ORC_GRAY);
  for (i = 0; i < 48; i++) {
    CHECK(orcm_allocate(&h, nil, nil, &a) == OR_OK);
    CHECK(a.slot == i / 3 + 1 && a.generation == i % 3 + 1);
    CHECK(orcm_release(&h, a) == OR_OK);
    CHECK(orcm_begin(&h) == OR_OK && drain(&h, &dead) == 0);
  }
  memcpy(&before, &h, sizeof h);
  out = (ORCHandle){99, 99};
  CHECK(orcm_allocate(&h, nil, nil, &out) == OR_FULL && out.slot == 99 &&
        !memcmp(&before, &h, sizeof h));
  orcm_init(&h, ORC_BLACK);
  CHECK(orcm_allocate(&h, nil, nil, &a) == OR_OK);
  while (h.revision < ORC_REV_LIMIT)
    CHECK(orcm_store(&h, a, 0, nil) == OR_OK);
  memcpy(&before, &h, sizeof h);
  CHECK(orcm_store(&h, a, 0, nil) == OR_FULL && !memcmp(&before, &h, sizeof h));
  CHECK(orcm_invariant(&h));
  return 0;
}
typedef struct {
  ORCConcurrent *control;
  ORCRequest request;
  ORStatus status;
  unsigned completed;
} Writer;
static void *one_write(void *arg) {
  Writer *w = arg;
  ORCResponse r;
  w->status = orcm_concurrent_call(w->control, &w->request, &r);
  return NULL;
}
static ORStatus call(ORCConcurrent *c, ORCOperation op, ORCHandle object,
                     ORCHandle target, unsigned budget, ORCFinish finish,
                     ORCResponse *r) {
  ORCRequest q = {.operation = op,
                  .object = object,
                  .target = target,
                  .budget = budget,
                  .finish = finish};
  return orcm_concurrent_call(c, &q, r);
}
static int finish_concurrent(ORCConcurrent *c, uint16_t *dead) {
  ORCResponse r;
  ORStatus s;
  ORCFinish empty = {0};
  do {
    s = call(c, ORC_STEP, nil, nil, 1, empty, &r);
    CHECK(s == OR_OK || s == OR_DONE);
  } while (s != OR_DONE);
  CHECK(call(c, ORC_PREPARE, nil, nil, 0, empty, &r) == OR_OK);
  CHECK(call(c, ORC_COMMIT, nil, nil, 0, r.finish, &r) == OR_OK);
  *dead = r.dead;
  return 0;
}
static int race_case(int root) {
  ORCConcurrent c;
  ORCResponse r;
  ORCFinish token, empty = {0};
  ORCHandle a, b;
  Writer w;
  pthread_t thread;
  uint16_t dead;
  CHECK(orcm_concurrent_init(&c, ORC_BLACK) == OR_OK);
  CHECK(call(&c, ORC_ALLOCATE, nil, nil, 0, empty, &r) == OR_OK);
  a = r.object;
  CHECK(call(&c, ORC_ALLOCATE, nil, nil, 0, empty, &r) == OR_OK);
  b = r.object;
  CHECK(call(&c, ORC_RELEASE, b, nil, 0, empty, &r) == OR_OK);
  CHECK(call(&c, ORC_BEGIN, nil, nil, 0, empty, &r) == OR_OK);
  CHECK(call(&c, ORC_STEP, nil, nil, 1, empty, &r) == OR_DONE);
  CHECK(call(&c, ORC_PREPARE, nil, nil, 0, empty, &r) == OR_OK);
  token = r.finish;
  w = (Writer){.control = &c,
               .request = {.operation = root ? ORC_RETAIN : ORC_STORE,
                           .object = root ? b : a,
                           .target = b}};
  CHECK(pthread_create(&thread, NULL, one_write, &w) == 0);
  CHECK(pthread_join(thread, NULL) == 0);
  CHECK(w.status == OR_OK);
  CHECK(call(&c, ORC_COMMIT, nil, nil, 0, token, &r) == OR_STATE);
  CHECK(finish_concurrent(&c, &dead) == 0 && dead == 0);
  CHECK(call(&c, ORC_INSPECT, nil, nil, 0, empty, &r) == OR_OK && r.invariant &&
        r.live == 3);
  CHECK(call(&c, ORC_RELEASE, a, nil, 0, empty, &r) == OR_OK);
  if (root)
    CHECK(call(&c, ORC_RELEASE, b, nil, 0, empty, &r) == OR_OK);
  CHECK(call(&c, ORC_BEGIN, nil, nil, 0, empty, &r) == OR_OK &&
        finish_concurrent(&c, &dead) == 0 && dead == 3);
  CHECK(orcm_concurrent_close(&c) == OR_OK);
  return 0;
}
static void *mutator(void *arg) {
  Writer *w = arg;
  ORCResponse r;
  ORCFinish empty = {0};
  unsigned i;
  w->status = OR_OK;
  for (i = 0; i < 1000; i++) {
    ORCRequest q = w->request;
    q.target = i % 2 ? q.target : nil;
    if (orcm_concurrent_call(w->control, &q, &r) != OR_OK ||
        call(w->control, ORC_RETAIN, w->request.target, nil, 0, empty, &r) !=
            OR_OK ||
        call(w->control, ORC_RELEASE, w->request.target, nil, 0, empty, &r) !=
            OR_OK) {
      w->status = OR_STATE;
      return NULL;
    }
    w->completed++;
  }
  return NULL;
}
static int concurrent(void) {
  ORCConcurrent c;
  ORCResponse r;
  ORCHandle a, b;
  ORCFinish empty = {0};
  Writer w;
  pthread_t thread;
  ORStatus status = OR_OK;
  unsigned i;
  uint16_t dead;
  CHECK(race_case(0) == 0 && race_case(1) == 0);
  CHECK(orcm_concurrent_init(&c, ORC_GRAY) == OR_OK);
  CHECK(call(&c, ORC_ALLOCATE, nil, nil, 0, empty, &r) == OR_OK);
  a = r.object;
  CHECK(call(&c, ORC_ALLOCATE, nil, nil, 0, empty, &r) == OR_OK);
  b = r.object;
  w = (Writer){.control = &c,
               .request = {.operation = ORC_STORE, .object = a, .target = b}};
  CHECK(pthread_create(&thread, NULL, mutator, &w) == 0);
  for (i = 0; i < 100 && status == OR_OK; i++) {
    status = call(&c, ORC_BEGIN, nil, nil, 0, empty, &r);
    if (status != OR_OK)
      break;
    do {
      do {
        status = call(&c, ORC_STEP, nil, nil, 1, empty, &r);
      } while (status == OR_OK);
      if (status != OR_DONE)
        break;
      status = call(&c, ORC_PREPARE, nil, nil, 0, empty, &r);
      if (status != OR_OK)
        break;
      status = call(&c, ORC_COMMIT, nil, nil, 0, r.finish, &r);
    } while (status == OR_STATE);
  }
  CHECK(pthread_join(thread, NULL) == 0);
  CHECK(status == OR_OK && w.status == OR_OK && w.completed == 1000);
  CHECK(call(&c, ORC_INSPECT, nil, nil, 0, empty, &r) == OR_OK && r.invariant &&
        r.live == 3 && r.roots == 3);
  CHECK(orcm_concurrent_close(&c) == OR_STATE);
  CHECK(call(&c, ORC_RELEASE, a, nil, 0, empty, &r) == OR_OK &&
        call(&c, ORC_RELEASE, b, nil, 0, empty, &r) == OR_OK);
  CHECK(call(&c, ORC_BEGIN, nil, nil, 0, empty, &r) == OR_OK &&
        finish_concurrent(&c, &dead) == 0 && dead == 3);
  CHECK(orcm_concurrent_close(&c) == OR_OK);
  /* Revision exhaustion retires the entire stationary instance. There are no
     clients or retained handles when the owner performs this terminal close. */
  CHECK(orcm_concurrent_init(&c, ORC_BLACK) == OR_OK);
  CHECK(call(&c, ORC_ALLOCATE, nil, nil, 0, empty, &r) == OR_OK);
  a = r.object;
  for (i = 2; i < ORC_REV_LIMIT; i++)
    CHECK(call(&c, ORC_STORE, a, nil, 0, empty, &r) == OR_OK);
  CHECK(call(&c, ORC_INSPECT, nil, nil, 0, empty, &r) == OR_OK &&
        r.revision == ORC_REV_LIMIT && r.invariant && r.live == 1);
  {
    ORCResponse before;
    memcpy(&before, &r, sizeof r);
    CHECK(call(&c, ORC_RELEASE, a, nil, 0, empty, &r) == OR_FULL &&
          !memcmp(&before, &r, sizeof r));
  }
  a = nil;
  CHECK(orcm_concurrent_close(&c) == OR_OK);
  CHECK(call(&c, ORC_INSPECT, nil, nil, 0, empty, &r) == OR_INVALID);
  return 0;
}
static int oracle(void) {
  unsigned graph, mask, policy, i, scanned;
  for (graph = 0; graph < 64; graph++)
    for (mask = 0; mask < 8; mask++)
      for (policy = 0; policy < 3; policy++) {
        ORCHeap h;
        ORCHandle object[4];
        unsigned code = graph;
        uint16_t dead;
        orcm_init(&h, (ORCPolicy)policy);
        for (i = 0; i < 3; i++)
          CHECK(orcm_allocate(&h, nil, nil, &object[i]) == OR_OK);
        for (i = 0; i < 3; i++) {
          unsigned t = code % 4;
          code /= 4;
          CHECK(orcm_store(&h, object[i], 0, t ? object[t - 1] : nil) == OR_OK);
          if (!(mask & (1u << i)))
            CHECK(orcm_release(&h, object[i]) == OR_OK);
        }
        CHECK(orcm_begin(&h) == OR_OK);
        {
          ORStatus status = orcm_step(&h, 1, &scanned);
          CHECK(status == OR_OK || status == OR_DONE);
        }
        CHECK(orcm_allocate(&h, object[0], nil, &object[3]) == OR_OK);
        CHECK(drain(&h, &dead) == 0 && orcm_invariant(&h));
        printf("M %u %u %u %u %u\n", graph, mask, policy, h.graph.live, dead);
      }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "allocation"))
    return allocation();
  if (!strcmp(argv[1], "concurrent"))
    return concurrent();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  return 2;
}
