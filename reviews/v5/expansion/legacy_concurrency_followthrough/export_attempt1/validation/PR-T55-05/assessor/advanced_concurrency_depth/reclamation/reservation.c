#include "reservation.h"
enum { FREE, RESERVED, REQUESTED, PUBLISHED, CANCELLED, CONSUMING };
static unsigned tag(unsigned ticket, unsigned state) {
  return ((ticket + 1u) << 3) | state;
}
void ar_init(ARQueue *q, APHook hook, void *context) {
  atomic_init(&q->head, 0);
  atomic_init(&q->tail, 0);
  for (unsigned i = 0; i < AR_CAPACITY; i++) {
    atomic_init(&q->cells[i].state, FREE);
    q->cells[i].value.sequence = q->cells[i].value.inverse = 0;
  }
  q->hook = hook;
  q->context = context;
}
int ar_reserve(ARQueue *q, ARTicket *out) {
  if (!q || !out)
    return AR_INVALID;
  unsigned head = atomic_load(&q->head);
  for (;;) {
    if (head >= AR_LIMIT)
      return AR_EXHAUSTED;
    unsigned tail = atomic_load(&q->tail);
    if (head < tail) {
      head = atomic_load(&q->head);
      continue;
    }
    if (head - tail >= AR_CAPACITY)
      return AR_FULL;
    if (atomic_compare_exchange_weak(&q->head, &head, head + 1u)) {
      atomic_store(&q->cells[head % AR_CAPACITY].state, tag(head, RESERVED));
      out->owner = q;
      out->sequence = head;
      return AR_OK;
    }
  }
}
static int valid(ARQueue *q, ARTicket t) {
  return q && t.owner == q && t.sequence < AR_LIMIT &&
         t.sequence >= atomic_load(&q->tail) &&
         t.sequence < atomic_load(&q->head);
}
int ar_cancel(ARQueue *q, ARTicket t) {
  if (!valid(q, t))
    return AR_INVALID;
  ARCell *cell = &q->cells[t.sequence % AR_CAPACITY];
  unsigned expected = tag(t.sequence, RESERVED);
#ifdef AR_MUTANT_CANCEL
  unsigned desired = tag(t.sequence, CANCELLED);
#else
  unsigned desired = tag(t.sequence, REQUESTED);
#endif
  if (atomic_compare_exchange_strong(&cell->state, &expected, desired))
    return AR_OK;
  if (expected == tag(t.sequence, REQUESTED))
    return AR_OK;
  if (expected == tag(t.sequence, PUBLISHED) ||
      expected == tag(t.sequence, CONSUMING) ||
      expected == tag(t.sequence, CANCELLED))
    return AR_TOO_LATE;
  return AR_INVALID;
}
int ar_finish(ARQueue *q, ARTicket t, APValue v) {
  if (!valid(q, t))
    return AR_INVALID;
  ARCell *cell = &q->cells[t.sequence % AR_CAPACITY];
  unsigned old = atomic_load(&cell->state);
  if (old != tag(t.sequence, RESERVED) && old != tag(t.sequence, REQUESTED))
    return AR_INVALID;
  cell->value.sequence = v.sequence;
  cell->value.inverse = v.inverse;
  if (q->hook)
    q->hook(q->context, AP_LINKED, 0, t.sequence);
  old = tag(t.sequence, RESERVED);
  if (atomic_compare_exchange_strong(&cell->state, &old,
                                     tag(t.sequence, PUBLISHED)))
    return AR_OK;
  if (old == tag(t.sequence, REQUESTED)) {
    atomic_store(&cell->state, tag(t.sequence, CANCELLED));
    return AR_CANCELLED;
  }
  return AR_INVALID;
}
int ar_pop(ARQueue *q, APValue *out) {
  if (!q || !out)
    return AR_INVALID;
  unsigned tail = atomic_load(&q->tail);
  if (tail == atomic_load(&q->head))
    return AR_EMPTY;
  ARCell *cell = &q->cells[tail % AR_CAPACITY];
  unsigned old = atomic_load(&cell->state);
  int cancelled = old == tag(tail, CANCELLED);
  if (!cancelled && old != tag(tail, PUBLISHED)) {
#ifdef AR_MUTANT_PENDING
    return AR_EMPTY;
#else
    return AR_PENDING;
#endif
  }
  if (!atomic_compare_exchange_strong(&cell->state, &old, tag(tail, CONSUMING)))
    return AR_PENDING;
  APValue v = {0, 0};
  if (!cancelled) {
    v.sequence = cell->value.sequence;
    v.inverse = cell->value.inverse;
  }
  atomic_store(&cell->state, FREE);
  atomic_store(&q->tail, tail + 1u);
  if (cancelled)
    return AR_CANCELLED;
  out->sequence = v.sequence;
  out->inverse = v.inverse;
  return AR_OK;
}
