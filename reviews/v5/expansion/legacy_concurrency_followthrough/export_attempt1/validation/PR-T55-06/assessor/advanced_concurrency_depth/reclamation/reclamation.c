#include "reclamation.h"
#include <limits.h>
static void event(APQueue *q, unsigned point, unsigned p, unsigned n) {
  if (q->hook)
    q->hook(q->hook_context, point, p, n);
}
static int lease_valid(const APQueue *q, APLease lease) {
  return q && lease.owner == q && lease.participant < q->participants &&
         lease.generation && lease.generation <= (UINT_MAX >> 2) &&
         atomic_load(&q->registration[lease.participant]) ==
             ((lease.generation << 2) | 1u);
}
int ap_init(APQueue *q, unsigned participants, unsigned limit, APHook hook,
            void *context) {
  if (!q || !participants || participants > AP_PARTICIPANTS || !limit ||
      limit > (UINT_MAX >> 2))
    return AP_INVALID;
  q->participants = participants;
  q->generation_limit = limit;
  q->hook = hook;
  q->hook_context = context;
  atomic_init(&q->head, 0);
  atomic_init(&q->tail, 0);
  for (unsigned p = 0; p < AP_PARTICIPANTS; p++) {
    atomic_init(&q->registration[p], 4);
    for (unsigned h = 0; h < 2; h++)
      atomic_init(&q->hazard[p][h], AP_NONE);
  }
  for (unsigned n = 0; n < AP_NODES; n++) {
    atomic_init(&q->node[n].next, AP_NONE);
    atomic_init(&q->node[n].state, n ? 0u : 1u);
    q->node[n].value.sequence = 0;
    q->node[n].value.inverse = 0;
  }
  return AP_OK;
}
int ap_register(APQueue *q, APLease *out) {
  if (!q || !out)
    return AP_INVALID;
  for (unsigned p = 0; p < q->participants; p++) {
    unsigned old = atomic_load(&q->registration[p]);
    if ((old & 3u) == 0 &&
        atomic_compare_exchange_strong(&q->registration[p], &old, old | 1u)) {
      out->owner = q;
      out->participant = p;
      out->generation = old >> 2;
      return AP_OK;
    }
  }
  return AP_FULL;
}
static void clear(APQueue *q, unsigned p) {
  atomic_store(&q->hazard[p][0], AP_NONE);
  atomic_store(&q->hazard[p][1], AP_NONE);
}
int ap_leave(APQueue *q, APLease lease) {
  if (!lease_valid(q, lease))
    return AP_INVALID;
  unsigned old = (lease.generation << 2) | 1u;
  clear(q, lease.participant);
  unsigned next = lease.generation == (UINT_MAX >> 2)
                      ? (old | 3u)
                      : ((lease.generation + 1u) << 2);
#ifdef AP_MUTANT_REGISTRATION
  next = lease.generation << 2;
#endif
  return atomic_compare_exchange_strong(&q->registration[lease.participant],
                                        &old, next)
             ? AP_OK
             : AP_INVALID;
}
static unsigned protect(APQueue *q, unsigned p, unsigned h,
                        atomic_uint *location) {
  unsigned n;
  do {
    n = atomic_load(location);
    event(q, AP_LOADED, p, n);
    atomic_store(&q->hazard[p][h], n);
#ifdef AP_MUTANT_VALIDATION
  } while (0);
#else
  } while (n != atomic_load(location));
#endif
  event(q, AP_VALIDATED, p, n);
  return n;
}
unsigned ap_collect(APQueue *q) {
  unsigned count = 0;
  for (unsigned n = 0; n < AP_NODES; n++) {
    unsigned old = atomic_load(&q->node[n].state);
    if ((old & 3u) != 2u
#ifndef AP_MUTANT_GENERATION
        || (old >> 2) >= q->generation_limit
#endif
    )
      continue;
    event(q, AP_RETIRE_SNAPSHOT, AP_NONE, n);
    unsigned protected = 0;
    for (unsigned p = 0; p < q->participants; p++)
      for (unsigned h = 0; h < 2; h++)
        if (atomic_load(&q->hazard[p][h]) == n)
          protected = 1;
#ifdef AP_MUTANT_PROTECTION
    protected = 0;
#endif
    if (!protected) {
#ifdef AP_MUTANT_COLLECTOR
      atomic_store(&q->node[n].state, (old & ~3u) + 4u);
      count++;
#else
      if (atomic_compare_exchange_strong(&q->node[n].state, &old,
                                         (old & ~3u) + 4u))
        count++;
#endif
    }
  }
  return count;
}
int ap_push(APQueue *q, APLease lease, APValue value) {
  if (!lease_valid(q, lease))
    return AP_INVALID;
  unsigned n = AP_NONE, p = lease.participant;
  for (unsigned i = 0; i < AP_NODES; i++) {
#ifdef AP_RELAX_ALLOC
    unsigned old =
        atomic_load_explicit(&q->node[i].state, memory_order_relaxed);
#else
    unsigned old = atomic_load(&q->node[i].state);
#endif
    if ((old & 3u) == 0 &&
        atomic_compare_exchange_strong(&q->node[i].state, &old, old | 1u)) {
      n = i;
      break;
    }
  }
  if (n == AP_NONE)
    return AP_FULL;
  q->node[n].value.sequence = value.sequence;
  q->node[n].value.inverse = value.inverse;
  atomic_store(&q->node[n].next, AP_NONE);
  for (;;) {
    unsigned tail = protect(q, p, 0, &q->tail),
             next = atomic_load(&q->node[tail].next);
    if (tail != atomic_load(&q->tail))
      continue;
    if (next == AP_NONE) {
      unsigned empty = AP_NONE;
      if (atomic_compare_exchange_weak(&q->node[tail].next, &empty, n)) {
        event(q, AP_LINKED, p, n);
        atomic_compare_exchange_strong(&q->tail, &tail, n);
        clear(q, p);
        return AP_OK;
      }
    } else
      atomic_compare_exchange_weak(&q->tail, &tail, next);
  }
}
int ap_pop(APQueue *q, APLease lease, APValue *out) {
  if (!out || !lease_valid(q, lease))
    return AP_INVALID;
  unsigned p = lease.participant;
  for (;;) {
    unsigned head = protect(q, p, 0, &q->head), tail = atomic_load(&q->tail),
             next = protect(q, p, 1, &q->node[head].next);
    if (head != atomic_load(&q->head))
      continue;
    if (next == AP_NONE) {
      clear(q, p);
      return AP_RETRY;
    }
    if (head == tail) {
      atomic_compare_exchange_weak(&q->tail, &tail, next);
      continue;
    }
    APValue item;
    item.sequence = q->node[next].value.sequence;
    item.inverse = q->node[next].value.inverse;
    unsigned old = head;
    if (atomic_compare_exchange_weak(&q->head, &old, next)) {
      out->sequence = item.sequence;
      out->inverse = item.inverse;
      atomic_fetch_add(&q->node[head].state, 1u);
      clear(q, p);
      return AP_OK;
    }
  }
}
