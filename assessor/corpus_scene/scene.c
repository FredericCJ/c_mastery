#include "scene.h"
#include <limits.h>
#ifndef CS_MUTANT
#define CS_MUTANT 0
#endif
static void matrix_copy(CsMatrix *d, const CsMatrix *s) {
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 4; j++)
      ((volatile int32_t *)d->m[i])[j] = s->m[i][j];
}
static void identity(CsMatrix *m) {
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 4; j++)
      m->m[i][j] = (i == j);
}
static int local_ok(const CsMatrix *m) {
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 4; j++) {
      int32_t b = j == 3 ? 1024 : 16;
      if (m->m[i][j] < -b || m->m[i][j] > b)
        return 0;
    }
  return 1;
}
static CsStatus mul(const CsMatrix *a, const CsMatrix *b, CsMatrix *out) {
  CsMatrix r;
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 4; j++) {
      int64_t x = j == 3 ? a->m[i][3] : 0;
      for (unsigned k = 0; k < 3; k++)
        x += (int64_t)a->m[i][k] * b->m[k][j];
      if (x < -1000000 || x > 1000000)
        return CS_RANGE;
      r.m[i][j] = (int32_t)x;
    }
  matrix_copy(out, &r);
  return CS_OK;
}
void cs_init(CsScene *s) {
  for (unsigned i = 0; i < CS_N; i++) {
    s->node[i].generation = 0;
    s->node[i].live = 0;
    s->node[i].dirty = 1;
    s->node[i].parent = CS_N;
    identity(&s->node[i].local);
    identity(&s->node[i].world);
  }
}
static int valid(const CsScene *s, CsHandle h) {
  return s && h.owner == s && h.slot < CS_N && s->node[h.slot].live &&
         s->node[h.slot].generation == h.generation;
}
static void dirty(CsScene *s, unsigned id) {
  for (unsigned i = 0; i < CS_N; i++)
    if (s->node[i].live) {
      unsigned p = i;
      while (p < CS_N) {
        if (p == id) {
          if (CS_MUTANT != 1 || i == id)
            s->node[i].dirty = 1;
          break;
        }
        p = s->node[p].parent;
      }
    }
}
CsStatus cs_add(CsScene *s, const CsHandle *parent, const CsMatrix *m,
                CsHandle *out) {
  if (!s || !m || !out || !local_ok(m))
    return CS_INVALID;
  if (parent && !valid(s, *parent))
    return CS_STALE;
  unsigned slot = 0;
  while (slot < CS_N &&
         (s->node[slot].live || s->node[slot].generation == UINT32_MAX))
    slot++;
  if (slot == CS_N)
    return CS_FULL;
  CsNode *p = &s->node[slot];
  matrix_copy(&p->local, m);
  p->parent = parent ? parent->slot : CS_N;
  p->generation++;
  p->live = 1;
  p->dirty = 1;
  out->owner = s;
  out->slot = slot;
  out->generation = p->generation;
  return CS_OK;
}
CsStatus cs_set(CsScene *s, CsHandle h, const CsMatrix *m) {
  if (!s || !m || !local_ok(m))
    return CS_INVALID;
  if (!valid(s, h))
    return CS_STALE;
  matrix_copy(&s->node[h.slot].local, m);
  dirty(s, h.slot);
  return CS_OK;
}
/* Stage only the dirty ancestor path; no graph/cache/output publication on
 * range refusal. */
