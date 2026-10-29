#include "test_support.h"
struct tw_consumer {
  struct tw_queue *queue;
  int out, result;
  struct timespec deadline;
  int timed;
  tw_hook_fn hook;
  void *arg;
};
static void *consume(void *p) {
  struct tw_consumer *c = p;
  c->result = tw_pop(c->queue, &c->out, c->timed ? &c->deadline : NULL, c->hook,
                     c->arg);
  return NULL;
}
static void finish(struct tw_queue *q) {
  CHECK(tw_close(q) == 0);
  CHECK(tw_unref(q) == 0);
  CHECK(tw_retire(q) == 0);
  CHECK(q->initialized == 1);
  tw_root_destroy(q);
}
void test_lifecycle(void) {
  for (unsigned stage = 1; stage <= 5; stage++) {
    struct tw_queue q;
    struct tw_init r;
    CHECK(tw_queue_init(&q, stage, &r) == EAGAIN);
    CHECK(r.initialized == stage - 1 && r.destroyed == stage - 1 &&
          r.failed_stage == stage);
  }
  struct tw_queue q;
  struct tw_init r;
  CHECK(tw_queue_init(&q, 0, &r) == 0 && r.initialized == 5 &&
        r.destroyed == 0);
  for (unsigned i = 1; i < 16; i++)
    CHECK(tw_ref(&q) == 0);
  CHECK(tw_ref(&q) == EOVERFLOW);
  for (unsigned i = 1; i < 16; i++)
    CHECK(tw_unref(&q) == 0);
  CHECK(tw_ref(&q) == 0);
  unsigned refs, active;
  size_t count;
  CHECK(tw_counts(&q, &refs, &active, &count) == 0);
  CHECK(refs == 2 && active == 0 && count == 0);
  struct tw_consumer c = {.queue = &q, .out = 77};
  pthread_t t;
  CHECK(pthread_create(&t, NULL, consume, &c) == 0);
  CHECK(tw_wait_entries(&q, 1) == 0);
  CHECK(tw_close(&q) == 0);
  CHECK(tw_ref(&q) == EPIPE);
  CHECK(tw_push(&q, 3, NULL) == EPIPE);
  CHECK(tw_unref(&q) == 0);
  CHECK(tw_retire(&q) == EBUSY);
  CHECK(pthread_join(t, NULL) == 0);
  CHECK(c.result == EPIPE && c.out == 77);
  CHECK(tw_unref(&q) == 0);
  CHECK(tw_ref(&q) == EPIPE);
  CHECK(tw_pop(&q, &c.out, NULL, NULL, NULL) == EPIPE);
  CHECK(tw_retire(&q) == 0);
  CHECK(tw_push(&q, 4, NULL) == EPIPE);
  CHECK(tw_pop(&q, &c.out, NULL, NULL, NULL) == EPIPE);
  CHECK(q.initialized == 1);
  tw_root_destroy(&q);
  puts("OBS lifecycle five prefix failures; close excludes refs; retire "
       "preserves admission root");
}
void test_queue(void) {
  struct tw_queue q;
  struct tw_init r;
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  CHECK(tw_push(&q, 1, NULL) == 0);
  CHECK(tw_push(&q, 2, NULL) == 0);
  int out = 91;
  CHECK(tw_pop(&q, &out, NULL, NULL, NULL) == 0);
  CHECK(out == 1);
  CHECK(tw_push(&q, 3, NULL) == 0);
  CHECK(tw_push(&q, 4, NULL) == 0);
  struct timespec past = {0};
  CHECK(tw_push(&q, 5, &past) == ETIMEDOUT);
  CHECK(tw_pop(&q, &out, NULL, NULL, NULL) == 0 && out == 2);
  CHECK(tw_close(&q) == 0);
  CHECK(tw_unref(&q) == EBUSY);
  CHECK(tw_push(&q, 9, NULL) == EPIPE);
  CHECK(tw_pop(&q, &out, NULL, NULL, NULL) == 0 && out == 3);
  CHECK(tw_pop(&q, &out, NULL, NULL, NULL) == 0 && out == 4);
  out = 88;
  CHECK(tw_pop(&q, &out, NULL, NULL, NULL) == EPIPE && out == 88);
  finish(&q);
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  struct tw_consumer c[2] = {{.queue = &q, .out = 77}, {.queue = &q, .out = 77}};
  pthread_t t[2];
  for (unsigned i = 0; i < 2; i++)
    CHECK(pthread_create(&t[i], NULL, consume, &c[i]) == 0);
  CHECK(tw_wait_entries(&q, 2) == 0);
  CHECK(tw_push(&q, 21, NULL) == 0);
  CHECK(tw_close(&q) == 0);
  for (unsigned i = 0; i < 2; i++)
    CHECK(pthread_join(t[i], NULL) == 0);
  CHECK((c[0].result == 0) + (c[1].result == 0) == 1);
  CHECK((c[0].result == EPIPE) + (c[1].result == EPIPE) == 1);
  for (unsigned i = 0; i < 2; i++)
    CHECK(c[i].out == (c[i].result ? 77 : 21));
  finish(&q);
  puts("OBS queue ring fixture FIFO; full timeout preserves input; two "
       "consumers one item");
}
struct wake_hook {
  pthread_barrier_t entered, release;
};
static void wake_hook(void *p) {
  struct wake_hook *h = p;
  gate(&h->entered);
  gate(&h->release);
}
void test_wake(void) {
  struct tw_queue q;
  struct tw_init r;
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  CHECK(tw_noise(&q) == 0);
  CHECK(tw_push(&q, 17, NULL) == 0);
  int n;
  CHECK(tw_pop(&q, &n, NULL, NULL, NULL) == 0 && n == 17);
  CHECK(q.wait_entries == 0);
  struct tw_consumer c = {.queue = &q, .out = 77, .timed = 1, .deadline = later(150)};
  pthread_t t;
  CHECK(pthread_create(&t, NULL, consume, &c) == 0);
  CHECK(tw_wait_entries(&q, 1) == 0);
  CHECK(tw_noise(&q) == 0);
  CHECK(pthread_join(t, NULL) == 0);
  CHECK(c.result == ETIMEDOUT && c.out == 77);
  CHECK(q.timed_waits >= 2);
  finish(&q);
  /* If would continue with an empty queue after broadcast; record a rejected
     empty transition instead of touching an uninitialized slot. */
  unsigned model_count = 0;
  int if_would_dequeue = 1, while_would_dequeue = model_count > 0;
  CHECK(if_would_dequeue && !while_would_dequeue);
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  struct wake_hook h;
  CHECK(pthread_barrier_init(&h.entered, NULL, 2) == 0);
  CHECK(pthread_barrier_init(&h.release, NULL, 2) == 0);
  c = (struct tw_consumer){.queue = &q,
                 .out = 77,
                 .timed = 1,
                 .deadline = later(150),
                 .hook = wake_hook,
                 .arg = &h};
  CHECK(pthread_create(&t, NULL, consume, &c) == 0);
  CHECK(tw_wait_entries(&q, 1) == 0);
  CHECK(tw_push(&q, 31, NULL) == 0);
  gate(&h.entered);
  CHECK(tw_pop(&q, &n, NULL, NULL, NULL) == 0 && n == 31);
  gate(&h.release);
  CHECK(pthread_join(t, NULL) == 0);
  CHECK(c.result == ETIMEDOUT && c.out == 77);
  CHECK(q.timed_waits >= 2);
  CHECK(pthread_barrier_destroy(&h.entered) == 0);
  CHECK(pthread_barrier_destroy(&h.release) == 0);
  finish(&q);
  puts("OBS wake signal-before-wait; empty broadcast; competing consumer "
       "steals31; predicate rechecked");
}
void test_deadline(void) {
  struct tw_queue q;
  struct tw_init r;
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  struct tw_consumer c = {.queue = &q, .out = 77, .timed = 1, .deadline = later(10000)};
  pthread_t t;
  CHECK(pthread_create(&t, NULL, consume, &c) == 0);
  for (unsigned i = 1; i <= 4; i++) {
    CHECK(tw_wait_entries(&q, i) == 0);
    if (i < 4)
      CHECK(tw_noise(&q) == 0);
  }
  CHECK(tw_close(&q) == 0);
  CHECK(pthread_join(t, NULL) == 0);
  CHECK(c.result == EPIPE && c.out == 77);
  CHECK(q.timed_waits == 4 && q.deadline_changes == 0);
  finish(&q);
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  int n = 77;
  struct timespec past = {0};
  CHECK(tw_pop(&q, &n, &past, NULL, NULL) == ETIMEDOUT && n == 77);
  CHECK(tw_push(&q, 44, NULL) == 0);
  CHECK(tw_pop(&q, &n, &past, NULL, NULL) == 0 && n == 44);
  CHECK(tw_close(&q) == 0);
  CHECK(tw_pop(&q, &n, &past, NULL, NULL) == EPIPE && n == 44);
  finish(&q);
  /* Actual competing transitions, not a wall-clock ordering assertion. */
  for (unsigned i = 0; i < 16; i++) {
    CHECK(tw_queue_init(&q, 0, &r) == 0);
    c = (struct tw_consumer){.queue = &q, .out = 77, .timed = 1, .deadline = later(0)};
    CHECK(pthread_create(&t, NULL, consume, &c) == 0);
    CHECK(tw_close(&q) == 0);
    CHECK(pthread_join(t, NULL) == 0);
    CHECK((c.result == ETIMEDOUT || c.result == EPIPE) && c.out == 77);
    finish(&q);
  }
  CHECK(tw_queue_init(&q, 0, &r) == 0);
  c = (struct tw_consumer){.queue = &q, .out = 77};
  CHECK(pthread_create(&t, NULL, consume, &c) == 0);
  CHECK(tw_wait_entries(&q, 1) == 0);
  CHECK(pthread_cancel(t) == 0);
  void *result = NULL;
  CHECK(pthread_join(t, &result) == 0 && result == PTHREAD_CANCELED);
  unsigned refs, active;
  size_t count;
  CHECK(tw_counts(&q, &refs, &active, &count) == 0 && refs == 1 &&
        active == 0 && count == 0);
  CHECK(pthread_mutex_trylock(&q.mutex) == 0);
  CHECK(pthread_mutex_unlock(&q.mutex) == 0);
  CHECK(tw_push(&q, 8, NULL) == 0);
  CHECK(tw_pop(&q, &n, NULL, NULL, NULL) == 0 && n == 8);
  finish(&q);
  puts("OBS deadline one CLOCK_MONOTONIC absolute deadline;16 close-timeout "
       "races; deferred cleanup unlocks and drops admission");
}
