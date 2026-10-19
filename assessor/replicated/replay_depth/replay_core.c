#include "replay_depth.h"
#include <string.h>
#ifndef RPR_MUTATION
#define RPR_MUTATION 0
#endif
static int move(int x) { return x >= -1 && x <= 1; }
static int state(RPState s, unsigned tick) {
  return s.tick == tick && s.position < 16 && s.score <= 65535;
}
static int equal(RPState a, RPState b) {
  return a.tick == b.tick && a.position == b.position && a.score == b.score &&
         a.random == b.random;
}
static int presentation(const RPState s[2], int32_t *out) {
  uint32_t value =
      s[0].position + 16u * s[1].position + 256u * (s[0].score + s[1].score);
  if (value > INT32_MAX / 2)
    return RPR_RANGE;
  *out = (int32_t)value;
  return RPR_OK;
}
static int rebuild(RprSession *s, unsigned from) {
  unsigned i;
  int predicted = 0;
  RPEffect effect;
  for (i = 0; i < from; ++i)
    if (s->known[i])
      predicted = s->remote[i];
  for (i = from; i < s->head; ++i) {
    if (s->known[i])
      predicted = s->remote[i];
    else
      s->remote[i] = predicted;
    if (rp_step(s->snapshots[i][0], s->local[i], &s->snapshots[i + 1][0],
                &effect) != RP_OK ||
        rp_step(s->snapshots[i][1], s->remote[i], &s->snapshots[i + 1][1],
                &effect) != RP_OK)
      return RPR_RANGE;
  }
  s->confirmed = 0;
  while (s->confirmed < s->head && s->known[s->confirmed])
    ++s->confirmed;
  return RPR_OK;
}
int rpr_invariant(const RprSession *s) {
  RprSession next;
  unsigned i;
  int32_t value;
  if (!s || !s->timeline || !s->revision || s->horizon < 1 || s->horizon > 8 ||
      s->head > 8 || s->confirmed > s->head ||
      s->head - s->confirmed > s->horizon || s->effects.pending > 1 ||
      s->effects.next > s->confirmed)
    return 0;
  for (i = 0; i < 2; ++i)
    if (!state(s->initial[i], 0) || !equal(s->initial[i], s->snapshots[0][i]))
      return 0;
  for (i = 0; i < s->head; ++i)
    if (!move(s->local[i]) || !move(s->remote[i]) || s->known[i] > 1)
      return 0;
  next = *s;
  if (rebuild(&next, 0) || next.confirmed != s->confirmed)
    return 0;
  for (i = 0; i < s->head; ++i)
    if (next.remote[i] != s->remote[i] ||
        !equal(next.snapshots[i + 1][0], s->snapshots[i + 1][0]) ||
        !equal(next.snapshots[i + 1][1], s->snapshots[i + 1][1]))
      return 0;
  if (s->effects.pending) {
    if (!s->effects.next || s->effects.effect.id != s->effects.next ||
        presentation(s->snapshots[s->effects.next], &value) ||
        s->effects.effect.value != value * 2)
      return 0;
  }
  return 1;
}
int rpr_init(uint64_t timeline, unsigned horizon, const RPState initial[2],
             RprSession *out) {
  RprSession s = {0};
  if (!timeline || horizon < 1 || horizon > 8 || !initial || !out ||
      !state(initial[0], 0) || !state(initial[1], 0))
    return RPR_INVALID;
  s.timeline = timeline;
  s.revision = 1;
  s.horizon = horizon;
  s.initial[0] = s.snapshots[0][0] = initial[0];
  s.initial[1] = s.snapshots[0][1] = initial[1];
  *out = s;
  return RPR_OK;
}
int rpr_value(const RprSession *s, unsigned tick, int32_t *out) {
  if (!out || !rpr_invariant(s) || tick > s->head)
    return RPR_INVALID;
  return presentation(s->snapshots[tick], out);
}
static RprNotice notice(const RprSession *old, const RprSession *next,
                        unsigned from) {
  RprNotice n = {from, next->head, old->revision, next->revision, 0, 0};
  (void)presentation(old->snapshots[old->head], &n.old_value);
  (void)presentation(next->snapshots[next->head], &n.new_value);
  return n;
}
int rpr_advance(RprSession *s, int local, RprNotice *out) {
  RprSession next;
  RprNotice n;
  int r;
  if (!out || !move(local) || !rpr_invariant(s))
    return RPR_INVALID;
  if (s->head == 8)
    return RPR_FULL;
  if (s->revision == UINT32_MAX)
    return RPR_RANGE;
  if (s->head - s->confirmed >= s->horizon && RPR_MUTATION != 5)
    return RPR_WAIT;
  next = *s;
  next.local[next.head] = local;
  next.known[next.head] = 0;
  ++next.head;
  ++next.revision;
  r = rebuild(&next, s->head);
  if (r)
    return r;
  n = notice(s, &next, next.head);
  *s = next;
  *out = n;
  return RPR_OK;
}
int rpr_correct(RprSession *s, uint64_t timeline, unsigned tick, int remote,
                RprNotice *out) {
  RprSession next;
  RprNotice n;
  int r;
  if (!out || !move(remote) || !rpr_invariant(s) || !tick || tick > s->head)
    return RPR_INVALID;
  if (timeline != s->timeline)
    return RPR_STALE;
  if (s->known[tick - 1]) {
    if (s->remote[tick - 1] != remote)
      return RPR_CONFLICT;
    n = notice(s, s, 0);
    *out = n;
    return RPR_OK;
  }
  if (s->head - tick >= s->horizon)
    return RPR_STALE;
  if (s->revision == UINT32_MAX)
    return RPR_RANGE;
  next = *s;
  next.remote[tick - 1] = remote;
  next.known[tick - 1] = 1;
  ++next.revision;
  r = rebuild(&next, RPR_MUTATION == 1 ? s->head : tick - 1);
  if (r)
    return r;
  n = notice(s, &next, tick);
  *s = next;
  *out = n;
  return RPR_OK;
}
int rpr_prepare(RprSession *s, RprEffect *out) {
  RprSession next;
  ArEffect effect;
  int32_t value;
  int r;
  if (!out || !rpr_invariant(s))
    return RPR_INVALID;
  next = *s;
  if (next.effects.pending) {
    if (ar_retry(&next.effects, &effect) != AR_OK)
      return RPR_STATE;
  } else {
    if (next.effects.next == (RPR_MUTATION == 2 ? next.head : next.confirmed))
      return RPR_EMPTY;
    r = presentation(next.snapshots[next.effects.next + 1], &value);
    if (r)
      return r;
    if (ar_decide(&next.effects, value, &effect) != AR_OK)
      return RPR_STATE;
  }
  *s = next;
  *out = (RprEffect){s->timeline, effect.id, effect.value};
  return RPR_OK;
}
int rpr_feedback(RprSession *s, RprEffect effect, unsigned result) {
  RprSession next;
  if (!rpr_invariant(s) || !s->effects.pending)
    return RPR_STATE;
  if (RPR_MUTATION != 8 &&
      (effect.timeline != s->timeline || effect.tick != s->effects.effect.id ||
       effect.value != s->effects.effect.value))
    return RPR_STALE;
  if (result != AR_OK && result != AR_FAILED && result != AR_PENDING)
    return RPR_INVALID;
  next = *s;
  (void)ar_feedback(&next.effects, result);
  *s = next;
  return result == AR_OK ? RPR_OK : RPR_IO;
}
