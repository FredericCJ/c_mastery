#include "support.h"
#ifndef LC_MUTANT
#define LC_MUTANT 0
#endif
void lc_destroy(struct lc_pair *p) {
  if (!p)
    return;
  for (unsigned id = 3; id; --id)
    if (p->mask & (1u << id)) {
      if (!(LC_MUTANT == 3 && id == 1))
        p->ops.release(p->ops.context, id);
      p->mask &= ~(1u << id);
    }
}
int lc_construct(struct lc_pair *out, struct lc_ops ops, int optional) {
  if (!out || !ops.acquire || !ops.release)
    return 0;
  struct lc_pair stage = {0, ops};
  for (unsigned id = 1; id <= 3; ++id) {
    if (id == 2 && !optional)
      continue;
    if (!ops.acquire(ops.context, id)) {
      lc_destroy(&stage);
      return 0;
    }
    stage.mask |= 1u << id;
  }
  *out = stage;
  return 1;
}
int lc_construct_nested(struct lc_pair *out, struct lc_ops ops, int optional) {
  if (!out || !ops.acquire || !ops.release)
    return 0;
  if (ops.acquire(ops.context, 1)) {
    if (!optional || ops.acquire(ops.context, 2)) {
      if (ops.acquire(ops.context, 3)) {
        *out = (struct lc_pair){optional ? 14u : 10u, ops};
        return 1;
      }
      if (optional)
        ops.release(ops.context, 2);
    }
    ops.release(ops.context, 1);
  }
  return 0;
}
static int acquire_step(struct lc_pair *p, unsigned id, int optional) {
  if (id == 2 && !optional)
    return 1;
  if (!p->ops.acquire(p->ops.context, id))
    return 0;
  p->mask |= 1u << id;
  return 1;
}
int lc_construct_helpers(struct lc_pair *out, struct lc_ops ops, int optional) {
  if (!out || !ops.acquire || !ops.release)
    return 0;
  struct lc_pair p = {0, ops};
  if (acquire_step(&p, 1, optional) && acquire_step(&p, 2, optional) &&
      acquire_step(&p, 3, optional)) {
    *out = p;
    return 1;
  }
  lc_destroy(&p);
  return 0;
}
