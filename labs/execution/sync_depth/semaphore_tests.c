#include "semaphore.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CDSEM_CHECK:%d:%s\n", __LINE__, #x);                    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
static void tasks(cmx_kernel *k, unsigned count) {
  cmx_init(k, CMX_FIXED);
  for (unsigned t = 0; t < count; ++t)
    CHECK(cmx_task_add(k, 9 - t % 8, 0, 4096 + t * 512, 256) == (int)t);
}
static void ledger(const CDSem *s) {
  const CWObject *o = &s->wait.objects[0];
  CHECK((uint64_t)s->initial + o->posts == o->acquired + o->semaphore.count);
}
static void counting(void) {
  cmx_kernel k;
  CDSem s;
  CWToken a, b;
  CDPost post;
  tasks(&k, 2);
  CHECK(cds_init(&s, &k, 1, 2, 0) == CMX_OK);
  CHECK(cds_take(&s, 0, cmx_forever(), &a) == CMX_OK);
  ledger(&s);
  CHECK(cds_take(&s, 1, cmx_forever(), &b) == CMX_BLOCKED);
  ledger(&s);
  CHECK(cds_post(&s, &post) == CMX_OK && post.woke == 1 &&
        k.current == CMX_NONE);
  ledger(&s);
  CHECK(cds_post(&s, &post) == CMX_OK && !post.woke);
  ledger(&s);
  CHECK(cds_post(&s, &post) == CMX_OK);
  ledger(&s);
  post = (CDPost){99, 98, 97, 96};
  CHECK(cds_post(&s, &post) == CMX_FULL && post.scanned == 99 &&
        post.eligible == 98 && post.woke == 97 && post.defer_schedule == 96);
  ledger(&s);
  CHECK(s.wait.objects[0].posts == 3 && s.wait.objects[0].acquired == 2 &&
        s.wait.objects[0].semaphore.count == 2);
  /* UINT_MAX is a declared capacity, not a wrap-to-zero sentinel. */
  tasks(&k, 1);
  CHECK(cds_init(&s, &k, UINT_MAX, UINT_MAX, 0) == CMX_OK);
  CHECK(cds_post(&s, &post) == CMX_FULL &&
        s.wait.objects[0].semaphore.count == UINT_MAX);
  ledger(&s);
  CHECK(cds_take(&s, 0, cmx_at(k.now), &a) == CMX_OK);
  CHECK(cds_post(&s, &post) == CMX_OK);
  ledger(&s);
  tasks(&k, 1);
  CHECK(cds_init(&s, &k, 1, 1, 0) == CMX_OK);
  CHECK(cds_post(&s, &post) == CMX_FULL);
  CHECK(cds_take(&s, 0, cmx_forever(), &a) == CMX_OK);
  CWToken sentinel = a;
  CHECK(cds_take(&s, 0, cmx_at(k.now), &a) == CMX_TIMEOUT &&
        a.operation == sentinel.operation);
  ledger(&s);
  puts("CDSEM_COUNT:initial1+accepted3=acquired2+free2;overflow-refused;UINT_"
       "MAX;binary-refusal");
}
static void orders(void) {
  unsigned histories = 0;
  for (unsigned first = 0; first < 4; ++first)
    for (unsigned second = 0; second < 4; ++second)
      if (first != second)
        for (unsigned third = 0; third < 4; ++third)
          if (third != first && third != second) {
            unsigned fourth = 6 - first - second - third;
            unsigned order[4] = {first, second, third, fourth};
            cmx_kernel k;
            CDSem s;
            CWToken token;
            CDPost post;
            tasks(&k, 1);
            CHECK(cds_init(&s, &k, 0, 2, 0) == CMX_OK);
            CHECK(cds_take(&s, 0, cmx_at(1), &token) == CMX_BLOCKED);
            unsigned free = 0, acquired = 0, accepted = 0, waiting = 1,
                     closed = 0;
            for (unsigned n = 0; n < 4; ++n) {
              unsigned event = order[n];
              if (event < 2) {
                int expected = closed ? CMX_CLOSED : CMX_OK;
                CHECK(cds_post(&s, &post) == expected);
                if (!closed) {
                  ++accepted;
                  if (waiting) {
                    waiting = 0;
                    ++acquired;
                  } else
                    ++free;
                }
              } else if (event == 2) {
                int r = cw_timeout(token, 1);
                CHECK(r == (waiting ? CMX_OK : CW_STALE));
                waiting = 0;
              } else {
                CHECK(cds_close(&s) == CMX_OK);
                closed = 1;
                waiting = 0;
              }
              CHECK(s.wait.objects[0].posts == accepted &&
                    s.wait.objects[0].acquired == acquired &&
                    s.wait.objects[0].semaphore.count == free);
              ledger(&s);
            }
            ++histories;
          }
  CHECK(histories == 24);
  puts("CDSEM_ORDERS:24 two-post/timeout/close histories;independent token "
       "ledger");
}
static void priority(void) {
  unsigned probes = 0;
  for (unsigned count = 0; count <= CMX_TASKS; ++count) {
    cmx_kernel k;
    CDSem s;
    CWToken tokens[CMX_TASKS];
    CDPost post;
    tasks(&k, count);
    CHECK(cds_init(&s, &k, 0, 2, 1) == CMX_OK);
    for (unsigned t = 0; t < count; ++t)
      CHECK(cds_take(&s, t, cmx_forever(), &tokens[t]) == CMX_BLOCKED);
    uint32_t sequences[CMX_TASKS];
    for (unsigned t = 0; t < count; ++t)
      sequences[t] = k.task[t].wait_sequence;
    CHECK(cds_post(&s, &post) == CMX_OK && post.scanned == CMX_TASKS &&
          post.eligible == count && k.current == CMX_NONE);
    probes += post.scanned;
    ledger(&s);
    if (count) {
      unsigned expected = count < 8 ? count - 1 : 7;
      CHECK(!s.wait.nodes[expected].linked);
      for (unsigned t = 0; t < count; ++t)
        CHECK(k.task[t].wait_sequence == sequences[t]);
    }
  }
  CHECK(probes == 272);
  /* An endless supply of urgent waiters may starve the same low waiter. */
  cmx_kernel k;
  CDSem s;
  CWToken low, high;
  CDPost post;
  tasks(&k, 2);
  CHECK(cds_init(&s, &k, 0, 1, 1) == CMX_OK);
  CHECK(cds_take(&s, 0, cmx_forever(), &low) == CMX_BLOCKED);
  for (unsigned i = 0; i < 100; ++i) {
    CHECK(cds_take(&s, 1, cmx_forever(), &high) == CMX_BLOCKED);
    CHECK(cds_post(&s, &post) == CMX_OK);
    CHECK(s.wait.nodes[0].linked);
    ledger(&s);
  }
  CHECK(cw_cancel(low) == CMX_OK);
  puts("CDSEM_PRIORITY:participants0..16;272 fixed-array probes;stable "
       "stamps;100 urgent bypasses");
}
static void deferred(void) {
  cmx_kernel k;
  CDSem s;
  CDDeferred d;
  CWToken token;
  unsigned done = 99;
  tasks(&k, 1);
  CHECK(cds_init(&s, &k, 1, 1, 0) == CMX_OK &&
        cds_deferred_init(&d, 2) == CMX_OK);
  CHECK(cds_deferred_offer(&d) == CMX_OK && cds_deferred_offer(&d) == CMX_OK &&
        cds_deferred_offer(&d) == CMX_FULL);
  CHECK(cds_deferred_drain(&d, &s, 2, &done) == CMX_FULL && done == 0 &&
        d.pending == 2 && s.wait.objects[0].posts == 0);
  CHECK(cds_take(&s, 0, cmx_forever(), &token) == CMX_OK);
  CHECK(cds_deferred_drain(&d, &s, 1, &done) == CMX_OK && done == 1 &&
        d.pending == 1 && k.current == CMX_NONE);
  ledger(&s);
  CHECK(cds_close(&s) == CMX_OK);
  CHECK(cds_deferred_drain(&d, &s, 2, &done) == CMX_CLOSED && done == 0 &&
        d.pending == 1);
  CHECK(cds_deferred_close(&d) == CMX_OK && d.pending == 0 &&
        d.cancelled == 1 && d.accepted == d.drained + d.cancelled);
  CHECK(cds_deferred_offer(&d) == CMX_CLOSED);
  puts("CDSEM_DEFER:accepted2=drained1+cancelled1;full-retains-credit;close-"
       "acknowledged");
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "counting"))
    counting();
  else if (!strcmp(argv[1], "orders"))
    orders();
  else if (!strcmp(argv[1], "priority"))
    priority();
  else if (!strcmp(argv[1], "deferred"))
    deferred();
  else
    return 2;
  return 0;
}
