#include "server.h"
#ifndef SS_MUTANT
#define SS_MUTANT 0
#endif
int ss_invariant(const SSBudget *s) {
  if (!s || !s->capacity || s->capacity > 16 ||
      (unsigned)s->merge > SS_DELAY_LATEST || !s->core.budget ||
      s->core.budget > s->core.period || s->core.period >= 0x80000000u ||
      s->core.remaining > s->core.budget || s->core.count > s->capacity)
    return 0;
  uint32_t sum = s->core.remaining, previous = 0;
  for (unsigned i = 0; i < s->core.count; ++i) {
    uint32_t distance = s->core.at[i] - s->core.last;
    if (!distance || distance > s->core.period || distance <= previous ||
        !s->core.amount[i] || s->core.amount[i] > s->core.budget - sum)
      return 0;
    previous = distance;
    sum += s->core.amount[i];
  }
  return sum == s->core.budget;
}
enum cmx_result ss_init(SSBudget *s, uint32_t q, uint32_t p, uint32_t start,
                        unsigned capacity, enum ss_merge merge) {
  if (!s || !capacity || capacity > 16 || (unsigned)merge > SS_DELAY_LATEST)
    return CMX_INVALID;
  SSBudget x = {0};
  enum cmx_result r = cmx_server_init(&x.core, q, p);
  if (r != CMX_OK) return r;
  x.capacity = capacity; x.merge = merge; x.core.last = start;
  if (!ss_invariant(&x)) return CMX_BROKEN;
  *s = x; return CMX_OK;
}
enum cmx_result ss_update(SSBudget *s, uint32_t now) {
  if (!ss_invariant(s)) return CMX_BROKEN;
  SSBudget x = *s;
  enum cmx_result r = cmx_server_update(&x.core, now);
  if (r != CMX_OK) return r;
  if (!ss_invariant(&x)) return CMX_BROKEN;
  *s = x; return CMX_OK;
}
enum cmx_result ss_charge(SSBudget *s, uint32_t now, uint32_t n) {
  if (!ss_invariant(s)) return CMX_BROKEN;
  SSBudget x = *s;
  enum cmx_result r = cmx_server_update(&x.core, now);
  if (r != CMX_OK) return r;
  if (SS_MUTANT == 3) *s = x; /* publishes due processing before refusal */
  if (!n) return CMX_INVALID;
  if (n > x.core.remaining) return CMX_FULL;
  unsigned count = x.core.count;
  uint32_t when = now + x.core.period;
  if (count == x.capacity && x.core.at[count - 1] != when) {
    if (x.merge == SS_REJECT) return CMX_FULL;
    /* All credits are later than now. The latest is moved later again,
     * never advancing eligibility; n is bounded by uncommitted capacity. */
    x.core.at[count - 1] = SS_MUTANT == 4 ? x.core.at[0] : when;
    x.core.amount[count - 1] += n;
    x.core.remaining -= n;
  } else {
    r = cmx_server_charge(&x.core, now, n);
    if (r != CMX_OK) return r;
  }
  if (SS_MUTANT == 1) x.core.at[x.core.count - 1] = now;
  if (SS_MUTANT == 2) x.core.amount[x.core.count - 1] += n;
  if (SS_MUTANT != 1 && SS_MUTANT != 2 && SS_MUTANT != 4 && !ss_invariant(&x))
    return CMX_BROKEN;
  *s = x; return CMX_OK;
}
enum cmx_result ss_compare(uint32_t q, uint32_t p, uint32_t begin,
                           uint32_t end, const uint32_t *service, unsigned n,
                           SSComparison *out) {
  if (!out || !service || !n || n > 16 || n > q || !p ||
      p >= 0x80000000u || !(uint32_t)(end - begin) ||
      (uint32_t)(end - begin) >= p) return CMX_INVALID;
  SSComparison x;
  enum cmx_result r = ss_init(&x.dispatch,q,p,begin,16,SS_REJECT);
  if (r != CMX_OK) return r;
  r = ss_init(&x.grouped,q,p,begin,16,SS_REJECT);
  if (r != CMX_OK) return r;
  uint32_t previous = 0;
  for (unsigned i = 0; i < n; ++i) {
    uint32_t offset = service[i] - begin;
    if (offset >= (uint32_t)(end-begin) || (i && offset <= previous))
      return CMX_INVALID;
    previous = offset;
    r = ss_charge(&x.dispatch,service[i],1);
    if (r != CMX_OK) return r;
  }
  r = ss_update(&x.dispatch,end);
  if (r != CMX_OK) return r;
  x.grouped.core.remaining -= n;
  x.grouped.core.last = end; x.grouped.core.count = 1;
  x.grouped.core.at[0] = (SS_MUTANT == 5 ? service[n-1] : begin) + p;
  x.grouped.core.amount[0] = n;
  if (!ss_invariant(&x.grouped)) return CMX_BROKEN;
  *out = x; return CMX_OK;
}
enum cmx_result ss_workload(const SSWorkload *w, enum ss_policy policy,
                            SSTrace *out) {
  if (!w || !out || (unsigned)policy > SS_DISPATCH ||
      w->count > SS_REQUESTS || !w->capacity || w->capacity > 8 ||
      !w->horizon || w->horizon > SS_HORIZON || !w->q ||
      w->q > w->period || w->period > SS_HORIZON) return CMX_INVALID;
  for (unsigned i = 0; i < w->count; ++i)
    if (w->request[i].release >= w->horizon || w->request[i].cost > 1024)
      return CMX_INVALID;
  SSTrace x = {0};
  SSBudget budget;
  enum cmx_result result = ss_init(&budget,w->q,w->period,0,16,SS_DELAY_LATEST);
  if (result != CMX_OK) return result;
  for (unsigned i = 0; i < SS_REQUESTS; ++i) x.finish[i] = SS_NONE;
  unsigned active = 0, periodic_left = 0, poll_checked = 0;
  uint32_t poll_budget = w->q;
  for (unsigned t = 0; t < w->horizon; ++t) {
    x.service[t] = SS_NONE;
    if (!(t % 10)) {
      if (periodic_left) { ++x.periodic_misses; x.periodic_dropped += periodic_left; }
      periodic_left = 2;
    }
    if (!(t % w->period)) { poll_budget = w->q; poll_checked = 0; }
    uint32_t before = budget.core.remaining;
    result = ss_update(&budget,t);
    if (result != CMX_OK) return result;
    x.replenished[t] = budget.core.remaining - before;
    for (unsigned i = 0; i < w->count; ++i) if (w->request[i].release == t) {
      if (!w->request[i].cost) { x.admitted[i] = 1; x.finish[i] = t; }
      else if (active == w->capacity) x.rejected[i] = 1;
      else { x.admitted[i] = 1; x.remaining[i] = w->request[i].cost; ++active; }
    }
    unsigned chosen = SS_REQUESTS;
    for (unsigned i = 0; i < w->count; ++i)
      if (x.remaining[i] && (chosen == SS_REQUESTS ||
          w->request[i].release < w->request[chosen].release)) chosen = i;
    int supervisor = !(t % 5);
    if (policy == SS_POLLING && !supervisor && !poll_checked) {
      poll_checked = 1;
      if (chosen == SS_REQUESTS && SS_MUTANT != 6) poll_budget = 0;
    }
    int ready = chosen != SS_REQUESTS &&
      (policy == SS_BACKGROUND || (policy == SS_POLLING ? poll_budget : budget.core.remaining));
    if (supervisor && SS_MUTANT != 7) { ++x.supervisor; x.service[t] = SS_REQUESTS; }
    else if (ready && (policy != SS_BACKGROUND || !periodic_left)) {
      if (policy == SS_DISPATCH) {
        result = ss_charge(&budget,t,1);
        if (result != CMX_OK) return result;
      } else if (policy == SS_POLLING) --poll_budget;
      --x.remaining[chosen]; ++x.executed[chosen]; x.service[t] = chosen;
      if (!x.remaining[chosen]) { --active; x.finish[chosen] = t+1; }
    } else if (periodic_left) {
      --periodic_left; ++x.periodic; x.service[t] = SS_REQUESTS+1;
    }
    x.budget[t] = policy == SS_DISPATCH ? budget.core.remaining : poll_budget;
    x.backlog[t] = active; x.records[t] = budget.core.count;
  }
  if (!(w->horizon % 10) && periodic_left) ++x.periodic_misses;
  x.periodic_remaining = periodic_left;
  *out = x; return CMX_OK;
}
