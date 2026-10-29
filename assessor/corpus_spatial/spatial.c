#include "spatial.h"
#ifndef SP_MUTANT
#define SP_MUTANT 0
#endif
#define NIL UINT32_MAX
static void zero(void *p, unsigned long n) {
  volatile unsigned char *q = p;
  while (n--)
    *q++ = 0;
}
static void copy(void *p, const void *s, unsigned long n) {
  volatile unsigned char *q = p;
  const volatile unsigned char *r = s;
  while (n--)
    *q++ = *r++;
}
static uint32_t bit(unsigned i) { return UINT32_C(1) << i; }
static int validbox(struct sp_box b, unsigned d) {
  for (unsigned k = 0; k < 3; k++)
    if (b.lo[k] < -1024 || b.hi[k] > 1024 || b.lo[k] > b.hi[k] ||
        (k >= d && (b.lo[k] != 0 || b.hi[k] != 0)))
      return 0;
  return 1;
}
static int overlap(struct sp_box a, struct sp_box b, unsigned d) {
  for (unsigned k = 0; k < d; k++)
    if (a.lo[k] > b.hi[k] || a.hi[k] < b.lo[k])
      return 0;
  return 1;
}
static struct sp_box join(struct sp_box a, struct sp_box b, unsigned d) {
  for (unsigned k = 0; k < d; k++) {
    if (b.lo[k] < a.lo[k])
      a.lo[k] = b.lo[k];
    if (b.hi[k] > a.hi[k])
      a.hi[k] = b.hi[k];
  }
  return a;
}
static uint64_t volume(struct sp_box a, unsigned d) {
  uint64_t v = 1;
  for (unsigned k = 0; k < d; k++)
    v *= (uint64_t)(a.hi[k] - a.lo[k]);
  return v;
}
static unsigned allocnode(struct sp_index *t) {
  for (unsigned i = 0; i < t->budget; i++)
    if (!t->nodes[i].active) {
      struct sp_node *n = &t->nodes[i];
      zero(n, sizeof *n);
      n->active = 1;
      for (unsigned k = 0; k < 8; k++)
        n->child[k] = NIL;
      t->used++;
      return i;
    }
  return NIL;
}
static struct sp_box maskbox(const struct sp_index *t, uint32_t m) {
  struct sp_box b = {{0, 0, 0}, {0, 0, 0}};
  unsigned found = 0;
  for (unsigned i = 0; i < 32; i++)
    if (m & bit(i)) {
      b = found ? join(b, t->items[i].box, t->dim) : t->items[i].box;
      found = 1;
    }
  return b;
}
static unsigned population(uint32_t m) {
  unsigned n = 0;
  for (; m; m &= m - 1)
    n++;
  return n;
}
static int less(const struct sp_index *t, unsigned a, unsigned b, unsigned axis) {
  int32_t x = t->items[a].box.lo[axis], y = t->items[b].box.lo[axis];
  if (t->kind == SP_BVH) {
    x += t->items[a].box.hi[axis];
    y += t->items[b].box.hi[axis];
  }
  return x < y || (x == y && a < b);
}
static unsigned binary(struct sp_index *t, uint32_t mask, unsigned depth) {
  if (!mask)
    return NIL;
  unsigned at = allocnode(t);
  if (at == NIL)
    return NIL;
  struct sp_node *n = &t->nodes[at];
  n->box = maskbox(t, mask);
  n->mask = mask;
  unsigned ids[32], count = 0, axis = depth % t->dim;
  if (t->kind == SP_BVH) {
    axis = 0;
    for (unsigned k = 1; k < t->dim; k++)
      if (n->box.hi[k] - n->box.lo[k] > n->box.hi[axis] - n->box.lo[axis])
        axis = k;
  }
  for (unsigned i = 0; i < 32; i++)
    if (mask & bit(i)) {
      unsigned j = count++;
      while (j && less(t, i, ids[j - 1], axis)) {
        ids[j] = ids[j - 1];
        j--;
      }
      ids[j] = i;
    }
  n->axis = axis;
  n->item = ids[count / 2];
  if (t->kind == SP_BVH && count == 1) {
    n->leaf = 1;
    return at;
  }
  uint32_t a = 0, b = 0;
  for (unsigned i = 0; i < count; i++) {
    if (i < count / 2)
      a |= bit(ids[i]);
    else if (t->kind == SP_BVH || i > count / 2)
      b |= bit(ids[i]);
  }
  if (a) {
    n->child[0] = binary(t, a, depth + 1);
    if (n->child[0] == NIL)
      return NIL;
  }
  if (b) {
    n->child[1] = binary(t, b, depth + 1);
    if (n->child[1] == NIL)
      return NIL;
  }
  return at;
}
static unsigned quad(struct sp_index *t, uint32_t mask, struct sp_box cell, unsigned side) {
  if (!mask)
    return NIL;
  unsigned at = allocnode(t);
  if (at == NIL)
    return NIL;
  struct sp_node *n = &t->nodes[at];
  n->box = cell;
  n->mask = mask;
  if (side == 1 || population(mask) <= 1) {
    n->leaf = 1;
    if (SP_MUTANT == 6)
      n->mask = mask & (~mask + 1u);
    return at;
  }
  unsigned half = side / 2;
  for (unsigned c = 0; c < (1u << t->dim); c++) {
    struct sp_box sub = cell;
    for (unsigned k = 0; k < t->dim; k++) {
      int32_t mid = cell.lo[k] + (int32_t)half;
      if (c & (1u << k))
        sub.lo[k] = mid;
      else
        sub.hi[k] = mid - 1;
    }
    uint32_t m = 0;
    for (unsigned i = 0; i < 32; i++)
      if ((mask & bit(i)) && overlap(sub, t->items[i].box, t->dim))
        m |= bit(i);
    if (SP_MUTANT == 5 && c == 0)
      m = 0;
    if (m) {
      n->child[c] = quad(t, m, sub, half);
      if (n->child[c] == NIL)
        return NIL;
    }
  }
  return at;
}
static struct sp_box entrybox(const struct sp_index *t, const struct sp_node *n, unsigned e) {
  return n->leaf ? t->items[e].box : t->nodes[e].box;
}
static void rbound(struct sp_index *t, unsigned at) {
  struct sp_node *n = &t->nodes[at];
  if (!n->count)
    return;
  struct sp_box first = entrybox(t, n, n->entry[0]);
  /* Explicit field transfer: Clang's overlapping aggregate assignment emitted
   * __aeabi_memmove4 in the strict target profile despite -fno-builtin. */
  for (unsigned k = 0; k < 3; k++) {
    volatile int32_t *lo = &n->box.lo[k], *hi = &n->box.hi[k];
    *lo = first.lo[k];
    *hi = first.hi[k];
  }
  for (unsigned i = 1; i < n->count; i++)
    n->box = join(n->box, entrybox(t, n, n->entry[i]), t->dim);
}
static uint64_t enlargement(const struct sp_index *t, struct sp_box a, struct sp_box b) {
  return volume(join(a, b, t->dim), t->dim) - volume(a, t->dim);
}
static unsigned rsplit(struct sp_index *t, unsigned at) {
  unsigned other = allocnode(t);
  if (other == NIL)
    return NIL;
  struct sp_node *n = &t->nodes[at], *o = &t->nodes[other];
  o->leaf = n->leaf;
  unsigned e[5];
  for (unsigned i = 0; i < 5; i++)
    e[i] = n->entry[i];
  unsigned a = 0, b = 1;
  int64_t worst = INT64_MIN;
  for (unsigned i = 0; i < 5; i++)
    for (unsigned j = i + 1; j < 5; j++) {
      struct sp_box x = entrybox(t, n, e[i]), y = entrybox(t, n, e[j]);
      int64_t waste = (int64_t)volume(join(x, y, t->dim), t->dim) -
                      (int64_t)volume(x, t->dim) - (int64_t)volume(y, t->dim);
      if (waste > worst) {
        worst = waste;
        a = i;
        b = j;
      }
    }
  n->count = o->count = 1;
  n->entry[0] = e[a];
  o->entry[0] = e[b];
  unsigned used = (1u << a) | (1u << b);
  rbound(t, at);
  rbound(t, other);
  while (used != 31) {
    unsigned pick = 0, dest = 0, remaining = 5 - population(used);
    uint64_t preference = 0;
    for (unsigned i = 0; i < 5; i++)
      if (!(used & (1u << i))) {
        struct sp_box box = entrybox(t, n, e[i]);
        uint64_t x = enlargement(t, n->box, box),
                 y = enlargement(t, o->box, box), diff = x > y ? x - y : y - x;
        if (diff >= preference) {
          preference = diff;
          pick = i;
          dest = x > y || (x == y &&
                           (volume(n->box, t->dim) > volume(o->box, t->dim) ||
                            (volume(n->box, t->dim) == volume(o->box, t->dim) &&
                             n->count > o->count)));
        }
      }
    if (n->count + remaining == 2)
      dest = 0;
    else if (o->count + remaining == 2)
      dest = 1;
    struct sp_node *d = dest ? o : n;
    d->entry[d->count++] = e[pick];
    used |= 1u << pick;
    rbound(t, dest ? other : at);
  }
  return other;
}
/* Return 0 success, 1 allocation refusal; partner is NIL unless this node
 * split. */
