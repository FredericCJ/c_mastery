#include "history.h"
struct cf_state {
  unsigned values[8], count;
};
struct cf_search {
  const struct ch_op *ops;
  unsigned n, capacity, spec, event_mode;
  uint32_t budget, visited;
  unsigned limit;
};
static int before(const struct ch_op *a, const struct ch_op *b, unsigned events) {
  return !a->pending &&
         (a->respond < b->invoke || (events && a->respond == b->invoke &&
                                     a->respond_event < b->invoke_event));
}
static int step(struct cf_state *s, const struct ch_op *o, unsigned cap, unsigned spec,
                unsigned *r, unsigned *v) {
  *v = o->value;
  if (o->operation == CH_INSERT) {
    if (s->count == cap)
      *r = CH_FULL;
    else {
      s->values[s->count++] = o->value;
      *r = CH_VALUE;
    }
  } else if (!s->count)
    *r = CH_EMPTY;
  else {
    *r = CH_VALUE;
    if (spec == CH_STACK)
      *v = s->values[--s->count];
    else {
      *v = s->values[0];
      s->count--;
      for (unsigned i = 0; i < s->count; i++)
        s->values[i] = s->values[i + 1];
    }
  }
  return o->pending || (o->result == *r && (o->operation == CH_INSERT ||
                                            *r != CH_VALUE || o->value == *v));
}
static int search(struct cf_search *c, unsigned remaining, struct cf_state state, struct ch_witness *w,
                  unsigned depth) {
  if (c->visited == c->budget) {
    c->limit = 1;
    return 0;
  }
  c->visited++;
  if (!remaining) {
    w->count = depth;
    return 1;
  }
  for (unsigned i = 0; i < c->n; i++)
    if (remaining & (1u << i)) {
      unsigned blocked = 0;
      for (unsigned j = 0; j < c->n; j++)
        if ((remaining & (1u << j)) &&
            before(&c->ops[j], &c->ops[i], c->event_mode))
          blocked = 1;
#ifdef CH_MUTANT_PRECEDENCE
      blocked = 0;
#endif
      if (blocked)
        continue;
      struct cf_state next = state;
      if (!step(&next, &c->ops[i], c->capacity, c->spec, &w->result[i],
                &w->value[i]))
        continue;
      w->order[depth] = i;
      if (search(c, remaining & ~(1u << i), next, w, depth + 1))
        return 1;
      if (c->limit)
        return 0;
    }
  return 0;
}
int ch_check(const struct ch_op *ops, unsigned n, unsigned cap, unsigned spec,
             unsigned extend, unsigned events, uint32_t budget, struct ch_witness *out,
             uint32_t *visited) {
  if ((n && !ops) || !out || !visited || n > 10 || !cap || cap > 8 ||
      spec > CH_STACK || extend > 1 || events > 1 || !budget ||
      budget > 1000000)
    return CH_INVALID;
  unsigned pending = 0, complete = 0;
  for (unsigned i = 0; i < n; i++) {
    const struct ch_op *o = &ops[i];
    if (o->pending > 1 || o->participant > 9 || o->operation > CH_REMOVE ||
        (!o->pending && (o->result > CH_EMPTY || o->respond < o->invoke ||
                         (o->respond == o->invoke &&
                          (!events || o->respond_event <= o->invoke_event)) ||
                         (events && o->respond_event <= o->invoke_event))))
      return CH_INVALID;
    for (unsigned j = 0; j < i; j++) {
      const struct ch_op *p = &ops[j];
      if (o->participant == p->participant && !before(o, p, events) &&
          !before(p, o, events))
        return CH_INVALID;
    }
    if (events)
      for (unsigned j = 0; j < n; j++)
        for (unsigned a = 0; a < 2; a++)
          for (unsigned b = 0; b < 2; b++) {
            if ((a && o->pending) || (b && ops[j].pending) ||
                (i == j && a == b))
              continue;
            uint32_t ei = a ? o->respond_event : o->invoke_event,
                     ej = b ? ops[j].respond_event : ops[j].invoke_event;
            uint32_t ti = a ? o->respond : o->invoke,
                     tj = b ? ops[j].respond : ops[j].invoke;
            if (ei == ej || (ei < ej && ti > tj))
              return CH_INVALID;
          }
    if (o->pending)
      pending |= 1u << i;
    else
      complete |= 1u << i;
  }
  *visited = 0;
  if (pending && !extend)
    return CH_PENDING;
  struct cf_search c = {ops, n, cap, spec, events, budget, 0, 0};
  unsigned subset = pending;
#ifdef CH_MUTANT_DROP_PENDING
  subset = 0;
  pending = 0;
#endif
  for (;;) {
    struct cf_state state = {{0}, 0};
    struct ch_witness w = {0};
    w.selected_pending = subset;
    if (search(&c, complete | subset, state, &w, 0)) {
      w.visited = c.visited;
      *out = w;
      *visited = c.visited;
      return CH_VALID;
    }
    if (c.limit) {
      *visited = c.visited;
      return CH_LIMIT;
    }
    if (!subset)
      break;
    subset = (subset - 1u) & pending;
  }
  *visited = c.visited;
  return CH_NONLINEAR;
}
