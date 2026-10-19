#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "waiters.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "REACTOR-WAIT-CHECK:%s:%d\n", #x, __LINE__);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  RdPort *port;
  unsigned producer;
  RdWait outcome;
  struct timespec end;
  int result;
  pthread_barrier_t *gate;
} Attempt;
static struct timespec deadline(int seconds) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t))
    return (struct timespec){0};
  t.tv_sec += seconds;
  return t;
}
static void *waiter(void *ctx) {
  Attempt *a = ctx;
  if (a->gate)
    pthread_barrier_wait(a->gate);
  a->result = rd_port_wait(a->port, a->producer, &a->outcome, &a->end);
  return 0;
}
static void *stopper(void *ctx) {
  Attempt *a = ctx;
  pthread_barrier_wait(a->gate);
  a->result = rd_port_stop(a->port);
  return 0;
}
static int blocked(unsigned producer, unsigned peer) {
  RdPort p;
  CHECK(!rd_port_init(&p, producer, 7));
  Attempt a = {.port = &p,
               .producer = producer,
               .outcome = {.value = 9},
               .end = deadline(3)};
  pthread_t t;
  CHECK(!pthread_create(&t, 0, waiter, &a));
  struct timespec guard = deadline(3);
  CHECK(!rd_port_enrolled(&p, 1, &guard));
  /* Main can obtain the mutex after this enrollment only once the waiter has
   * atomically released it in cond_wait. A clock delay is not the evidence. */
  if (peer) {
    int s[2];
    unsigned char byte;
    CHECK(!socketpair(AF_UNIX, SOCK_STREAM, 0, s) && !close(s[1]) &&
          read(s[0], &byte, 1) == 0 && !close(s[0]) && !rd_port_peer_gone(&p));
  } else
    CHECK(!rd_port_stop(&p) && !rd_port_stop(&p));
  CHECK(!pthread_join(t, 0) && !a.result &&
        a.outcome.result == (peer ? RD_WAIT_PEER : RD_WAIT_STOP) &&
        p.queued == producer && p.value == 7 && p.waiters == 0);
  CHECK(!rd_port_destroy(&p));
  return 0;
}
static int spurious(void) {
  RdPort p;
  CHECK(!rd_port_init(&p, 0, 7));
  Attempt a = {.port = &p, .end = deadline(3)};
  pthread_t t;
  CHECK(!pthread_create(&t, 0, waiter, &a));
  struct timespec guard = deadline(3);
  CHECK(!rd_port_enrolled(&p, 1, &guard) && !rd_port_notify(&p));
  /* This is a deliberate application notification without predicate change,
   * not a claim that the kernel happened to generate a spurious wake. */
  CHECK(!rd_port_enrolled(&p, 2, &guard));
  CHECK(!pthread_mutex_lock(&p.mutex));
  CHECK(a.outcome.result == RD_WAIT_PENDING && p.waiters == 1 && !p.queued);
  CHECK(!pthread_mutex_unlock(&p.mutex));
  RdWait send = {.value = 42};
  CHECK(!rd_port_wait(&p, 1, &send, &guard) && send.result == RD_WAIT_SUCCESS);
  CHECK(!pthread_join(t, 0) && !a.result &&
        a.outcome.result == RD_WAIT_SUCCESS && a.outcome.value == 42 &&
        !p.queued && !p.waiters && !rd_port_destroy(&p));
  return 0;
}
static int races(unsigned *stops, unsigned *timeouts) {
  *stops = *timeouts = 0;
  for (unsigned i = 0; i < 200; ++i) {
    RdPort p;
    CHECK(!rd_port_init(&p, 0, 0));
    pthread_barrier_t gate;
    CHECK(!pthread_barrier_init(&gate, 0, 3));
    Attempt a = {.port = &p, .end = deadline(-1), .gate = &gate},
            b = {.port = &p, .gate = &gate};
    pthread_t t, u;
    CHECK(!pthread_create(&t, 0, waiter, &a) &&
          !pthread_create(&u, 0, stopper, &b));
    pthread_barrier_wait(&gate);
    CHECK(!pthread_join(t, 0) && !pthread_join(u, 0) && !a.result && !b.result);
    CHECK(a.outcome.result == RD_WAIT_STOP ||
          a.outcome.result == RD_WAIT_TIMEOUT);
    if (a.outcome.result == RD_WAIT_STOP)
      ++*stops;
    else
      ++*timeouts;
    enum RdWaitResult winner = a.outcome.result;
    CHECK(!rd_port_stop(&p) && a.outcome.result == winner && p.waiters == 0 &&
          !p.queued && !pthread_barrier_destroy(&gate) && !rd_port_destroy(&p));
  }
  /* Force the timeout-first history separately; race scheduling need not
   * produce both outcomes in any finite run. */
  RdPort p;
  RdWait w = {0};
  struct timespec past = deadline(-1);
  CHECK(!rd_port_init(&p, 0, 0) && !rd_port_wait(&p, 0, &w, &past) &&
        w.result == RD_WAIT_TIMEOUT && !rd_port_stop(&p) &&
        w.result == RD_WAIT_TIMEOUT && !rd_port_destroy(&p));
  return 0;
}
int main(void) {
  unsigned stops, timeouts;
  CHECK(!blocked(0, 0) && !blocked(1, 0) && !blocked(0, 1) && !spurious() &&
        !races(&stops, &timeouts));
  printf("REACTOR-WAIT:enrolled_empty_full_peer=3;predicate_recheck=1;races="
         "200;stop=%u;timeout=%u;single_winner=1;\n",
         stops, timeouts);
  return 0;
}
