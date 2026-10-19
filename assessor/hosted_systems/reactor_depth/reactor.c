#include "reactor.h"
int rd_operation_init(RdOperation *o, size_t goal, const unsigned char *tx) {
  if (!o || !goal || goal > 8)
    return -1;
  *o = (RdOperation){.goal = goal};
  if (tx)
    for (size_t i = 0; i < goal; ++i)
      o->bytes[i] = tx[i];
  return 0;
}
int rd_progress(RdOperation *o, int amount, const unsigned char *rx) {
  if (!o || o->goal > 8 || o->done > o->goal || o->terminal != RD_ACTIVE ||
      amount < -2)
    return -1;
  if (amount == -2) {
#ifdef RD_MUTANT_EAGAIN
    o->terminal = RD_ERROR;
#endif
    return 0;
  }
  if (amount <= 0) {
    o->terminal = amount ? RD_ERROR : RD_EOF;
    return 1;
  }
  if ((size_t)amount > o->goal - o->done)
    return -1;
  if (rx)
    for (int i = 0; i < amount; ++i)
      o->bytes[o->done + (size_t)i] = rx[i];
  o->done += (size_t)amount;
  if (o->done == o->goal)
    o->terminal = RD_DONE;
  return 1;
}
int rd_cancel(RdOperation *o) {
  if (!o)
    return -1;
  if (o->terminal == RD_ACTIVE) {
    o->terminal = RD_CANCELLED;
    return 1;
  }
  return 0;
}
int rd_registry_init(RdRegistry *r, unsigned first, unsigned maximum) {
  if (!r || !first || first > maximum || maximum > 65535)
    return -1;
  *r = (RdRegistry){.maximum = maximum};
  for (unsigned i = 0; i < 4; ++i) {
    r->slots[i].generation = first;
    r->slots[i].descriptor = -1;
  }
  return 0;
}
static RdSlot *slot(RdRegistry *r, RdToken token) {
  if (!r || token.slot >= 4)
    return 0;
  RdSlot *s = &r->slots[token.slot];
#ifndef RD_MUTANT_GENERATION
  if (s->generation != token.generation)
    return 0;
#endif
  return s->active ? s : 0;
}
static void release(RdRegistry *r, RdSlot *s) {
  if (!s->closing || s->borrowers || s->pending)
    return;
  s->active = 0;
  s->descriptor = -1;
  if (s->generation == r->maximum) {
#ifdef RD_MUTANT_WRAP
    s->generation = 1;
#else
    s->retired = 1;
#endif
  } else
    ++s->generation;
}
int rd_register(RdRegistry *r, unsigned at, int fd, RdToken *out) {
  if (!r || !out || at >= 4 || fd < 0 || !r->maximum)
    return -1;
  RdSlot *s = &r->slots[at];
  if (s->active || s->retired)
    return 0;
  unsigned g = s->generation;
  *s = (RdSlot){.generation = g, .active = 1, .descriptor = fd};
  *out = (RdToken){at, g};
  return 1;
}
RdSlot *rd_borrow(RdRegistry *r, RdToken token) {
  RdSlot *s = slot(r, token);
  if (!s || s->closing || s->borrowers == 64)
    return 0;
  ++s->borrowers;
  return s;
}
int rd_return(RdRegistry *r, RdToken token) {
  RdSlot *s = slot(r, token);
  if (!s || !s->borrowers)
    return -1;
  --s->borrowers;
  release(r, s);
  return 0;
}
int rd_close(RdRegistry *r, RdToken token) {
  RdSlot *s = slot(r, token);
  if (!s)
    return 0;
  s->closing = 1;
  release(r, s);
  return 1;
}
int rd_submit(RdRegistry *r, RdToken token, unsigned direction,
              RdCompletion *out) {
  RdSlot *s = slot(r, token);
  if (!s || s->closing || direction > 1 || !out)
    return -1;
  unsigned bit = 1u << direction;
  if ((s->pending & bit) || s->submitted[direction] == 65535)
    return 0;
  RdOperation *op = direction ? &s->transmit : &s->receive;
  if (op->terminal != RD_ACTIVE || !op->goal)
    return -1;
  s->pending |= bit;
  ++s->submitted[direction];
  *out = (RdCompletion){token, direction, s->submitted[direction]};
  return 1;
}
int rd_complete(RdRegistry *r, RdCompletion completion, int amount,
                const unsigned char *rx) {
  RdSlot *s = slot(r, completion.connection);
  unsigned direction = completion.direction;
  if (!s || direction > 1)
    return 0;
#ifndef RD_MUTANT_OPERATION
  if (!completion.sequence || s->submitted[direction] != completion.sequence)
    return 0;
#endif
  unsigned bit = 1u << direction;
  if (!(s->pending & bit))
    return 0;
  RdOperation *op = direction ? &s->transmit : &s->receive;
  if (amount == -2 || amount < -1 || amount > (int)(op->goal - op->done) ||
      (!direction && amount > 0 && !rx))
    return -1;
  if (rd_progress(op, amount, direction ? 0 : rx) < 0)
    return -1;
  s->pending &= ~bit;
  release(r, s);
  return 1;
}
int rd_ready(RdScheduler *s, unsigned at) {
  if (!s || at >= 3)
    return -1;
  s->ready |= 1u << at;
  return 0;
}
int rd_turn(RdScheduler *s, unsigned budget, int (*cb)(void *, unsigned),
            void *ctx, RdTurn *out) {
  if (!s || !cb || !out || !budget || budget > 8 || s->cursor >= 3 ||
      s->ready > 7 || s->running)
    return -1;
  unsigned pending = s->ready, quota[3] = {0}, cursor = s->cursor;
  s->ready = 0;
  s->running = 1;
  RdTurn t = {0};
  /* Snapshot traversal: one round, up to two successes each. Callback work is
   * bounded even when success count does not advance. */
  for (unsigned visited = 0;
       visited < 3 && t.callbacks < budget && t.successes < 8; ++visited) {
    unsigned at = (cursor + visited) % 3, bit = 1u << at;
    if (!(pending & bit))
      continue;
    while (t.callbacks < budget && t.successes < 8 && quota[at] < 2) {
      t.order[t.callbacks++] = at;
      int result = cb(ctx, at);
      if (result == 1) {
        ++quota[at];
        ++t.successes;
      } else if (result == -1 || result == -2) {
        pending &= ~bit;
        break;
      } else if (result != 0) {
        s->running = 0;
        s->ready |= pending;
        return -1;
      }
#ifdef RD_MUTANT_WORK
      if (!result && budget < 8)
        ++budget;
#endif
    }
  }
  s->ready |= pending;
  s->cursor = (cursor + 1) % 3;
  s->running = 0;
  *out = t;
  return 0;
}
static int settle(RdService *s, RfMessage *m) {
  for (unsigned i = 0; i < s->accepted; ++i)
    if (s->records[i].id == m->id) {
      if (s->records[i].outcome != RF_OUTBOX)
        return -1;
      s->records[i].outcome = m->owner;
      ++s->terminal;
      return 0;
    }
  return -1;
}
static int cancel_connection(RdService *s, unsigned at) {
  RfConnection *c = &s->outbox.connections[at];
  RfMessage *saved[8];
  unsigned n = c->count;
  for (unsigned i = 0; i < n; ++i)
    saved[i] = c->items[(c->head + i) % 8];
  if (rf_disconnect(&s->outbox, at))
    return -1;
  for (unsigned i = 0; i < n; ++i)
    if (settle(s, saved[i]))
      return -1;
  return 0;
}
int rd_service_init(RdService *s, size_t budget) {
  if (!s || budget > 256)
    return -1;
  *s = (RdService){0};
  return rf_outbox_init(&s->outbox, budget);
}
int rd_service_open(RdService *s, unsigned *at) {
  if (!s || s->phase != RD_RUNNING)
    return 0;
  return rf_open(&s->outbox, at);
}
int rd_admit(RdService *s, unsigned at, RfMessage *m, enum RdPolicy policy) {
  if (!s || !m || at >= 4 || (unsigned)policy > RD_PAUSE_READ)
    return -1;
  if (s->phase != RD_RUNNING || s->accepted == 64 || (s->paused & (1u << at)))
    return 0;
  if (s->borrowers)
    return -1;
  for (unsigned i = 0; i < s->accepted; ++i)
    if (s->records[i].id == m->id)
      return -1;
  int result = rf_enqueue(&s->outbox, at, m);
  if (result == 1)
    s->records[s->accepted++] = (RdRecord){m->id, RF_OUTBOX};
  else if (result == 0) {
    if (policy == RD_DISCONNECT) {
      if (cancel_connection(s, at))
        return -1;
    }
    if (policy == RD_PAUSE_READ)
      s->paused |= 1u << at;
  }
  return result;
}
int rd_resume(RdService *s, unsigned at) {
  if (!s || at >= 4 || s->phase != RD_RUNNING ||
      !s->outbox.connections[at].active)
    return -1;
  if (s->outbox.connections[at].count >= 4)
    return 0;
  s->paused &= ~(1u << at);
  return 1;
}
int rd_commit(RdService *s, unsigned at, size_t n) {
  if (!s || at >= 4 || s->phase >= RD_CANCELLING)
    return -1;
  RfConnection *c = &s->outbox.connections[at];
  if (!c->count)
    return -1;
  RfMessage *m = c->items[c->head];
  if (rf_advance(&s->outbox, at, n))
    return -1;
  return m->owner == RF_DELIVERED ? settle(s, m) : 0;
}
int rd_service_borrow(RdService *s) {
  if (!s || s->phase >= RD_CANCELLING || s->borrowers == 64)
    return -1;
  ++s->borrowers;
  return 0;
}
static int finalize(RdService *s) {
  if (s->phase != RD_CANCELLING)
    return 0;
#ifndef RD_MUTANT_BORROW
  if (s->borrowers)
    return 0;
#endif
  for (unsigned at = 0; at < 4; ++at)
    if (cancel_connection(s, at))
      return -1;
  s->phase = RD_RELEASED;
  return 0;
}
int rd_service_return(RdService *s) {
  if (!s || !s->borrowers)
    return -1;
  --s->borrowers;
  return finalize(s);
}
int rd_tick(RdService *s, uint64_t now) {
  if (!s)
    return -1;
  if (s->phase == RD_RUNNING || s->phase == RD_RELEASED)
    return 0;
  if (s->phase == RD_DRAINING &&
      (now >= s->deadline || s->terminal == s->accepted)) {
    s->phase = RD_CANCELLING;
  }
#ifdef RD_MUTANT_EARLY_MESSAGES
  if (s->phase == RD_CANCELLING && s->borrowers)
    for (unsigned at = 0; at < 4; ++at)
      if (cancel_connection(s, at))
        return -1;
#endif
  return finalize(s);
}
int rd_stop(RdService *s, uint64_t now, uint64_t deadline) {
  if (!s)
    return -1;
  if (s->phase == RD_RUNNING) {
    s->phase = RD_DRAINING;
    s->deadline = deadline;
  } else if (s->phase == RD_DRAINING && deadline < s->deadline)
    s->deadline = deadline;
  return rd_tick(s, now);
}