static int rinsert(struct sp_index *t, unsigned at, unsigned id, unsigned *partner) {
  struct sp_node *n = &t->nodes[at];
  *partner = NIL;
  if (n->leaf)
    n->entry[n->count++] = id;
  else {
    unsigned best = 0;
    for (unsigned i = 1; i < n->count; i++) {
      struct sp_box a = t->nodes[n->entry[i]].box, b = t->nodes[n->entry[best]].box;
      uint64_t x = enlargement(t, a, t->items[id].box),
               y = enlargement(t, b, t->items[id].box);
      if (x < y || (x == y && volume(a, t->dim) < volume(b, t->dim)))
        best = i;
    }
    unsigned extra;
    if (rinsert(t, n->entry[best], id, &extra))
      return 1;
    if (extra != NIL)
      n->entry[n->count++] = extra;
  }
  rbound(t, at);
  if (n->count > 4) {
    *partner = rsplit(t, at);
    if (*partner == NIL)
      return 1;
  }
  return 0;
}
static int radd(struct sp_index *t, unsigned id) {
  if (t->root == NIL) {
    t->root = allocnode(t);
    if (t->root == NIL)
      return 1;
    t->nodes[t->root].leaf = 1;
  }
  unsigned extra;
  if (rinsert(t, t->root, id, &extra))
    return 1;
  if (extra != NIL) {
    unsigned old = t->root, n = allocnode(t);
    if (n == NIL)
      return 1;
    t->root = n;
    t->nodes[n].count = 2;
    t->nodes[n].entry[0] = old;
    t->nodes[n].entry[1] = extra;
    rbound(t, n);
  }
  return 0;
}
static void collect(struct sp_index *t, unsigned at, uint32_t *orphan) {
  struct sp_node *n = &t->nodes[at];
  if (n->leaf) {
    for (unsigned i = 0; i < n->count; i++)
      *orphan |= bit(n->entry[i]);
  } else
    for (unsigned i = 0; i < n->count; i++)
      collect(t, n->entry[i], orphan);
  n->active = 0;
  t->used--;
}
static int rremove(struct sp_index *t, unsigned at, unsigned id, uint32_t *orphan) {
  struct sp_node *n = &t->nodes[at];
  for (unsigned i = 0; i < n->count; i++) {
    int found =
        n->leaf ? (n->entry[i] == id) : rremove(t, n->entry[i], id, orphan);
    if (!found)
      continue;
    if (n->leaf || t->nodes[n->entry[i]].count < 2) {
      if (!n->leaf)
        collect(t, n->entry[i], orphan);
      for (unsigned j = i + 1; j < n->count; j++)
        n->entry[j - 1] = n->entry[j];
      n->count--;
    }
    rbound(t, at);
    return 1;
  }
  return 0;
}
static int32_t floordiv(int32_t x, unsigned w) {
  int32_t q = x / (int32_t)w;
  if (SP_MUTANT != 9 && x % (int32_t)w < 0)
    q--;
  return q;
}
static unsigned hashcell(const int32_t c[3]) {
  return ((uint32_t)c[0] * UINT32_C(73856093) ^
          (uint32_t)c[1] * UINT32_C(19349663) ^
          (uint32_t)c[2] * UINT32_C(83492791)) %
         SP_BUCKETS;
}
static int gridadd(struct sp_index *t, unsigned id) {
  struct sp_box b = t->items[id].box;
  int32_t low[3], hi[3];
  uint64_t need = 1;
  for (unsigned k = 0; k < 3; k++) {
    low[k] = floordiv(b.lo[k], t->width);
    hi[k] = floordiv(b.hi[k], t->width);
    need *= (uint64_t)(hi[k] - low[k] + 1);
  }
  if (need > t->budget - t->used)
    return 1;
  for (int32_t x = low[0]; x <= hi[0]; x++)
    for (int32_t y = low[1]; y <= hi[1]; y++)
      for (int32_t z = low[2]; z <= hi[2]; z++) {
        struct sp_ref *r = &t->refs[t->used];
        r->cell[0] = x;
        r->cell[1] = y;
        r->cell[2] = z;
        r->item = id;
        unsigned h = hashcell(r->cell);
        r->next = t->buckets[h];
        t->buckets[h] = t->used++;
      }
  return 0;
}
static int itemvalid(struct sp_item x, unsigned kind, unsigned dim) {
  if (x.id >= 32 || !validbox(x.box, dim))
    return 0;
  for (unsigned k = 0; k < dim; k++) {
    if ((kind == SP_KD || kind == SP_QUAD) && x.box.lo[k] != x.box.hi[k])
      return 0;
    if (kind == SP_QUAD && (x.box.lo[k] < -16 || x.box.lo[k] >= 16))
      return 0;
  }
  return 1;
}
int sp_build(struct sp_index *out, struct sp_index *t, unsigned kind, unsigned dim,
             const struct sp_item *items, unsigned n, unsigned budget, unsigned width) {
  if (!out || !t || (!items && n) || n > 32 || kind < 1 || kind > 5 ||
      (dim != 2 && dim != 3) || budget > (kind == SP_GRID ? 256u : 128u) ||
      (kind == SP_GRID && (width == 0 || width > 16)))
    return SP_INVALID;
  zero(t, sizeof *t);
  t->kind = kind;
  t->dim = dim;
  t->budget = budget;
  t->width = width;
  t->root = NIL;
  for (unsigned i = 0; i < 17; i++)
    t->buckets[i] = NIL;
  for (unsigned i = 0; i < n; i++) {
    struct sp_item x = items[i];
    if (!itemvalid(x, kind, dim) || (t->live & bit(x.id)))
      return SP_INVALID;
    t->items[x.id] = x;
    t->live |= bit(x.id);
    t->count++;
  }
  if (kind == SP_KD || kind == SP_BVH)
    t->root = binary(t, t->live, 0);
  if (kind == SP_QUAD) {
    struct sp_box b = {{-16, -16, 0}, {15, 15, 0}};
    if (dim == 3) {
      b.lo[2] = -16;
      b.hi[2] = 15;
    }
    t->root = quad(t, t->live, b, 32);
  }
  if (kind <= SP_QUAD && n && t->root == NIL)
    return SP_FULL;
  if (kind == SP_RTREE)
    for (unsigned i = 0; i < n; i++)
      if (radd(t, items[i].id))
        return SP_FULL;
  if (kind == SP_GRID)
    for (unsigned i = 0; i < n; i++)
      if (gridadd(t, items[i].id))
        return SP_FULL;
  copy(out, t, sizeof *out);
  return SP_OK;
}
int sp_edit(struct sp_index *t, struct sp_index *s, unsigned action, struct sp_item item) {
  if (!t || !s || action < 1 || action > 3 || item.id >= 32)
    return SP_INVALID;
  unsigned present = (t->live & bit(item.id)) != 0;
  if (action == 1 && present)
    return SP_INVALID;
  if (action != 1 && !present)
    return SP_MISSING;
  if (action != 3 && !itemvalid(item, t->kind, t->dim))
    return SP_INVALID;
  if (t->kind != SP_RTREE) {
    struct sp_item list[32];
    unsigned n = 0;
    for (unsigned i = 0; i < 32; i++)
      if ((t->live & bit(i)) && i != item.id)
        list[n++] = t->items[i];
    if (action != 3)
      list[n++] = item;
    return sp_build(t, s, t->kind, t->dim, list, n, t->budget, t->width);
  }
  copy(s, t, sizeof *s);
  if (action != 1) {
    uint32_t orphan = 0;
    if (!rremove(s, s->root, item.id, &orphan))
      return SP_MISSING;
    s->live &= ~bit(item.id);
    s->count--;
    struct sp_node *r = &s->nodes[s->root];
    if (!r->count) {
      r->active = 0;
      s->used--;
      s->root = NIL;
    } else if (!r->leaf && r->count == 1) {
      unsigned old = s->root;
      s->root = r->entry[0];
      s->nodes[old].active = 0;
      s->used--;
    }
    if (SP_MUTANT == 8)
      orphan = 0;
    for (unsigned i = 0; i < 32; i++)
      if (orphan & bit(i))
        if (radd(s, i))
          return SP_FULL;
  }
  if (action != 3) {
    s->items[item.id] = item;
    s->live |= bit(item.id);
    s->count++;
    if (radd(s, item.id))
      return SP_FULL;
  }
  copy(t, s, sizeof *t);
  return SP_OK;
}
static struct sp_box refit(struct sp_index *t, unsigned at) {
  struct sp_node *n = &t->nodes[at];
  if (n->leaf)
    n->box = t->items[n->item].box;
  else {
    struct sp_box a = refit(t, n->child[0]), b = refit(t, n->child[1]);
    if (SP_MUTANT != 4)
      n->box = join(a, b, t->dim);
  }
  return n->box;
}
int sp_refit(struct sp_index *t, struct sp_index *s, struct sp_item item) {
  if (!t || !s || t->kind != SP_BVH || !itemvalid(item, t->kind, t->dim))
    return SP_INVALID;
  if (!(t->live & bit(item.id)))
    return SP_MISSING;
  copy(s, t, sizeof *s);
  s->items[item.id] = item;
  (void)refit(s, s->root);
  copy(t, s, sizeof *t);
  return SP_OK;
}
static void querytree(const struct sp_index *t, unsigned at, struct sp_box q, struct sp_result *r) {
  if (at == NIL)
    return;
  const struct sp_node *n = &t->nodes[at];
  r->visits++;
  if (!overlap(n->box, q, t->dim))
    return;
  if (t->kind == SP_KD) {
    r->tests++;
    if (overlap(t->items[n->item].box, q, t->dim))
      r->mask |= bit(n->item);
    querytree(t, n->child[0], q, r);
    querytree(t, n->child[1], q, r);
  } else if (t->kind == SP_BVH) {
    if (n->leaf) {
      r->tests++;
      if (SP_MUTANT != 3 || t->items[n->item].box.hi[0] != q.lo[0])
        r->mask |= bit(n->item);
    } else {
      querytree(t, n->child[0], q, r);
      querytree(t, n->child[1], q, r);
    }
  } else if (t->kind == SP_QUAD) {
    if (n->leaf) {
      for (unsigned i = 0; i < 32; i++)
        if (n->mask & bit(i)) {
          r->tests++;
          if (overlap(t->items[i].box, q, t->dim))
            r->mask |= bit(i);
        }
    } else
      for (unsigned i = 0; i < (1u << t->dim); i++)
        querytree(t, n->child[i], q, r);
  } else
    for (unsigned i = 0; i < n->count; i++) {
      if (n->leaf) {
        unsigned id = n->entry[i];
        r->tests++;
        if (overlap(t->items[id].box, q, t->dim))
          r->mask |= bit(id);
      } else {
        querytree(t, n->entry[i], q, r);
        if (SP_MUTANT == 7)
          break;
      }
    }
}
int sp_query(const struct sp_index *t, struct sp_box q, struct sp_result *out) {
  if (!t || !out || !validbox(q, t->dim))
    return SP_INVALID;
  struct sp_result r = {0, 0, 0};
  if (t->kind != SP_GRID)
    querytree(t, t->root, q, &r);
  else {
    /* Probe occupied cells: cost remains bounded even for a huge query window.
     */
    int32_t low[3], high[3];
    for (unsigned k = 0; k < 3; k++) {
      low[k] = floordiv(q.lo[k], t->width);
      high[k] = floordiv(q.hi[k], t->width);
    }
    uint32_t seen = 0;
    for (unsigned h = 0; h < 17; h++)
      for (unsigned j = t->buckets[h]; j != NIL; j = t->refs[j].next) {
        const struct sp_ref *x = &t->refs[j];
        r.visits++;
        unsigned in = 1;
        for (unsigned k = 0; k < 3; k++)
          if (x->cell[k] < low[k] || x->cell[k] > high[k])
            in = 0;
        if (in && !(seen & bit(x->item))) {
          seen |= bit(x->item);
          r.tests++;
          if (overlap(t->items[x->item].box, q, t->dim))
            r.mask |= bit(x->item);
        }
      }
  }
  *out = r;
  return SP_OK;
}
static uint64_t distance(struct sp_box b, const int32_t *p, const unsigned *w,
                         unsigned d) {
  uint64_t sum = 0;
  for (unsigned k = 0; k < d; k++) {
    int64_t v = p[k] < b.lo[k]   ? (int64_t)b.lo[k] - p[k]
                : p[k] > b.hi[k] ? (int64_t)p[k] - b.hi[k]
                                 : 0;
    sum += (uint64_t)(v * v) * w[k];
  }
  return sum;
}
static void nearest(const struct sp_index *t, unsigned at, const int32_t *p,
                    const unsigned *w, struct sp_nearest *r) {
  if (at == NIL)
    return;
  const struct sp_node *n = &t->nodes[at];
  r->visits++;
  uint64_t bound = distance(n->box, p, w, t->dim);
  if (bound > r->distance || (SP_MUTANT == 1 && bound == r->distance))
    return;
  if (t->kind == SP_KD || n->leaf) {
    unsigned id = n->item;
    uint64_t d = distance(t->items[id].box, p, w, t->dim);
    r->tests++;
    if (d < r->distance || (d == r->distance && id < r->id)) {
      r->distance = d;
      r->id = id;
    }
  }
  if (t->kind == SP_KD || !n->leaf) {
    unsigned a = n->child[0], b = n->child[1];
    if (a != NIL && b != NIL &&
        distance(t->nodes[b].box, p, w, t->dim) <
            distance(t->nodes[a].box, p, w, t->dim)) {
      unsigned tmp = a;
      a = b;
      b = tmp;
    }
    nearest(t, a, p, w, r);
    nearest(t, b, p, w, r);
  }
}
int sp_nearest(const struct sp_index *t, const int32_t p[3], const unsigned w[3],
               struct sp_nearest *out) {
  if (!t || !p || !w || !out || (t->kind != SP_KD && t->kind != SP_BVH))
    return SP_INVALID;
  for (unsigned k = 0; k < 3; k++)
    if (p[k] < -1024 || p[k] > 1024 || w[k] < 1 || w[k] > 4 ||
        (k >= t->dim && p[k]))
      return SP_INVALID;
  if (!t->count)
    return SP_MISSING;
  struct sp_nearest r = {NIL, 0, 0, UINT64_MAX};
  unsigned weights[3];
  for (unsigned k = 0; k < 3; k++)
    weights[k] = SP_MUTANT == 2 ? 1 : w[k];
  nearest(t, t->root, p, weights, &r);
  *out = r;
  return SP_OK;
}
int sp_pairs(const struct sp_index *t, uint32_t rows[32], unsigned *candidates) {
  if (!t || !rows || !candidates || t->kind != SP_GRID)
    return SP_INVALID;
  uint32_t result[32], seen[32];
  for (unsigned i = 0; i < 32; i++)
    result[i] = seen[i] = 0;
  unsigned count = 0;
  for (unsigned h = 0; h < 17; h++)
    for (unsigned a = t->buckets[h]; a != NIL; a = t->refs[a].next)
      for (unsigned b = t->refs[a].next; b != NIL; b = t->refs[b].next) {
        const struct sp_ref *x = &t->refs[a], *y = &t->refs[b];
        if (x->item == y->item)
          continue;
        unsigned same = 1;
        for (unsigned k = 0; k < 3; k++)
          if (x->cell[k] != y->cell[k])
            same = 0;
        if (!same)
          continue;
        unsigned i = x->item < y->item ? x->item : y->item,
                 j = x->item < y->item ? y->item : x->item;
        if (!(seen[i] & bit(j)) || SP_MUTANT == 10) {
          count++;
          seen[i] |= bit(j);
          if (overlap(t->items[i].box, t->items[j].box, t->dim))
            result[i] |= bit(j);
        }
      }
  for (unsigned i = 0; i < 32; i++)
    rows[i] = result[i];
  *candidates = count;
  return SP_OK;
}
