#include "reactor.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "REACTOR-CHECK:%s:%d\n", #x, __LINE__);                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int operations(void) {
  struct rd_operation rx, tx;
  const unsigned char input[3] = {7, 0, 9}, output[3] = {1, 2, 3};
  CHECK(!rd_operation_init(&rx, 3, 0) && !rd_operation_init(&tx, 3, output));
  CHECK(rd_progress(&rx, 2, input) == 1 && rx.done == 2 &&
        rx.terminal == RD_ACTIVE);
  CHECK(rd_progress(&rx, -2, 0) == 0 && rx.done == 2 &&
        rx.terminal == RD_ACTIVE);
  CHECK(rd_progress(&rx, 1, input + 2) == 1 && rx.done == 3 &&
        rx.terminal == RD_DONE);
  CHECK(!memcmp(rx.bytes, input, 3) && rd_cancel(&rx) == 0);
  CHECK(!rd_operation_init(&rx, 3, 0));
  for (unsigned i = 0; i < 2; ++i) {
    CHECK(rd_progress(&rx, 1, input + i) == 1);
    CHECK(rd_progress(&tx, 1, 0) == 1);
    CHECK(rd_progress(&tx, -2, 0) == 0);
  }
  CHECK(rd_cancel(&rx) == 1 && rd_cancel(&rx) == 0 &&
        rx.terminal == RD_CANCELLED && rx.done == 2);
  CHECK(tx.terminal == RD_ACTIVE && tx.done == 2 &&
        !memcmp(tx.bytes, output, 3));
  CHECK(rd_progress(&tx, 1, 0) == 1 && tx.terminal == RD_DONE);
  CHECK(!rd_operation_init(&rx, 3, 0) && rd_progress(&rx, 1, input) == 1 &&
        rd_progress(&rx, 0, 0) == 1 && rx.terminal == RD_EOF && rx.done == 1);
  return 0;
}
static int identities(void) {
  struct rd_registry r;
  struct rd_token old, fresh, untouched = {99, 99};
  CHECK(!rd_registry_init(&r, 7, 8) && rd_register(&r, 2, 100, &old) == 1 &&
        old.generation == 7);
  struct rd_slot *s = rd_borrow(&r, old);
  CHECK(s && s->descriptor == 100);
  const unsigned char tx[2] = {3, 4};
  CHECK(!rd_operation_init(&s->receive, 3, 0) &&
        !rd_operation_init(&s->transmit, 2, tx));
  struct rd_completion rx1, rx2, tx1, ignored;
  const unsigned char byte = 42;
  CHECK(rd_submit(&r, old, 0, &rx1) == 1 && rd_submit(&r, old, 1, &tx1) == 1 &&
        rd_submit(&r, old, 1, &ignored) == 0);
  CHECK(rd_complete(&r, rx1, 1, &byte) == 1 && s->receive.done == 1 &&
        rd_submit(&r, old, 0, &rx2) == 1 && rx2.sequence == 2);
  CHECK(rd_complete(&r, rx1, 1, &byte) == 0 && s->receive.done == 1 &&
        s->pending == 3);
  CHECK(rd_close(&r, old) == 1 && !rd_borrow(&r, old) &&
        rd_register(&r, 2, 101, &untouched) == 0 && untouched.slot == 99);
  CHECK(r.slots[2].active && r.slots[2].pending == 3);
  /* A cancellation acknowledgment carries no target completion and changes no
   * pending bit. Model the later actual target terminal separately. */
  struct rd_completion wrong = rx2;
  wrong.connection.generation = 6;
  CHECK(rd_complete(&r, wrong, -1, 0) == 0 && r.slots[2].pending == 3);
  CHECK(rd_complete(&r, tx1, 1, 0) == 1 && r.slots[2].pending == 1 &&
        s->transmit.done == 1);
  CHECK(rd_complete(&r, rx2, -1, 0) == 1 && r.slots[2].active &&
        s->receive.terminal == RD_ERROR);
  CHECK(!rd_return(&r, old) && !r.slots[2].active &&
        r.slots[2].generation == 8);
  CHECK(rd_register(&r, 2, 100, &fresh) == 1 && fresh.generation == 8 &&
        !rd_borrow(&r, old));
  CHECK(rd_complete(&r, tx1, 1, 0) == 0);
  s = rd_borrow(&r, fresh);
  CHECK(s && !rd_operation_init(&s->receive, 1, 0));
  s->submitted[0] = 65535;
  ignored = (struct rd_completion){.sequence = 99};
  CHECK(rd_submit(&r, fresh, 0, &ignored) == 0 && ignored.sequence == 99 &&
        !s->pending && !rd_return(&r, fresh));
  CHECK(rd_close(&r, fresh) == 1 && r.slots[2].retired &&
        rd_register(&r, 2, 101, &fresh) == 0);
  CHECK(!rd_registry_init(&r, 1, 3));
  for (unsigned g = 1; g <= 3; ++g) {
    CHECK(rd_register(&r, 0, 10, &fresh) == 1 && fresh.generation == g);
    CHECK(rd_close(&r, fresh) == 1);
  }
  CHECK(r.slots[0].retired && rd_register(&r, 0, 10, &fresh) == 0);
  const unsigned permutations[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                                       {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
  for (unsigned p = 0; p < 6; ++p) {
    CHECK(!rd_registry_init(&r, 1, 2) && rd_register(&r, 0, 10, &fresh) == 1);
    s = rd_borrow(&r, fresh);
    CHECK(s && !rd_operation_init(&s->receive, 1, 0) &&
          !rd_operation_init(&s->transmit, 1, &byte));
    CHECK(rd_submit(&r, fresh, 0, &rx1) == 1 &&
          rd_submit(&r, fresh, 1, &tx1) == 1);
    struct rd_completion invalid = rx1;
    invalid.direction = 2;
    CHECK(rd_complete(&r, invalid, 1, &byte) == 0 && s->pending == 3 &&
          rd_close(&r, fresh) == 1);
    for (unsigned step = 0; step < 3; ++step) {
      unsigned event = permutations[p][step];
      if (event == 0)
        CHECK(rd_complete(&r, rx1, 1, &byte) == 1 &&
              rd_complete(&r, rx1, 1, &byte) == 0);
      else if (event == 1)
        CHECK(rd_complete(&r, tx1, 1, 0) == 1 &&
              rd_complete(&r, tx1, 1, 0) == 0);
      else
        CHECK(!rd_return(&r, fresh));
      CHECK(r.slots[0].active == (step < 2));
    }
    CHECK(r.slots[0].generation == 2);
  }
  return 0;
}
struct rd_work {
  struct rd_scheduler *scheduler;
  unsigned counts[3], zero, inject, nested_rejected;
};
static int callback(void *context, unsigned at) {
  struct rd_work *w = context;
  ++w->counts[at];
  if (w->inject && at == 0) {
    (void)rd_ready(w->scheduler, 2);
    struct rd_turn ignored;
    w->nested_rejected = rd_turn(w->scheduler, 1, callback, w, &ignored) == -1;
  }
  return w->zero ? 0 : 1;
}
static int self_publish(void *context, unsigned at) {
  struct rd_work *w = context;
  ++w->counts[at];
  (void)rd_ready(w->scheduler, at);
  return -2;
}
static int scheduling(void) {
  struct rd_scheduler s = {0};
  struct rd_turn t;
  struct rd_work w = {.scheduler = &s};
  CHECK(!rd_ready(&s, 0) && !rd_ready(&s, 1) &&
        !rd_turn(&s, 2, callback, &w, &t));
  CHECK(t.callbacks == 2 && t.successes == 2 && t.order[0] == 0 &&
        t.order[1] == 0);
  CHECK(!rd_turn(&s, 2, callback, &w, &t) && t.order[0] == 1 &&
        w.counts[1] == 2);
  s = (struct rd_scheduler){.ready = 7};
  w = (struct rd_work){.scheduler = &s};
  CHECK(!rd_turn(&s, 8, callback, &w, &t) && t.successes == 6 &&
        t.callbacks == 6);
  for (unsigned i = 0; i < 6; ++i)
    CHECK(t.order[i] == i / 2);
  s = (struct rd_scheduler){.ready = 7};
  w = (struct rd_work){.scheduler = &s, .zero = 1};
  for (unsigned turn = 0; turn < 3; ++turn) {
    CHECK(!rd_turn(&s, 2, callback, &w, &t));
    CHECK(t.callbacks == 2 && t.successes == 0 && t.order[0] == turn);
  }
  CHECK(w.counts[0] == 2 && w.counts[1] == 2 && w.counts[2] == 2);
  s = (struct rd_scheduler){.ready = 1};
  w = (struct rd_work){.scheduler = &s, .inject = 1};
  CHECK(!rd_turn(&s, 8, callback, &w, &t) && t.successes == 2 &&
        w.counts[2] == 0 && w.nested_rejected);
  CHECK(!rd_turn(&s, 8, callback, &w, &t) && w.counts[2] == 2);
  s = (struct rd_scheduler){.ready = 1};
  w = (struct rd_work){.scheduler = &s};
  CHECK(!rd_turn(&s, 8, self_publish, &w, &t) && t.callbacks == 1 &&
        s.ready == 1 && w.counts[0] == 1);
  CHECK(!rd_turn(&s, 8, self_publish, &w, &t) && t.callbacks == 1 &&
        s.ready == 1 && w.counts[0] == 2);
  /* Bounded execution of the flawed always-drain policy; no infinite hang. */
  w = (struct rd_work){.scheduler = &s};
  for (unsigned step = 0; step < 16; ++step)
    CHECK(callback(&w, 0) == 1);
  CHECK(w.counts[0] == 16 && w.counts[1] == 0);
  return 0;
}
static int policies(void) {
  for (unsigned policy = 0; policy < 3; ++policy) {
    struct rd_service s;
    struct rf_message messages[9];
    unsigned slots[4], extra = 99;
    struct rf_frame frame = {.length = 1, .bytes = {42}};
    CHECK(!rd_service_init(&s, 256));
    for (unsigned i = 0; i < 4; ++i)
      CHECK(rd_service_open(&s, &slots[i]) == 1);
    CHECK(rd_service_open(&s, &extra) == 0 && extra == 99);
    for (unsigned i = 0; i < 9; ++i) {
      CHECK(!rf_message_init(&messages[i], i + 1, 1, &frame));
      CHECK(rd_admit(&s, 0, &messages[i], (enum rd_policy)policy) == (i < 8));
    }
    CHECK(messages[8].owner == RF_CALLER && s.accepted == 8);
    if (policy == RD_DISCONNECT) {
      CHECK(s.terminal == 8 && !s.outbox.connections[0].active);
      for (unsigned i = 0; i < 8; ++i)
        CHECK(messages[i].owner == RF_CANCELLED);
    } else if (policy == RD_PAUSE_READ) {
      CHECK(s.paused == 1 && rd_resume(&s, 0) == 0);
      for (unsigned i = 0; i < 5; ++i)
        CHECK(!rd_commit(&s, 0, 2));
      CHECK(rd_admit(&s, 0, &messages[8], RD_PAUSE_READ) == 0);
      CHECK(rd_resume(&s, 0) == 1 &&
            rd_admit(&s, 0, &messages[8], RD_PAUSE_READ) == 1);
    } else
      CHECK(s.terminal == 0 && !s.paused && s.outbox.connections[0].count == 8);
    CHECK(!rd_stop(&s, 1, 1) && s.phase == RD_RELEASED &&
          s.terminal == s.accepted);
  }
  unsigned cells = 0;
  for (unsigned limit = 0; limit <= 32; ++limit)
    for (unsigned length = 0; length <= 8; ++length) {
      struct rd_service s;
      struct rf_message m[9];
      unsigned at;
      struct rf_frame f = {.length = length};
      size_t expected_bytes = 0;
      unsigned accepted = 0;
      CHECK(!rd_service_init(&s, limit) && rd_service_open(&s, &at) == 1);
      for (unsigned i = 0; i < 9; ++i) {
        int expected = accepted < 8 && expected_bytes + length <= limit;
        CHECK(!rf_message_init(&m[i], i + 1, 2, &f));
        CHECK(rd_admit(&s, at, &m[i], RD_DROP_NEWEST) == expected);
        if (expected) {
          ++accepted;
          expected_bytes += length;
        }
        CHECK(s.outbox.bytes == expected_bytes);
      }
      CHECK(!rd_stop(&s, 0, 0) && s.terminal == accepted);
      ++cells;
    }
  CHECK(cells == 297);
  struct rd_service epoch;
  struct rf_message message;
  struct rf_frame empty = {0};
  unsigned slot;
  CHECK(!rd_service_init(&epoch, 0) && rd_service_open(&epoch, &slot) == 1);
  for (unsigned id = 1; id <= 64; ++id) {
    CHECK(!rf_message_init(&message, id, 1, &empty) &&
          rd_admit(&epoch, slot, &message, RD_DROP_NEWEST) == 1 &&
          !rd_commit(&epoch, slot, 1));
  }
  CHECK(!rf_message_init(&message, 65, 1, &empty) &&
        rd_admit(&epoch, slot, &message, RD_DROP_NEWEST) == 0 &&
        message.owner == RF_CALLER && epoch.accepted == 64 &&
        epoch.terminal == 64 && !rd_stop(&epoch, 0, 0));
  return 0;
}
static int shutdowns(void) {
  struct rd_service s;
  struct rf_message m[7];
  struct rf_frame f = {.length = 1, .bytes = {4}};
  unsigned at;
  CHECK(!rd_service_init(&s, 8) && rd_service_open(&s, &at) == 1);
  for (unsigned i = 0; i < 7; ++i)
    CHECK(!rf_message_init(&m[i], i + 1, 1, &f));
  for (unsigned i = 0; i < 6; ++i)
    CHECK(rd_admit(&s, at, &m[i], RD_DROP_NEWEST) == 1);
  for (unsigned i = 0; i < 3; ++i)
    CHECK(!rd_commit(&s, at, 2));
  CHECK(!rd_service_borrow(&s) && !rd_stop(&s, 10, 20) &&
        s.phase == RD_DRAINING);
  CHECK(!rd_stop(&s, 11, 30) && s.deadline == 20 &&
        rd_admit(&s, at, &m[6], RD_DROP_NEWEST) == 0 &&
        m[6].owner == RF_CALLER);
  CHECK(!rd_tick(&s, 19) && s.terminal == 3 && s.phase == RD_DRAINING);
  CHECK(!rd_tick(&s, 20) && s.phase == RD_CANCELLING && s.terminal == 3 &&
        s.borrowers == 1);
  for (unsigned i = 3; i < 6; ++i)
    CHECK(m[i].owner == RF_OUTBOX);
  CHECK(!rd_stop(&s, 25, 25) && s.terminal == 3 && s.phase == RD_CANCELLING);
  CHECK(!rd_service_return(&s) && s.phase == RD_RELEASED &&
        !rd_stop(&s, 30, 30) && s.terminal == 6);
  for (unsigned i = 0; i < 6; ++i)
    CHECK(s.records[i].id == i + 1 &&
          s.records[i].outcome == (i < 3 ? RF_DELIVERED : RF_CANCELLED));
  CHECK(!rd_service_init(&s, 8) && !rd_service_borrow(&s));
  /* Reentrant stop during a callback may settle work but cannot destroy the
   * service until the current callback returns its borrow. */
  CHECK(!rd_stop(&s, 0, 0) && s.phase == RD_CANCELLING &&
        !rd_service_return(&s) && s.phase == RD_RELEASED);
  return 0;
}
int main(void) {
  CHECK(!operations() && !identities() && !scheduling() && !policies() &&
        !shutdowns());
  puts("REACTOR-PASS:cursor_duplex_generations=1;three_policies=3;byte_budget_"
       "cells=297;shutdown_unique=6;");
  return 0;
}