CsStatus cs_world(CsScene *s, CsHandle h, CsMatrix *out, unsigned *work) {
  if (!s || !out || !work)
    return CS_INVALID;
  if (!valid(s, h))
    return CS_STALE;
  unsigned chain[CS_N], n = 0, p = h.slot;
  CsMatrix staged[CS_N], base;
  while (p < CS_N && s->node[p].dirty) {
    chain[n++] = p;
    p = s->node[p].parent;
  }
  if (p < CS_N)
    matrix_copy(&base, &s->node[p].world);
  else
    identity(&base);
  for (unsigned i = n; i > 0; i--) {
    CsStatus r = CS_MUTANT == 2
                     ? mul(&s->node[chain[i - 1]].local, &base, &staged[i - 1])
                     : mul(&base, &s->node[chain[i - 1]].local, &staged[i - 1]);
    if (r != CS_OK)
      return r;
    matrix_copy(&base, &staged[i - 1]);
  }
  for (unsigned i = 0; i < n; i++) {
    matrix_copy(&s->node[chain[i]].world, &staged[i]);
    s->node[chain[i]].dirty = 0;
  }
  matrix_copy(out, &base);
  *work = n;
  return CS_OK;
}
static void scene_copy(CsScene *d, const CsScene *s) {
  for (unsigned i = 0; i < CS_N; i++) {
    matrix_copy(&d->node[i].local, &s->node[i].local);
    matrix_copy(&d->node[i].world, &s->node[i].world);
    d->node[i].generation = s->node[i].generation;
    d->node[i].live = s->node[i].live;
    d->node[i].dirty = s->node[i].dirty;
    d->node[i].parent = s->node[i].parent;
  }
}
CsStatus cs_reparent(CsScene *s, CsHandle h, const CsHandle *parent,
                     unsigned keep) {
  if (!s || keep > 1)
    return CS_INVALID;
  if (!valid(s, h) || (parent && !valid(s, *parent)))
    return CS_STALE;
  unsigned p = parent ? parent->slot : CS_N;
  for (unsigned q = p; q < CS_N; q = s->node[q].parent)
    if (q == h.slot)
      return CS_INVALID;
  CsMatrix local;
  matrix_copy(&local, &s->node[h.slot].local);
  if (keep) {
    CsScene stage;
    scene_copy(&stage, s);
    CsHandle a = {&stage, h.slot, h.generation};
    CsMatrix world, pw, inv;
    unsigned work;
    CsStatus r = cs_world(&stage, a, &world, &work);
    if (r != CS_OK)
      return r;
    identity(&pw);
    if (parent) {
      CsHandle b = {&stage, p, parent->generation};
      r = cs_world(&stage, b, &pw, &work);
      if (r != CS_OK)
        return r;
    }
    /* Exact inverse only for a signed permutation linear transform. */
    for (unsigned i = 0; i < 3; i++) {
      unsigned row = 0, col = 0;
      for (unsigned j = 0; j < 3; j++) {
        int32_t x = pw.m[i][j], y = pw.m[j][i];
        if (x < -1 || x > 1 || y < -1 || y > 1)
          return CS_RANGE;
        row += (x != 0);
        col += (y != 0);
      }
      if (row != 1 || col != 1)
        return CS_RANGE;
    }
    for (unsigned i = 0; i < 3; i++) {
      int64_t t = 0;
      for (unsigned j = 0; j < 3; j++) {
        inv.m[i][j] = pw.m[j][i];
        t -= (int64_t)inv.m[i][j] * pw.m[j][3];
      }
      inv.m[i][3] = (int32_t)t;
    }
    r = mul(&inv, &world, &local);
    if (r != CS_OK || !local_ok(&local))
      return CS_RANGE;
  }
  matrix_copy(&s->node[h.slot].local, &local);
  s->node[h.slot].parent = p;
  dirty(s, h.slot);
  return CS_OK;
}
CsStatus cs_remove(CsScene *s, CsHandle h) {
  if (!s)
    return CS_INVALID;
  if (!valid(s, h))
    return CS_STALE;
  for (unsigned i = 0; i < CS_N; i++)
    if (s->node[i].live && s->node[i].parent == h.slot)
      return CS_BUSY;
  s->node[h.slot].live = 0;
  return CS_OK;
}

