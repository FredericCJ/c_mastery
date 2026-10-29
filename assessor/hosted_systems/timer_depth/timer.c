#include "timer.h"
int td_init(struct td_timer *t, unsigned limit) {
  if (!t || !limit || limit > 65535u)
    return -1;
  *t = (struct td_timer){.limit = limit};
  return 0;
}
int td_arm(struct td_timer *t, uint64_t deadline, uint64_t period, int skip,
           unsigned *generation) {
  if (!t || !generation || !deadline || (skip != 0 && skip != 1))
    return -1;
  if (t->state == TD_RUNNING || t->state == TD_CANCELLING)
    return 0;
  if (t->generation == t->limit) {
#ifdef TD_MUTANT_WRAP
    t->generation = 0;
#else
    if (t->state == TD_IDLE)
      t->state = TD_RETIRED;
    return 0;
#endif
  }
  ++t->generation;
  t->state = TD_ARMED;
  t->deadline = deadline;
  t->period = period;
  t->skip = skip;
  t->next = 0;
  t->exhausted = 0;
  *generation = t->generation;
  return 1;
}
int td_begin(struct td_timer *t, unsigned generation, uint64_t now, struct td_dispatch *out) {
  if (!t || !out)
    return -1;
#ifndef TD_MUTANT_GENERATION
  if (generation != t->generation)
    return 0;
#else
  (void)generation;
#endif
  if (t->state != TD_ARMED || now < t->deadline)
    return 0;
  struct td_dispatch d = {.generation = t->generation,
                  .scheduled = t->deadline,
                  .expirations = 1,
                  .effects = 1};
  if (t->period) {
    /* deadline>=1 implies difference<=UINT64_MAX-1, so +1 is safe. */
    d.expirations = (now - t->deadline) / t->period + 1;
#ifdef TD_MUTANT_COUNT
    d.expirations = 1;
#endif
    d.effects = t->skip ? 1 : d.expirations;
    if (d.expirations > (UINT64_MAX - t->deadline) / t->period)
      d.exhausted = 1;
    else
      d.next = t->deadline + d.expirations * t->period;
#ifdef TD_MUTANT_DRIFT
    if (t->period <= UINT64_MAX - now)
      d.next = now + t->period;
#endif
  }
  t->next = d.next;
  t->exhausted = d.exhausted;
  t->state = TD_RUNNING;
  *out = d;
  return 1;
}
int td_finish(struct td_timer *t, unsigned generation) {
  if (!t)
    return -1;
  if (generation != t->generation ||
      (t->state != TD_RUNNING && t->state != TD_CANCELLING))
    return 0;
  if (t->state == TD_CANCELLING || !t->period || t->exhausted)
    t->state = TD_IDLE;
  else {
    t->deadline = t->next;
    t->state = TD_ARMED;
  }
  return 1;
}
int td_cancel(struct td_timer *t) {
  if (!t)
    return -1;
  if (t->state == TD_RUNNING || t->state == TD_CANCELLING) {
#ifdef TD_MUTANT_EARLY_RELEASE
    t->state = TD_IDLE;
    return 1;
#else
    t->state = TD_CANCELLING;
    return 2;
#endif
  }
  if (t->state != TD_ARMED)
    return 0;
  t->state = TD_IDLE;
  return 1;
}
int td_earliest(const struct td_timer *timers, size_t count, size_t *index,
                uint64_t *deadline) {
  if ((!timers && count) || !index || !deadline)
    return -1;
  size_t best = count;
  for (size_t i = 0; i < count; ++i)
    if (timers[i].state == TD_ARMED &&
        (best == count || timers[i].deadline < timers[best].deadline))
      best = i;
  if (best == count)
    return 0;
  *index = best;
  *deadline = timers[best].deadline;
  return 1;
}
