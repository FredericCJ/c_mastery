#include "workflow.h"
#include <limits.h>
#ifndef SW_MUTANT
#define SW_MUTANT 0
#endif
static const char key[] = "workflow";
static int valid(store *s, Vh *a, Vh *b) {
  return s && a && b && a != b && a->database && b->database &&
         s != a->database && s != b->database && a->database != b->database &&
         a->mode == VH_VERSIONED5 && b->mode == VH_VERSIONED5 &&
         (a->capabilities & 8u) && (b->capabilities & 8u) && !s->poisoned &&
         !a->database->poisoned && !b->database->poisoned;
}
static void encode(uint8_t *p, int32_t amount, unsigned decline,
                   unsigned phase) {
  uint32_t n = (uint32_t)amount;
  p[0] = 1;
  p[1] = (uint8_t)phase;
  p[2] = (uint8_t)decline;
  for (unsigned i = 0; i < 4; i++)
    p[3 + i] = (uint8_t)(n >> (8 * i));
}
static int decode(const uint8_t *p, size_t n, int32_t *amount,
                  unsigned *decline, unsigned *phase) {
  if (n != 7 || p[0] != 1 || p[1] < SW_PREPARED || p[1] > SW_REVERSED ||
      p[2] > 1)
    return 0;
  uint32_t a = (uint32_t)p[3] | (uint32_t)p[4] << 8 | (uint32_t)p[5] << 16 |
               (uint32_t)p[6] << 24;
  if (!a || a > INT32_MAX)
    return 0;
  *amount = (int32_t)a;
  *decline = p[2];
  *phase = p[1];
  if ((*phase == SW_COMPENSATING || *phase == SW_REVERSED) && !*decline)
    return 0;
  if (*phase == SW_FINISHED && *decline)
    return 0;
  return 1;
}
static int save(store *s, int32_t amount, unsigned decline, unsigned phase) {
  uint8_t p[7];
  encode(p, amount, decline, phase);
  int r = store_begin(s);
  if (r)
    return r;
  r = store_put(s, key, sizeof key - 1, p, sizeof p);
  if (r) {
    store_abort(s);
    return r;
  }
  return store_commit(s);
}
int sw_create(SwFlow *f, store *s, Vh *a, Vh *b, int32_t amount,
              unsigned decline) {
  if (!f || !valid(s, a, b) || amount <= 0 || decline > 1)
    return SW_INVALID;
  uint8_t p[64];
  size_t n;
  int r = store_get(s, key, sizeof key - 1, p, sizeof p, &n);
  if (r == STORE_OK)
    return SW_CONFLICT;
  if (r != STORE_NOT_FOUND)
    return SW_RECOVER;
  /* Refuse preexisting participant operation identities; assignment values are
   * read by vh_host when the effects are admitted. */
  r = store_get(a->database, "vm-operation", 12, p, sizeof p, &n);
  if (r != STORE_NOT_FOUND)
    return r == STORE_OK ? SW_CONFLICT : SW_RECOVER;
  r = store_get(b->database, "vm-operation", 12, p, sizeof p, &n);
  if (r != STORE_NOT_FOUND)
    return r == STORE_OK ? SW_CONFLICT : SW_RECOVER;
  if (save(s, amount, decline, SW_PREPARED))
    return SW_RECOVER;
  *f = (SwFlow){s, a, b, amount, decline, SW_PREPARED, 0, STORE_OK};
  return SW_PROGRESS;
}
int sw_open(SwFlow *f, store *s, Vh *a, Vh *b) {
  if (!f || !valid(s, a, b))
    return SW_INVALID;
  uint8_t p[64];
  size_t n;
  int r = store_get(s, key, sizeof key - 1, p, sizeof p, &n);
  if (r)
    return SW_RECOVER;
  int32_t amount;
  unsigned decline, phase;
  if (!decode(p, n, &amount, &decline, &phase))
    return SW_INVALID;
  *f = (SwFlow){s, a, b, amount, decline, phase, 0, STORE_OK};
  return SW_PROGRESS;
}
static int effect(Vh *h, uint64_t id, int32_t delta) {
  Vh call = *h;
  call.operation_id = id;
  int32_t out;
  return vh_host(&call, 3, delta, &out);
}
static int failure(SwFlow *f, int r) {
  f->last_store_result = r;
  f->poisoned = 1;
  return SW_RECOVER;
}
int sw_step(SwFlow *f) {
  if (!f || f->poisoned || !valid(f->journal, f->debit, f->credit))
    return SW_RECOVER;
  if (f->phase == SW_FINISHED)
    return SW_DONE;
  if (f->phase == SW_REVERSED)
    return SW_COMPENSATED;
  unsigned next;
  int r;
  if (f->phase == SW_PREPARED) {
    if (SW_MUTANT == 1) {
      r = save(f->journal, f->amount, f->decline, SW_DEBITED);
      if (r)
        return failure(f, r);
    }
    r = effect(f->debit, 1, -f->amount);
    if (r)
      return failure(f, r);
    next = SW_DEBITED;
  } else if (f->phase == SW_DEBITED) {
    if (f->decline)
      next = SW_COMPENSATING;
    else {
      r = effect(f->credit, 1, f->amount);
      if (r) {
        if (SW_MUTANT == 3) {
          f->decline = 1;
          next = SW_COMPENSATING;
        } else
          return failure(f, r);
      } else
        next = SW_FINISHED;
    }
  } else if (f->phase == SW_COMPENSATING) {
    r = effect(f->debit, SW_MUTANT == 2 ? 1 : 2, f->amount);
    if (r)
      return failure(f, r);
    next = SW_REVERSED;
  } else
    return SW_INVALID;
  r = save(f->journal, f->amount, f->decline, next);
  if (r && SW_MUTANT != 4)
    return failure(f, r);
  f->phase = next;
  return next == SW_FINISHED   ? SW_DONE
         : next == SW_REVERSED ? SW_COMPENSATED
                               : SW_PROGRESS;
}
