#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SC_SERVICE_ASSERT:%s\n", #x);                           \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static SCBatch batch(unsigned work) {
  SCBatch b = {0};
  b.count = 1;
  b.work = work;
  b.commands[0].opcode = 1;
  b.commands[0].count = 1;
  b.commands[0].values[0] = 1;
  return b;
}
static void quota(void) {
  SCService s;
  SCTicket t[4];
  SCBatch b;
  unsigned i;
  static const unsigned costs[] = {2, 3, 5, 1};
  static const enum sc_result wanted[] = {SC_OK, SC_OK, SC_INVALID, SC_OK};
  sc_service_init(&s);
  for (i = 0; i < 4; ++i) {
    b = batch(costs[i]);
    CHECK(sc_submit(&s, &b, &t[i]) == wanted[i]);
  }
  CHECK(s.used == 6 && s.count == 3);
  CHECK(sc_control(&s) == SC_OK && s.control_pending == 1);
  b = batch(1);
  CHECK(sc_submit(&s, &b, &t[2]) == SC_BUDGET);
  CHECK(sc_control(&s) == SC_FULL);
  CHECK(sc_control_complete(&s) == SC_OK && !s.control_pending);
  CHECK(sc_control_complete(&s) == SC_STALE);
  CHECK(sc_shutdown(&s) == SC_OK && !s.used && !s.count && s.released == 3);
  sc_service_init(&s);
  b = batch(4);
  CHECK(sc_submit(&s, &b, &t[0]) == SC_OK);
  b = batch(3);
  CHECK(sc_submit(&s, &b, &t[1]) == SC_BUDGET && s.used == 4 && s.count == 1);
  CHECK(sc_shutdown(&s) == SC_OK);
  /* Sustained finite hostile bursts cannot increase retained capacity. */
  sc_service_init(&s);
  for (i = 0; i < 1024; ++i) {
    SCAuthority a = {1, 1};
    uint32_t p = 0, q = 0;
    b = batch(4);
    CHECK(sc_submit(&s, &b, &t[0]) == SC_OK);
    CHECK(sc_submit(&s, &b, &t[1]) == SC_BUDGET && s.used == 4);
    CHECK(sc_finish(&s, t[0], 0, &a, &p, &q) == SC_OK);
    CHECK(s.used == 0 && s.count == 0);
  }
  CHECK(s.terminal == 1024 && s.released == 1024);
  printf(
      "OBS storage batch=%zu service=%zu recovery=%zu pointer=%zu size_t=%zu\n",
      sizeof(SCBatch), sizeof(SCService), sizeof(SCRecovery), sizeof(void *),
      sizeof(size_t));
  puts("OBS quota costs=2,3,5,1 admitted=1,1,0,1 retained=6 queue=3 "
       "reserved_control=1");
}
static void schedules(void) {
  unsigned order, mutate;
  for (order = 0; order < 2; ++order)
    for (mutate = 0; mutate < 2; ++mutate) {
      SCService s;
      SCTicket t;
      SCBatch b = batch(1);
      SCAuthority a = {1, 1};
      uint32_t p = 0, q = 0;
      enum sc_result first;
      sc_service_init(&s);
      b.commands[0].opcode = 2;
      b.commands[0].generation = 1;
      CHECK(sc_submit(&s, &b, &t) == SC_OK);
      b.commands[0].count = 5; /* Queue owns an immutable copy. */
      if (mutate)
        CHECK(sc_revoke(&a) == SC_OK);
      first = sc_finish(&s, t, order, &a, &p, &q);
      CHECK(first == ((!order && mutate) ? SC_STALE : SC_OK));
      CHECK(sc_finish(&s, t, !order, &a, &p, &q) == SC_STALE);
      CHECK(s.terminal == 1 && s.released == 1 && !s.used && !s.count);
      CHECK(q == (!order && !mutate));
    }
  {
    SCService s;
    SCTicket t;
    SCBatch b = batch(1);
    SCAuthority a = {1, 1};
    uint32_t p = 0, q = 0;
    sc_service_init(&s);
    s.jobs[0].generation = UINT_MAX - 1;
    CHECK(sc_submit(&s, &b, &t) == SC_OK);
    CHECK(sc_retry(&s, t) == SC_OK && sc_retry(&s, t) == SC_OK &&
          sc_retry(&s, t) == SC_EXHAUSTED);
    CHECK(sc_finish(&s, t, 1, &a, &p, &q) == SC_OK &&
          s.jobs[0].generation == UINT_MAX);
    CHECK(sc_finish(&s, t, 0, &a, &p, &q) == SC_STALE);
    CHECK(sc_submit(&s, &b, &t) == SC_OK && t.index == 1);
    CHECK(sc_shutdown(&s) == SC_OK);
  }
  for (order = 0; order < 2; ++order) {
    SCService s;
    SCTicket t;
    SCBatch b = batch(1);
    SCAuthority a = {1, 1};
    uint32_t p = 0, q = 0;
    sc_service_init(&s);
    CHECK(sc_submit(&s, &b, &t) == SC_OK);
    if (!order)
      CHECK(sc_retry(&s, t) == SC_OK);
    CHECK(sc_shutdown(&s) == SC_OK && s.released == 1 && s.terminal == 1);
    CHECK(sc_retry(&s, t) == SC_STALE);
    CHECK(sc_finish(&s, t, 0, &a, &p, &q) == SC_STALE);
    CHECK(sc_submit(&s, &b, &t) == SC_EXHAUSTED);
    CHECK(sc_control(&s) == SC_EXHAUSTED);
    CHECK(sc_shutdown(&s) == SC_OK && s.released == 1);
  }
  {
    SCService s;
    SCTicket t;
    SCBatch b = batch(1);
    sc_service_init(&s);
    s.terminal = s.released = UINT_MAX - 1;
    CHECK(sc_submit(&s, &b, &t) == SC_OK);
    CHECK(sc_submit(&s, &b, &t) == SC_EXHAUSTED);
    CHECK(sc_shutdown(&s) == SC_OK && s.terminal == UINT_MAX);
  }
  puts("OBS schedules terminal_orders=4 exact_release=1 copy_ownership=1 "
       "late_callback=stale retry_limit=2 generation=retired");
}
typedef struct {
  unsigned fail, live, count, order[4];
} Resources;
static int acquire(void *p, unsigned i) {
  Resources *r = p;
  if (i == r->fail)
    return 1;
  CHECK(!(r->live & (1u << i)));
  r->live |= 1u << i;
  return 0;
}
static void release(void *p, unsigned i) {
  Resources *r = p;
  CHECK(r->live & (1u << i));
  r->live &= ~(1u << i);
  r->order[r->count++] = i;
}
static void recovery(void) {
  unsigned fail;
  for (fail = 0; fail <= 2; ++fail) {
    Resources r = {0};
    SCRecovery s = {0};
    SCResources ops = {acquire, release, &r};
    r.fail = fail;
    CHECK(sc_recover(&s, ops, 0) == (fail < 2 ? SC_FULL : SC_OK));
    CHECK(s.attempts == 1 && s.log_fallbacks == 1);
    CHECK(sc_recover(&s, ops, 0) == (fail < 2 ? SC_FULL : SC_OK));
    CHECK(s.attempts == 2 && s.log_fallbacks == 2);
    CHECK(sc_recover(&s, ops, 0) == SC_EXHAUSTED && s.state == 2 && !r.live &&
          !s.owned);
    CHECK(sc_recover(&s, ops, 0) == SC_EXHAUSTED && s.attempts == 2);
    CHECK(sc_recover(&s, ops, 0) == SC_EXHAUSTED && s.attempts == 2 && !r.live);
    if (fail == 2)
      CHECK(r.count == 4 && r.order[0] == 1 && r.order[1] == 0 &&
            r.order[2] == 1 && r.order[3] == 0);
  }
  {
    Resources r = {0};
    SCRecovery s = {2, 2, 0, UINT_MAX};
    SCResources ops = {acquire, release, &r};
    CHECK(sc_recover(&s, ops, 0) == SC_EXHAUSTED &&
          s.log_fallbacks == UINT_MAX);
  }
  puts("OBS recovery acquisition_failures=2 successful_prefix=2 "
       "cleanup_before_stop=1 recoveries=2 logger_fallback=bounded");
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  if (!strcmp(argv[1], "quota"))
    quota();
  else if (!strcmp(argv[1], "schedules"))
    schedules();
  else if (!strcmp(argv[1], "recovery"))
    recovery();
  else
    CHECK(0);
  return 0;
}
