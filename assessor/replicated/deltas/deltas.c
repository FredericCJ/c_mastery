#include "deltas.h"
#include "hashing.h"
#ifndef RPD_MUTATION
#define RPD_MUTATION 0
#endif
static int one_valid(const RPSet *s, int half) {
  unsigned i;
  uint16_t seen = 0, allowed = 0;
  if (s->next[0] > 8 || s->next[1] > (half ? 0u : 8u))
    return 0;
  allowed =
      (uint16_t)(((1u << s->next[0]) - 1) | (((1u << s->next[1]) - 1) << 8));
  for (i = 0; i < 4; ++i) {
    uint16_t bits = (uint16_t)(s->adds[i] | s->removed[i]);
    if (bits & ~allowed || bits & seen)
      return 0;
    seen |= bits;
  }
  return 1;
}
static int valid(const RpdSet *s) {
  return s && one_valid(&s->first, 0) && one_valid(&s->second, 1);
}
static int same(const RpdSet *a, const RpdSet *b) {
  unsigned i;
  for (i = 0; i < 4; ++i)
    if (a->first.adds[i] != b->first.adds[i] ||
        a->first.removed[i] != b->first.removed[i] ||
        a->second.adds[i] != b->second.adds[i] ||
        a->second.removed[i] != b->second.removed[i])
      return 0;
  for (i = 0; i < 2; ++i)
    if (a->first.next[i] != b->first.next[i] ||
        a->second.next[i] != b->second.next[i])
      return 0;
  return 1;
}
int rpd_set_join(const RpdSet *s, const RpdSet *d, RpdSet *out) {
  RpdSet t;
  if (!out || !valid(s) || !valid(d))
    return RPD_INVALID;
  t = *s;
  rp_set_merge(&t.first, &d->first);
  rp_set_merge(&t.second, &d->second);
  if (RPD_MUTATION == 1) {
    unsigned i;
    for (i = 0; i < 4; ++i) {
      t.first.removed[i] = s->first.removed[i];
      t.second.removed[i] = s->second.removed[i];
    }
  }
  if (!valid(&t))
    return RPD_INVALID;
  *out = t;
  return RPD_OK;
}
int rpd_set_add(const RpdSet *s, unsigned writer, unsigned value, RpdSet *out,
                RpdSet *delta) {
  RpdSet t, d = {0};
  int r;
  if (!out || !delta || out == delta || !valid(s) || writer > 2 || value > 3)
    return RPD_INVALID;
  t = *s;
  r = writer < 2 ? rp_set_add(&t.first, writer, value, &d.first)
                 : rp_set_add(&t.second, 0, value, &d.second);
  if (r != RP_OK)
    return r == RP_FULL ? RPD_FULL : RPD_INVALID;
  *out = t;
  *delta = d;
  return RPD_OK;
}
int rpd_set_remove(const RpdSet *s, unsigned value, RpdSet *out,
                   RpdSet *delta) {
  RpdSet t, d = {0};
  unsigned i;
  if (!out || !delta || out == delta || !valid(s) || value > 3)
    return RPD_INVALID;
  t = *s;
  (void)rp_set_remove(&t.first, value, &d.first);
  (void)rp_set_remove(&t.second, value, &d.second);
  /* Carry known counter frontiers so every serialized delta is structurally
     self-contained, including remove-before-add delivery. */
  for (i = 0; i < 2; ++i) {
    d.first.next[i] = s->first.next[i];
    d.second.next[i] = s->second.next[i];
  }
  if (RPD_MUTATION == 2) {
    d.second.next[0] = 8;
    d.second.removed[value] = 255;
    rp_set_merge(&t.second, &d.second);
  }
  *out = t;
  *delta = d;
  return RPD_OK;
}
int rpd_contains(const RpdSet *s, unsigned value) {
  return valid(s) && value < 4 &&
         (rp_set_contains(&s->first, value) ||
          rp_set_contains(&s->second, value));
}
static int state_valid(const RpdState *s) {
  return s && s->uuid && s->epoch && s->self < 3 && valid(&s->set);
}
static void put64(unsigned char *p, uint64_t x) {
  unsigned i;
  for (i = 0; i < 8; ++i)
    p[i] = (unsigned char)(x >> (56 - 8 * i));
}
static uint64_t get64(const unsigned char *p) {
  uint64_t x = 0;
  unsigned i;
  for (i = 0; i < 8; ++i)
    x = x << 8 | p[i];
  return x;
}
static void set_wire(const RPSet *s, unsigned char *b) {
  unsigned i;
  for (i = 0; i < 4; ++i) {
    b[2 * i] = (unsigned char)(s->adds[i] >> 8);
    b[2 * i + 1] = (unsigned char)s->adds[i];
    b[8 + 2 * i] = (unsigned char)(s->removed[i] >> 8);
    b[9 + 2 * i] = (unsigned char)s->removed[i];
  }
  b[16] = (unsigned char)s->next[0];
  b[17] = (unsigned char)s->next[1];
}
static void set_read(const unsigned char *b, RPSet *s) {
  unsigned i;
  for (i = 0; i < 4; ++i) {
    s->adds[i] = (uint16_t)((unsigned)b[2 * i] << 8 | b[2 * i + 1]);
    s->removed[i] = (uint16_t)((unsigned)b[8 + 2 * i] << 8 | b[9 + 2 * i]);
  }
  s->next[0] = b[16];
  s->next[1] = b[17];
}
int rpd_encode(const RpdState *s, unsigned char out[128]) {
  unsigned char b[128] = {0};
  unsigned i;
  if (!out || !state_valid(s))
    return RPD_INVALID;
  b[0] = 'R';
  b[1] = 'P';
  b[2] = 'D';
  b[3] = '1';
  b[4] = 1;
  b[5] = (unsigned char)s->self;
  put64(b + 8, s->uuid);
  put64(b + 16, s->epoch);
  put64(b + 24, s->sequence);
  set_wire(&s->set.first, b + 32);
  set_wire(&s->set.second, b + 56);
  (void)rph_hash(b, 96, b + 96);
  for (i = 0; i < 128; ++i)
    out[i] = b[i];
  return RPD_OK;
}
int rpd_decode(const unsigned char b[128], RpdState *out) {
  RpdState t = {0};
  unsigned char hash[32];
  unsigned i;
  if (!b || !out)
    return RPD_INVALID;
  if (b[0] != 'R' || b[1] != 'P' || b[2] != 'D' || b[3] != '1' || b[4] != 1 ||
      b[5] > 2 || b[6] || b[7])
    return RPD_CORRUPT;
  for (i = 50; i < 56; ++i)
    if (b[i] && RPD_MUTATION != 10)
      return RPD_CORRUPT;
  for (i = 74; i < 96; ++i)
    if (b[i])
      return RPD_CORRUPT;
  (void)rph_hash(b, 96, hash);
  if (RPD_MUTATION != 3 && !rph_equal(hash, b + 96))
    return RPD_CORRUPT;
  t.self = b[5];
  t.uuid = get64(b + 8);
  t.epoch = get64(b + 16);
  t.sequence = get64(b + 24);
  set_read(b + 32, &t.set.first);
  set_read(b + 56, &t.set.second);
  if (!state_valid(&t))
    return RPD_CORRUPT;
  *out = t;
  return RPD_OK;
}
static int save(store *p, const RpdState *s) {
  unsigned char b[128];
  int r;
  unsigned i;
  if (!p || !state_valid(s))
    return RPD_INVALID;
  if (p->active || p->poisoned)
    return RPD_STATE;
  (void)rpd_encode(s, b);
  r = store_begin(p);
  for (i = 0; !r && i < 2; ++i) {
    unsigned char key[2] = {'D', (unsigned char)i};
    r = store_put(p, key, 2, b + 64 * i, 64);
  }
  if (!r)
    r = store_commit(p);
  if (r)
    store_abort(p);
  return !r                ? RPD_OK
         : p->poisoned     ? RPD_UNCERTAIN
         : r == STORE_FULL ? RPD_FULL
                           : RPD_IO;
}
static int load(store *p, RpdState *out) {
  unsigned char b[128];
  unsigned i;
  if (!p || !out)
    return RPD_INVALID;
  if (p->active || p->poisoned)
    return RPD_STATE;
  for (i = 0; i < 2; ++i) {
    unsigned char key[2] = {'D', (unsigned char)i};
    size_t n = 0;
    int r = store_get(p, key, 2, b + 64 * i, 64, &n);
    if (r || n != 64)
      return r == STORE_IO ? RPD_IO : RPD_CORRUPT;
  }
  return rpd_decode(b, out);
}
static int current(RpdReplica *r) {
  RpdState s;
  unsigned char a[128], b[128];
  unsigned i;
  int status;
  if (!r || !state_valid(&r->state) || r->count > 4)
    return RPD_INVALID;
  status = load(r->pages, &s);
  if (status)
    return status;
  (void)rpd_encode(&r->state, a);
  (void)rpd_encode(&s, b);
  for (i = 0; i < 128; ++i)
    if (a[i] != b[i])
      return RPD_STATE;
  return RPD_OK;
}
int rpd_create(store *p, uint64_t uuid, unsigned self, RpdReplica *out) {
  RpdReplica r = {0};
  int status;
  unsigned i;
  if (!p || !out || !uuid || self > 2)
    return RPD_INVALID;
  if (p->active || p->poisoned)
    return RPD_STATE;
  for (i = 0; i < 2; ++i) {
    unsigned char key[2] = {'D', (unsigned char)i}, bytes[64];
    size_t n = 0;
    status = store_get(p, key, 2, bytes, 64, &n);
    if (status != STORE_NOT_FOUND)
      return status == STORE_IO ? RPD_IO : RPD_STATE;
  }
  r.pages = p;
  r.state.uuid = uuid;
  r.state.self = self;
  r.state.epoch = 1;
  status = save(p, &r.state);
  if (!status)
    *out = r;
  return status;
}
int rpd_open(store *p, uint64_t uuid, unsigned self, RpdReplica *out) {
  RpdReplica r = {0};
  int status;
  if (!out || !uuid || self > 2)
    return RPD_INVALID;
  status = load(p, &r.state);
  if (status)
    return status;
  if (r.state.uuid != uuid || r.state.self != self)
    return RPD_IDENTITY;
  if (r.state.epoch == UINT64_MAX)
    return RPD_RANGE;
  if (RPD_MUTATION != 6)
    ++r.state.epoch;
  r.pages = p;
  status = save(p, &r.state);
  if (!status)
    *out = r;
  return status;
}
static void append(RpdReplica *r, uint64_t before, const RpdSet *d) {
  unsigned i;
  if (r->count == 4) {
    for (i = 1; i < 4; ++i)
      r->history[i - 1] = r->history[i];
    --r->count;
  }
  r->history[r->count++] = (RpdEntry){before, *d};
}
static int publish(RpdReplica *r, const RpdSet *next, const RpdSet *delta) {
  RpdState s = r->state;
  int status;
  if (s.sequence == UINT64_MAX)
    return RPD_RANGE;
  s.set = *next;
  ++s.sequence;
  if (RPD_MUTATION == 4)
    r->state = s;
  status = save(r->pages, &s);
  if (status)
    return status;
  append(r, s.sequence - 1, delta);
  r->state = s;
  return RPD_OK;
}
int rpd_local(RpdReplica *r, unsigned add, unsigned value) {
  RpdSet next, delta;
  int status;
  if (add > 1 || value > 3)
    return RPD_INVALID;
  status = current(r);
  if (status)
    return status;
  status = add ? rpd_set_add(&r->state.set, r->state.self, value, &next, &delta)
               : rpd_set_remove(&r->state.set, value, &next, &delta);
  if (status)
    return status;
  return publish(r, &next, &delta);
}
int rpd_send(RpdReplica *r, unsigned peer, uint64_t peer_epoch,
             RpdPacket *out) {
  RpdPacket p = {0};
  RpdMark a;
  unsigned i;
  int status;
  if (!out || peer > 2 || !peer_epoch)
    return RPD_INVALID;
  status = current(r);
  if (status)
    return status;
  if (peer == r->state.self)
    return RPD_INVALID;
  a = r->acked[peer];
  if (a.epoch == peer_epoch && a.upto == r->state.sequence)
    return RPD_EMPTY;
  p.uuid = r->state.uuid;
  p.sender = r->state.self;
  p.target = peer;
  p.sender_epoch = r->state.epoch;
  p.target_epoch = peer_epoch;
  p.end = r->state.sequence;
  p.full =
      a.epoch != peer_epoch || !r->count || a.upto < r->history[0].sequence;
  if (RPD_MUTATION == 5)
    p.full = 0;
  if (p.full)
    p.body = r->state.set;
  else {
    p.start = a.upto;
    for (i = 0; i < r->count; ++i)
      if (r->history[i].sequence >= p.start) {
        RpdSet t;
        status = rpd_set_join(&p.body, &r->history[i].delta, &t);
        if (status)
          return status;
        p.body = t;
      }
  }
  r->sent[peer] = (RpdMark){peer_epoch, p.end};
  *out = p;
  return RPD_OK;
}
int rpd_receive(RpdReplica *r, const RpdPacket *p, RpdAck *out) {
  RpdSet next;
  RpdAck ack;
  RpdMark seen;
  int status;
  if (!p || !out || p->sender > 2 || p->target > 2 || p->full > 1 ||
      !p->sender_epoch || !p->target_epoch || p->start > p->end ||
      !valid(&p->body))
    return RPD_INVALID;
  status = current(r);
  if (status)
    return status;
  if (p->uuid != r->state.uuid || p->target != r->state.self ||
      p->sender == r->state.self)
    return RPD_IDENTITY;
  if (p->target_epoch != r->state.epoch)
    return RPD_STALE;
  seen = r->received[p->sender];
  if (p->sender_epoch < seen.epoch)
    return RPD_STALE;
  if (RPD_MUTATION != 8 && !p->full &&
      (p->sender_epoch != seen.epoch || p->start > seen.upto))
    return RPD_CAUSAL;
  if (p->full && p->start)
    return RPD_INVALID;
  status = rpd_set_join(&r->state.set, &p->body, &next);
  if (status)
    return status;
  ack = (RpdAck){r->state.uuid, r->state.epoch, p->sender_epoch,
                 p->end,        r->state.self,  p->sender};
  if (RPD_MUTATION == 7)
    *out = ack;
  if (!same(&next, &r->state.set)) {
    status = publish(r, &next, &p->body);
    if (status)
      return status;
  }
  if (seen.epoch != p->sender_epoch)
    seen = (RpdMark){p->sender_epoch, 0};
  if (p->end > seen.upto)
    seen.upto = p->end;
  r->received[p->sender] = seen;
  *out = ack;
  return RPD_OK;
}
int rpd_ack(RpdReplica *r, uint64_t peer_epoch, const RpdAck *a) {
  RpdMark *mark;
  int status;
  if (!a || a->from > 2 || a->to > 2 || !peer_epoch)
    return RPD_INVALID;
  status = current(r);
  if (status)
    return status;
  if (a->uuid != r->state.uuid || a->to != r->state.self ||
      a->from == r->state.self)
    return RPD_IDENTITY;
  if (RPD_MUTATION != 9 &&
      (a->to_epoch != r->state.epoch || a->from_epoch != peer_epoch ||
       r->sent[a->from].epoch != peer_epoch))
    return RPD_STALE;
  if (a->upto > r->state.sequence || a->upto > r->sent[a->from].upto)
    return RPD_INVALID;
  mark = &r->acked[a->from];
  if (mark->epoch != peer_epoch)
    *mark = (RpdMark){peer_epoch, 0};
  if (a->upto > mark->upto)
    mark->upto = a->upto;
  return RPD_OK;
}
