#include "condition.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "CDQ_CHECK:%d:%s\n", __LINE__, #c);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int setup(cmx_kernel *k, CDQueue *q, unsigned capacity,
                 unsigned broadcast) {
  cmx_init(k, CMX_FIXED);
  for (unsigned t = 0; t < 4; t++)
    if (cmx_task_add(k, 9 - t, 0, 0x1000 + 0x200 * t, 256) != (int)t)
      return 0;
  return cdq_init(q, k, capacity, broadcast) == CMX_OK;
}
static int ledger(const CDQueue *q) {
  return q->count <= q->capacity &&
         q->accepted == q->count + q->claimed + q->consumed;
}
static int collect(CDQueue *q, unsigned t, int expected, uint32_t value) {
  int result = -1;
  uint32_t out = 777;
  return cdq_result(q, t, &result, &out) == CMX_OK && result == expected &&
         (expected == CMX_OK ? out == value : out == 777);
}
static int mesa(void) {
  cmx_kernel k;
  CDQueue q;
  uint32_t value;
  CHECK(setup(&k, &q, 1, 1));
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_pulse(&q, &q.mutex, 3, 1) == CMX_OK);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK);
  CHECK(cdq_begin(&q, 0, CDQ_GET, 0, cmx_forever()) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_pulse(&q, &q.mutex, 3, 1) == CMX_OK &&
        k.task[0].wait == CMX_W_MUTEX);
  CHECK(cdq_put_locked(&q, 3, 42) == CMX_OK && k.task[0].wait == CMX_W_MUTEX);
  CHECK(cdq_get_locked(&q, 3, &value) == CMX_OK && value == 42);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK && q.mutex.owner == 0);
  CHECK(cdq_resume(&q, 0) == CMX_BLOCKED && k.task[0].wait == CMX_W_COND &&
        ledger(&q));
  printf("MESA_STATE:owner=%d;wait=%u;queued=%u;accepted=%llu;consumed=%llu\n",
         q.mutex.owner, (unsigned)k.task[0].wait, q.count,
         (unsigned long long)q.accepted, (unsigned long long)q.consumed);
  CHECK(cmx_cancel(&k, 0) == CMX_OK && q.mutex.owner == 0);
  CHECK(cdq_resume(&q, 0) == CMX_CLOSED && collect(&q, 0, CMX_CLOSED, 0));
  CHECK(q.accepted == 1 && q.consumed == 1 && cdq_destroy(&q) == CMX_OK);
  puts("CDQ_MESA:unremembered-signal;stolen42;recheck-reenrolled;conserved1");
  return 0;
}
static int broadcast(void) {
  cmx_kernel k;
  CDQueue q;
  CHECK(setup(&k, &q, 1, 1));
  for (unsigned t = 0; t < 3; t++)
    CHECK(cdq_begin(&q, t, CDQ_GET, 0, cmx_forever()) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_put_locked(&q, 3, 44) == CMX_OK);
  for (unsigned t = 0; t < 3; t++)
    CHECK(k.task[t].wait == CMX_W_MUTEX);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK && q.mutex.owner == 0);
  CHECK(cdq_resume(&q, 0) == CMX_OK && q.claimed == 1 && q.consumed == 0);
  CHECK(cdq_resume(&q, 1) == CMX_BLOCKED && cdq_resume(&q, 2) == CMX_BLOCKED);
  CHECK(collect(&q, 0, CMX_OK, 44) && ledger(&q) && q.consumed == 1);
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_pulse(&q, &q.mutex, 3, 1) == CMX_OK);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&q, 1) == CMX_BLOCKED && cdq_resume(&q, 2) == CMX_BLOCKED &&
        ledger(&q));
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_shutdown(&q, 3) == CMX_OK);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&q, 1) == CMX_CLOSED && cdq_resume(&q, 2) == CMX_CLOSED);
  CHECK(collect(&q, 1, CMX_CLOSED, 0) && collect(&q, 2, CMX_CLOSED, 0));
  CHECK(cdq_destroy(&q) == CMX_OK);
  puts("CDQ_BROADCAST:three-woken;one-claimed;two-rewait;spurious-recheck;"
       "shutdown");
  return 0;
}
static int terminal(void) {
  for (unsigned action = 0; action < 3; action++) {
    cmx_kernel k;
    CDQueue q;
    cmx_mutex other;
    CHECK(setup(&k, &q, 2, 1));
    CHECK(cmx_mutex_init(&k, &other, 0, 0) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &other, 1, cmx_forever()) == CMX_OK);
    CHECK(cdq_pulse(&q, &other, 1, 1) == CMX_INVALID);
    CHECK(cmx_mutex_unlock(&k, &other, 1) == CMX_OK);
    CHECK(cdq_begin(&q, 0, CDQ_GET, 0, cmx_at(5)) == CMX_BLOCKED);
    CHECK(cdq_destroy(&q) == CMX_BLOCKED &&
          cmx_mutex_close(&k, &q.mutex) == CMX_BLOCKED);
    CHECK(cmx_mutex_lock(&k, &q.mutex, 1, cmx_forever()) == CMX_OK);
    if (action == 0)
      cmx_tick(&k, 5);
    else if (action == 1)
      CHECK(cmx_cancel(&k, 0) == CMX_OK);
    else
      CHECK(cdq_shutdown(&q, 1) == CMX_OK);
    int expected = action == 0 ? CMX_TIMEOUT : CMX_CLOSED;
    CHECK(k.task[0].state == CMX_WAITING && k.task[0].wait == CMX_W_MUTEX &&
          (int)k.task[0].pending == expected);
    cmx_tick(&k, 99);
    CHECK(cdq_resume(&q, 0) == CMX_BLOCKED && q.mutex.owner == 1);
    printf("TERMINAL_STATE:action=%u;time=%u;owner=%d;pending=%u;phase=%u\n",
           action, k.now, q.mutex.owner, (unsigned)k.task[0].pending,
           q.request[0].phase);
    int result = 888;
    uint32_t value = 777;
    CHECK(cdq_result(&q, 0, &result, &value) == CMX_BLOCKED && result == 888 &&
          value == 777);
    CHECK(cdq_destroy(&q) == CMX_BLOCKED);
    CHECK(cmx_mutex_unlock(&k, &q.mutex, 1) == CMX_OK && q.mutex.owner == 0);
    CHECK(cdq_resume(&q, 0) == expected && collect(&q, 0, expected, 0));
    CHECK(q.mutex.owner < 0 && cdq_destroy(&q) == CMX_OK);
  }
  puts("CDQ_TERMINAL:timeout-cancel-close;deadline5-return99;reacquire-before-"
       "result;destroy-refused");
  return 0;
}
static int predicates(void) {
  for (unsigned broad = 0; broad < 2; broad++) {
    cmx_kernel k;
    CDQueue q;
    uint32_t value;
    CHECK(setup(&k, &q, 1, broad));
    CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
    CHECK(cdq_put_locked(&q, 3, 1) == CMX_OK);
    CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK);
    CHECK(cdq_begin(&q, 0, CDQ_PUT, 2, cmx_forever()) == CMX_BLOCKED);
    CHECK(cdq_begin(&q, 1, CDQ_PUT, 3, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
    CHECK(cdq_get_locked(&q, 3, &value) == CMX_OK && value == 1);
    CHECK(cdq_begin_locked(&q, 3, CDQ_GET, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(q.mutex.owner == 0 && cdq_resume(&q, 0) == CMX_OK);
    CHECK(cdq_resume(&q, 1) == CMX_BLOCKED);
    if (!broad) {
      CHECK(q.count == 1 && q.mutex.owner < 0 && k.task[3].wait == CMX_W_COND);
      CHECK(
          cdq_resume(&q, 3) ==
          CMX_BLOCKED); /* Eligible predicate stranded by wrong-kind signal. */
      CHECK(cmx_mutex_lock(&k, &q.mutex, 2, cmx_forever()) == CMX_OK);
      CHECK(cdq_pulse(&q, &q.mutex, 2, 1) == CMX_OK);
      CHECK(cmx_mutex_unlock(&k, &q.mutex, 2) == CMX_OK);
    }
    CHECK(q.mutex.owner == 3 && cdq_resume(&q, 3) == CMX_OK);
    CHECK(collect(&q, 3, CMX_OK, 2) && ledger(&q));
    CHECK(cdq_resume(&q, 1) ==
          CMX_OK); /* Consumer's change enables producer. */
    int result;
    value = 999;
    CHECK(cdq_result(&q, 0, &result, &value) == CMX_OK && result == CMX_OK &&
          value == 999);
    CHECK(cdq_result(&q, 1, &result, &value) == CMX_OK && result == CMX_OK &&
          value == 999);
    CHECK(cmx_mutex_lock(&k, &q.mutex, 2, cmx_forever()) == CMX_OK);
    CHECK(cdq_get_locked(&q, 2, &value) == CMX_OK && value == 3);
    CHECK(cmx_mutex_unlock(&k, &q.mutex, 2) == CMX_OK);
    CHECK(q.accepted == 3 && q.consumed == 3 && ledger(&q) &&
          cdq_destroy(&q) == CMX_OK);
  }
  puts("CDQ_PREDICATES:single-signal-strands;broadcast-progress;accepted3="
       "consumed3");
  return 0;
}
static int shutdown(void) {
  cmx_kernel k;
  CDQueue q;
  uint32_t value;
  int result;
  CHECK(setup(&k, &q, 2, 1));
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_put_locked(&q, 3, 7) == CMX_OK &&
        cdq_put_locked(&q, 3, 8) == CMX_OK);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK);
  CHECK(cdq_begin(&q, 0, CDQ_PUT, 9, cmx_forever()) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_shutdown(&q, 3) == CMX_OK &&
        cdq_put_locked(&q, 3, 10) == CMX_CLOSED);
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&q, 0) == CMX_CLOSED && collect(&q, 0, CMX_CLOSED, 0));
  CHECK(cdq_destroy(&q) == CMX_BLOCKED);
  for (unsigned i = 0; i < 2; i++) {
    CHECK(cdq_begin(&q, 1, CDQ_GET, 0, cmx_forever()) == CMX_OK);
    CHECK(cdq_result(&q, 1, &result, &value) == CMX_OK && result == CMX_OK &&
          value == 7 + i);
  }
  CHECK(cdq_begin(&q, 1, CDQ_GET, 0, cmx_forever()) == CMX_CLOSED &&
        collect(&q, 1, CMX_CLOSED, 0));
  CHECK(q.accepted == 2 && q.consumed == 2 && ledger(&q) &&
        cdq_destroy(&q) == CMX_OK);
  CHECK(setup(&k, &q, 1, 1));
  q.accepted = UINT64_MAX;
  q.consumed = UINT64_MAX; /* Valid finite-counter fixture. */
  CHECK(cmx_mutex_lock(&k, &q.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_put_locked(&q, 3, 11) == CMX_FULL && q.count == 0 && ledger(&q));
  CHECK(cmx_mutex_unlock(&k, &q.mutex, 3) == CMX_OK &&
        cdq_destroy(&q) == CMX_OK);
  puts("CDQ_SHUTDOWN:producer-closed;accepted7-8-drained;empty-closed;"
       "quiescent-destroy");
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "mesa"))
    return mesa();
  if (!strcmp(argv[1], "broadcast"))
    return broadcast();
  if (!strcmp(argv[1], "terminal"))
    return terminal();
  if (!strcmp(argv[1], "predicates"))
    return predicates();
  if (!strcmp(argv[1], "shutdown"))
    return shutdown();
  return 2;
}
