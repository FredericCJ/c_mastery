#include "cancellation.h"
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CANCEL-THREAD-CHECK:%d:%s\n", __LINE__, #x);            \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  pthread_mutex_t lock;
  AeOperation *operation;
  atomic_uint ready;
  unsigned reports, bad, yield, timeout;
} Control;
static void receiver(void *v, AeCode code, unsigned generation,
                     const unsigned char *p, unsigned n) {
  Control *c = v;
  ++c->reports;
  if ((code != AE_OK && code != AE_TIMEOUT) || generation != 1 || n != 4 ||
      p[0] != 1 || p[3] != 4)
    ++c->bad;
  c->timeout = code == AE_TIMEOUT;
  AeOperation *old = c->operation;
  c->operation = 0;
  free(old);
}
static void *complete(void *v) {
  Control *c = v;
  while (!atomic_load_explicit(&c->ready, memory_order_acquire)) {
  }
  if (c->yield)
    sched_yield();
  pthread_mutex_lock(&c->lock);
  if (c->operation && ae_operation_event(c->operation, 1, AE_COMPLETE) != AE_OK)
    ++c->bad;
  pthread_mutex_unlock(&c->lock);
  return 0;
}
int main(void) {
  unsigned timed = 0;
  for (unsigned i = 0; i < 256; ++i) {
    Control c = {.yield = i % 2};
    CHECK(!pthread_mutex_init(&c.lock, 0));
    c.operation = calloc(1, sizeof *c.operation);
    CHECK(c.operation);
    const unsigned char p[4] = {1, 2, 3, 4};
    unsigned generation;
    CHECK(ae_operation_start(c.operation, p, 4, receiver, &c, &generation) ==
          AE_OK);
    pthread_t t;
    CHECK(!pthread_create(&t, 0, complete, &c));
    atomic_store_explicit(&c.ready, 1, memory_order_release);
    if (!c.yield)
      sched_yield();
    pthread_mutex_lock(&c.lock);
    if (c.operation)
      CHECK(ae_operation_event(c.operation, 1, AE_DEADLINE) == AE_AGAIN);
    pthread_mutex_unlock(&c.lock);
    CHECK(!pthread_join(t, 0));
    CHECK(!c.operation && c.reports == 1 && !c.bad);
    timed += c.timeout;
    CHECK(!pthread_mutex_destroy(&c.lock));
  }
  printf("CANCEL-THREAD-PASS:races=256;timeout_winners=%u;\n", timed);
  return 0;
}