static int point_ok(NvPoint p) {
  return p.x >= -64 && p.x <= 64 && p.y >= -64 && p.y <= 64;
}
static int equal(NvPoint a, NvPoint b) { return a.x == b.x && a.y == b.y; }
static int32_t area(NvPoint a, NvPoint b, NvPoint c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
static int inside(const NvPolygon *p, NvPoint q) {
  for (unsigned i = 0; i < p->n; i++)
    if (area(p->p[i], p->p[(i + 1) % p->n], q) < 0)
      return 0;
  return 1;
}
static int polygon_ok(const NvPolygon *p) {
  if (p->n < 3 || p->n > NV_VERTS)
    return 0;
  for (unsigned i = 0; i < p->n; i++)
    if (!point_ok(p->p[i]))
      return 0;
  for (unsigned i = 0; i < p->n; i++)
    for (unsigned j = 0; j < p->n; j++)
      if (j != i && j != (i + 1) % p->n &&
          area(p->p[i], p->p[(i + 1) % p->n], p->p[j]) <= 0)
        return 0;
  return 1;
}
/* Separating-axis test: touching closed boundaries are permitted; interiors may
 * not overlap. */
static int separated(const NvPolygon *a, const NvPolygon *b) {
  for (unsigned i = 0; i < a->n; i++) {
    unsigned outside = 0;
    for (unsigned j = 0; j < b->n; j++)
      outside += (area(a->p[i], a->p[(i + 1) % a->n], b->p[j]) <= 0);
    if (outside == b->n)
      return 1;
  }
  return 0;
}
static unsigned shared(const NvPolygon *a, const NvPolygon *b, NvPoint *l,
                       NvPoint *r) {
  unsigned n = 0;
  for (unsigned i = 0; i < a->n; i++)
    for (unsigned j = 0; j < b->n; j++)
      if (equal(a->p[i], b->p[(j + 1) % b->n]) &&
          equal(a->p[(i + 1) % a->n], b->p[j])) {
        *l = a->p[(i + 1) % a->n];
        *r = a->p[i];
        n++;
      }
  return n;
}
static void path_copy(NvPath *d, const NvPath *s) {
  d->n = s->n;
  for (unsigned i = 0; i < s->n; i++) {
    d->p[i].x = s->p[i].x;
    d->p[i].y = s->p[i].y;
  }
}
static CsStatus append(NvPath *p, NvPoint q, unsigned budget) {
  if (p->n && equal(p->p[p->n - 1], q))
    return CS_OK;
  if (p->n >= budget)
    return CS_FULL;
  p->p[p->n++] = q;
  return CS_OK;
}
CsStatus nv_path(const NvPolygon *c, unsigned n, NvPoint start, NvPoint end,
                 NvScratch *s, unsigned pb, unsigned ob, NvPath *out) {
  if (!c || !s || !out || n == 0 || n > NV_POLYS || !point_ok(start) ||
      !point_ok(end) || pb > NV_POINTS || ob > NV_POINTS)
    return CS_INVALID;
  for (unsigned i = 0; i < n; i++)
    if (!polygon_ok(&c[i]))
      return CS_INVALID;
  if (!inside(&c[0], start) || !inside(&c[n - 1], end))
    return CS_INVALID;
  if (n + 1 > pb)
    return CS_FULL;
  for (unsigned i = 0; i < n; i++)
    for (unsigned j = i + 1; j < n; j++) {
      NvPoint l, r;
      unsigned edges = shared(&c[i], &c[j], &l, &r);
      if (!separated(&c[i], &c[j]) && !separated(&c[j], &c[i]))
        return CS_INVALID;
      if (j == i + 1) {
        if (edges != 1)
          return CS_INVALID;
        s->left[j] = l;
        s->right[j] = r;
      } else if (edges)
        return CS_INVALID;
    }
  s->left[0] = start;
  s->right[0] = start;
  s->left[n] = end;
  s->right[n] = end;
  s->staged.n = 0;
  CsStatus r = append(&s->staged, start, ob);
  if (r != CS_OK)
    return r;
  NvPoint apex = start, left = start, right = start;
  unsigned li = 0, ri = 0;
  for (unsigned i = 1; i <= n; i++) {
    NvPoint nl = s->left[i], nr = s->right[i];
    if (CS_MUTANT == 3) {
      NvPoint t = nl;
      nl = nr;
      nr = t;
    }
    if (area(apex, right, nr) >= 0) {
      if (equal(apex, right) || area(apex, left, nr) < 0) {
        right = nr;
        ri = i;
      } else {
        r = append(&s->staged, left, ob);
        if (r != CS_OK)
          return r;
        apex = left;
        right = left;
        i = li;
        ri = li;
        continue;
      }
    }
    if (area(apex, left, nl) <= 0) {
      if (equal(apex, left) || area(apex, right, nl) > 0) {
        left = nl;
        li = i;
      } else {
        r = append(&s->staged, right, ob);
        if (r != CS_OK)
          return r;
        apex = right;
        left = right;
        i = ri;
        li = ri;
        continue;
      }
    }
  }
  r = append(&s->staged, end, ob);
  if (r != CS_OK)
    return r;
  path_copy(out, &s->staged);
  return CS_OK;
}
void nv_init(NvSession *s) {
  s->generation = 0;
  s->active = 0;
  s->path.n = 0;
}
CsStatus nv_begin(NvSession *s, const NvPolygon *c, unsigned n, NvPoint a,
                  NvPoint b, NvScratch *scratch, unsigned pb, unsigned ob,
                  NvTicket *out) {
  if (!s || !out)
    return CS_INVALID;
  if (s->active)
    return CS_BUSY;
  if (s->generation == UINT32_MAX)
    return CS_FULL;
  NvPath path;
  CsStatus r = nv_path(c, n, a, b, scratch, pb, ob, &path);
  if (r != CS_OK)
    return r;
  path_copy(&s->path, &path);
  s->generation++;
  s->active = 1;
  out->owner = s;
  out->generation = s->generation;
  return CS_OK;
}
CsStatus nv_copy(const NvSession *s, NvTicket t, NvPath *out) {
  if (!s || !out)
    return CS_INVALID;
  if (t.owner != s || !s->active ||
      (CS_MUTANT != 4 && t.generation != s->generation))
    return CS_STALE;
  path_copy(out, &s->path);
  return CS_OK;
}
CsStatus nv_cancel(NvSession *s, NvTicket t) {
  if (!s)
    return CS_INVALID;
  if (t.owner != s || !s->active || t.generation != s->generation)
    return CS_STALE;
  s->active = 0;
  return CS_OK;
}
