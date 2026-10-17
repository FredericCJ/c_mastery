#include "resilience.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RZ_CHECK:%s:%d:%s\n", __func__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int deadline(void) {
  RZScope parent, child;
  uint64_t remaining = 99;
  CHECK(rz_scope_init(&parent, NULL, 10, 10) == RZ_OK);
  CHECK(rz_scope_init(&child, &parent, 13, 100) == RZ_OK);
  CHECK(child.deadline == 20);
  CHECK(rz_remaining(&child, 19, &remaining) == RZ_OK && remaining == 1);
  CHECK(rz_remaining(&child, 20, &remaining) == RZ_EXPIRED && remaining == 1);
  rz_cancel(&parent);
  CHECK(rz_check(&child, 20) == RZ_CANCELLED);
  CHECK(rz_scope_init(&child, NULL, UINT64_MAX, 1) == RZ_RANGE);
  return 0;
}
static int pressure(void) {
  RZScope scope;
  RZQueue q;
  CHECK(rz_scope_init(&scope, NULL, 0, 10) == RZ_OK);
  rz_queue_init(&q);
  for (unsigned i = 1; i <= 4; i++) {
    RZOffer o = {&scope, i, true};
    CHECK(rz_offer(&q, &o, RZ_REJECT, 0) == RZ_OK && !o.pending);
  }
  RZOffer block = {&scope, 9, true};
  CHECK(rz_offer(&q, &block, RZ_BLOCK, 0) == RZ_PENDING && block.pending);
  RZOffer drop = {&scope, 9, true};
  CHECK(rz_offer(&q, &drop, RZ_DROP, 0) == RZ_DROPPED);
  CHECK(q.values[q.head] == 1 && q.count == 4);
  RZOffer replace = {&scope, 9, true};
  CHECK(rz_offer(&q, &replace, RZ_OVERWRITE, 0) == RZ_OK);
  unsigned value;
  CHECK(rz_pop(&q, &value) == RZ_OK && value == 2);
  RZOffer a = {&scope, 7, true};
  CHECK(rz_offer(&q, &a, RZ_REJECT, 1) == RZ_OK);
  RZOffer degrade = {&scope, 10, true};
  CHECK(rz_offer(&q, &degrade, RZ_DEGRADE, 1) == RZ_DEGRADED && q.count == 4);
  CHECK(q.values[(q.head + 3) % 4] == 10);
  RZQueue before_expiry = q;
  CHECK(rz_offer(&q, &block, RZ_BLOCK, 10) == RZ_EXPIRED && !block.pending);
  CHECK(q.head == before_expiry.head && q.count == before_expiry.count &&
        q.closed == before_expiry.closed);
  for (unsigned i = 0; i < 4; i++)
    CHECK(q.values[i] == before_expiry.values[i]);
  return 0;
}
static int wheel(void) {
  for (unsigned delay = 1; delay <= 16; delay++) {
    RZWheel w;
    RZTimerId id;
    rz_wheel_init(&w, 0);
    CHECK(rz_schedule(&w, delay, &id) == RZ_OK);
    for (unsigned time = 1; time <= 16; time++) {
      unsigned mask = 77;
      CHECK(rz_tick(&w, &mask) == RZ_OK);
      CHECK(mask == (time == delay ? 1u : 0u));
    }
  }
  RZWheel w;
  RZTimerId old, newer;
  rz_wheel_init(&w, 0);
  CHECK(rz_schedule(&w, 1, &old) == RZ_OK);
  CHECK(rz_unschedule(&w, old) == RZ_OK);
  CHECK(rz_schedule(&w, 2, &newer) == RZ_OK);
  CHECK(rz_unschedule(&w, old) == RZ_STALE);
  CHECK(w.slots[newer.slot].active);
  CHECK(rz_unschedule(&w, newer) == RZ_OK);
  return 0;
}
static int bucket(void) {
  RZBucket b;
  CHECK(rz_bucket_init(&b, 4, 1, 0) == RZ_OK);
  CHECK(rz_consume(&b, 0, 4) == RZ_OK && b.tokens == 0);
  CHECK(rz_consume(&b, 0, 1) == RZ_FULL && b.tokens == 0);
  CHECK(rz_consume(&b, 1, 1) == RZ_OK && b.tokens == 0);
  CHECK(rz_consume(&b, UINT64_MAX, 4) == RZ_OK && b.tokens == 0);
  CHECK(rz_consume(&b, 0, 1) == RZ_INVALID);
  for (unsigned cap = 1; cap <= 16; cap++)
    for (unsigned rate = 1; rate <= 16; rate++)
      for (unsigned elapsed = 0; elapsed < 20; elapsed++) {
        CHECK(rz_bucket_init(&b, cap, rate, 0) == RZ_OK);
        CHECK(rz_consume(&b, 0, cap) == RZ_OK);
        unsigned available = elapsed * rate;
        if (available > cap)
          available = cap;
        int s = rz_consume(&b, elapsed, 1);
        CHECK(s == (available ? RZ_OK : RZ_FULL));
        CHECK(b.tokens == (available ? available - 1 : 0));
      }
  return 0;
}
static int retry(void) {
  RZScope scope;
  RZRetry r;
  uint64_t out = 99;
  CHECK(rz_scope_init(&scope, NULL, 0, 10) == RZ_OK);
  CHECK(rz_retry_init(&r, &scope, 3, 2, 8, false) == RZ_OK);
  CHECK(rz_retry_next(&r, 0, 1, &out) == RZ_DENIED && out == 99);
  CHECK(rz_retry_init(&r, &scope, 3, 2, 8, true) == RZ_OK);
  CHECK(rz_retry_next(&r, 0, 2, &out) == RZ_OK && out == 2);
  CHECK(rz_retry_next(&r, 2, 4, &out) == RZ_OK && out == 6);
  CHECK(rz_retry_next(&r, 6, 3, &out) == RZ_OK && out == 9);
  CHECK(rz_retry_next(&r, 9, 0, &out) == RZ_FULL);
  /* Independent clock history: rejecting a late start preserves its attempt
     count and output without moving the preceding history backwards. */
  RZScope late_scope;
  RZRetry late;
  uint64_t late_out = 99;
  CHECK(rz_scope_init(&late_scope, NULL, 0, 10) == RZ_OK);
  CHECK(rz_retry_init(&late, &late_scope, 3, 2, 8, true) == RZ_OK);
  CHECK(rz_retry_next(&late, 0, 2, &late_out) == RZ_OK && late_out == 2);
  CHECK(rz_retry_next(&late, 9, 1, &late_out) == RZ_EXPIRED && late_out == 2 &&
        late.attempts == 1);
  return 0;
}
static int gate(void) {
  RZGate g;
  RZPermit a, b, c;
  CHECK(rz_gate_init(&g, 1, 0) == RZ_OK);
  CHECK(rz_admit(&g, 0, &a) == RZ_OK);
  CHECK(rz_admit(&g, 0, &b) == RZ_BUSY);
  CHECK(rz_finish(&g, a, 0, 1) == RZ_OK);
  CHECK(rz_finish(&g, a, 0, 1) == RZ_STALE);
  CHECK(rz_gate_init(&g, 2, 0) == RZ_OK);
  CHECK(rz_admit(&g, 0, &a) == RZ_OK);
  CHECK(rz_admit(&g, 0, &b) == RZ_OK);
  CHECK(rz_finish(&g, b, 0, 0) == RZ_OK);
  CHECK(rz_admit(&g, 0, &c) == RZ_OK);
  CHECK(rz_finish(&g, c, 0, 0) == RZ_OK && g.phase == 1);
  CHECK(rz_finish(&g, a, 0, 1) == RZ_OK && g.phase == 1);
  CHECK(rz_admit(&g, 3, &a) == RZ_OPEN);
  CHECK(rz_admit(&g, 4, &a) == RZ_OK && g.phase == 2);
  CHECK(rz_admit(&g, 4, &b) == RZ_BUSY);
  CHECK(rz_finish(&g, a, 4, 1) == RZ_OK && g.phase == 0);
  return 0;
}
static int hedge(void) {
  RZScope s;
  RZHedge h;
  unsigned out = 99;
  CHECK(rz_scope_init(&s, NULL, 0, 10) == RZ_OK);
  CHECK(rz_hedge_init(&h, &s, 0, 2) == RZ_OK);
  CHECK(rz_hedge_complete(&h, 0, 1, false, 7) == RZ_DONE);
  CHECK(rz_hedge_result(&h, &out) == RZ_PENDING && out == 99);
  CHECK(rz_hedge_start(&h, 2) == RZ_OK);
  CHECK(rz_hedge_complete(&h, 1, 3, true, 8) == RZ_OK);
  CHECK(rz_hedge_result(&h, &out) == RZ_OK && out == 8);
  CHECK(rz_hedge_quiescent(&h));
  CHECK(rz_hedge_init(&h, &s, 3, 2) == RZ_OK);
  CHECK(rz_hedge_start(&h, 5) == RZ_OK);
  CHECK(rz_hedge_complete(&h, 1, 6, true, 8) == RZ_OK);
  CHECK(!rz_hedge_quiescent(&h) && h.cancel_requested == 1);
  CHECK(rz_hedge_complete(&h, 0, 7, true, 7) == RZ_DONE);
  CHECK(rz_hedge_result(&h, &out) == RZ_OK && out == 8 &&
        rz_hedge_quiescent(&h));
  return 0;
}
static int receiver(void) {
  RZReceiver r;
  int out = 99;
  bool duplicate = false;
  CHECK(rz_receiver_init(&r, 0, 4) == RZ_OK);
  CHECK(rz_receive(&r, 0, 0, 1, 2, 3, &out, &duplicate) == RZ_OK && out == 3 &&
        !duplicate);
  CHECK(rz_receive(&r, 1, 0, 1, 2, 3, &out, &duplicate) == RZ_OK && out == 3 &&
        duplicate && r.total == 3);
  CHECK(rz_receive(&r, 1, 0, 1, 2, 4, &out, &duplicate) == RZ_CONFLICT &&
        r.total == 3);
  CHECK(rz_receive(&r, 1, 1, 1, 1, 2, &out, &duplicate) == RZ_STALE &&
        r.total == 3);
  CHECK(rz_receive(&r, 4, 0, 1, 2, 3, &out, &duplicate) == RZ_OK && out == 6 &&
        !duplicate);
  return 0;
}
static int service(void) {
  RZScope scope;
  RZService s;
  MsgHub hub;
  int out = 99;
  unsigned channel = 99, value = 99;
  CHECK(msg_hub_init(&hub, 1, 0) == MSG_OK);
  CHECK(rz_scope_init(&scope, NULL, 0, 100) == RZ_OK);
  CHECK(rz_service_init(&s, &scope, &hub, 0) == RZ_OK);
  CHECK(rz_service_submit(&s, 0, 0, 1, 1, 2, &out) == RZ_OK && out == 2);
  CHECK(rz_service_flush(&s) == RZ_OK);
  CHECK(msg_select(&hub, 1, false, &channel, &value) == MSG_OK &&
        channel == 0 && value == 1);
  CHECK(rz_service_submit(&s, 1, 0, 2, 1, 2, &out) == RZ_OK);
  CHECK(rz_service_submit(&s, 2, 0, 3, 1, 2, &out) == RZ_OK);
  CHECK(rz_service_submit(&s, 3, 0, 4, 1, 2, &out) == RZ_OK);
  CHECK(rz_service_submit(&s, 8, 0, 5, 1, 2, &out) == RZ_OK &&
        s.pending.count == 4);
  int before = s.receiver.total;
  CHECK(rz_service_submit(&s, 9, 0, 6, 1, 2, &out) == RZ_FULL &&
        s.receiver.total == before);
  CHECK(rz_service_flush(&s) == RZ_OK);
  CHECK(rz_service_flush(&s) == RZ_FULL && s.pending.count == 3);
  msg_hub_cancel(&hub);
  CHECK(rz_service_flush(&s) == RZ_CANCELLED && s.pending.count == 3);
  msg_hub_destroy(&hub);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
#define RUN(name)                                                              \
  if (strcmp(argv[1], #name) == 0)                                             \
  return name()
  RUN(deadline);
  RUN(pressure);
  RUN(wheel);
  RUN(bucket);
  RUN(retry);
  RUN(gate);
  RUN(hedge);
  RUN(receiver);
  RUN(service);
  return 2;
}
