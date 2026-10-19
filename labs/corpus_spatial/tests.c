#include "spatial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SP_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x);          \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
#define NIL UINT32_MAX
static SpIndex index_a, scratch, saved;
static unsigned rng = 913;
static unsigned next(void) {
  rng = rng * 1664525u + 1013904223u;
  return rng;
}
static uint32_t bit(unsigned i) { return UINT32_C(1) << i; }
static SpItem point(unsigned id, int x, int y, int z) {
  SpItem a = {id, {{x, y, z}, {x, y, z}}};
  return a;
}
static int hit(SpBox a, SpBox b, unsigned dim) {
  for (unsigned k = 0; k < dim; k++)
    if (a.lo[k] > b.hi[k] || a.hi[k] < b.lo[k])
      return 0;
  return 1;
}
static int inside(SpBox outer, SpBox inner, unsigned dim) {
  for (unsigned k = 0; k < dim; k++)
    if (outer.lo[k] > inner.lo[k] || outer.hi[k] < inner.hi[k])
      return 0;
  return 1;
}
static unsigned nodes_seen, leafdepth;
static uint32_t treecheck(const SpIndex *t, unsigned at, unsigned depth,
                          int root) {
  CHECK(at < 128);
  const SpNode *n = &t->nodes[at];
  CHECK(n->active);
  CHECK(depth < 32);
  nodes_seen++;
  uint32_t mask = 0;
  if (t->kind == SP_KD || (t->kind == SP_BVH && n->leaf)) {
    CHECK(n->item < 32);
    mask = bit(n->item);
    CHECK(inside(n->box, t->items[n->item].box, t->dim));
  }
  if (t->kind == SP_KD || t->kind == SP_BVH) {
    for (unsigned i = 0; i < 2; i++)
      if (n->child[i] != NIL) {
        uint32_t m = treecheck(t, n->child[i], depth + 1, 0);
        CHECK(!(mask & m));
        mask |= m;
        CHECK(inside(n->box, t->nodes[n->child[i]].box, t->dim));
      }
  } else if (t->kind == SP_QUAD) {
    if (n->leaf)
      mask = n->mask;
    else
      for (unsigned i = 0; i < (1u << t->dim); i++)
        if (n->child[i] != NIL) {
          uint32_t m = treecheck(t, n->child[i], depth + 1, 0);
          CHECK(!(mask & m));
          mask |= m;
        }
    for (unsigned i = 0; i < 32; i++)
      if (mask & bit(i))
        CHECK(inside(n->box, t->items[i].box, t->dim));
    CHECK(mask == n->mask);
  } else {
    CHECK(n->count <= 4);
    CHECK(root ? n->count >= 1 : n->count >= 2);
    if (n->leaf) {
      if (leafdepth == NIL)
        leafdepth = depth;
      CHECK(depth == leafdepth);
      for (unsigned i = 0; i < n->count; i++) {
        CHECK(n->entry[i] < 32);
        uint32_t m = bit(n->entry[i]);
        CHECK(!(mask & m));
        mask |= m;
        CHECK(inside(n->box, t->items[n->entry[i]].box, t->dim));
      }
    } else {
      CHECK(!root || n->count >= 2);
      for (unsigned i = 0; i < n->count; i++) {
        uint32_t m = treecheck(t, n->entry[i], depth + 1, 0);
        CHECK(!(mask & m));
        mask |= m;
        CHECK(inside(n->box, t->nodes[n->entry[i]].box, t->dim));
      }
    }
  }
  return mask;
}
static void invariant(const SpIndex *t) {
  if (t->kind == SP_GRID) {
    unsigned refs = 0;
    uint32_t mask = 0;
    unsigned seen[256] = {0};
    for (unsigned h = 0; h < 17; h++)
      for (unsigned j = t->buckets[h]; j != NIL; j = t->refs[j].next) {
        CHECK(j < t->used && seen[j]++ == 0);
        refs++;
        const SpRef *r = &t->refs[j];
        CHECK(r->item < 32);
        mask |= bit(r->item);
        for (unsigned k = 0; k < 3; k++) {
          int x = t->items[r->item].box.lo[k], y = t->items[r->item].box.hi[k],
              w = (int)t->width; /* Independent division via exact containing
                                    inequalities. */
          CHECK(r->cell[k] * w <= y && (r->cell[k] + 1) * w > x);
        }
      }
    CHECK(refs == t->used && mask == t->live);
    return;
  }
  if (!t->count) {
    CHECK(t->root == NIL && t->used == 0);
    return;
  }
  nodes_seen = 0;
  leafdepth = NIL;
  CHECK(treecheck(t, t->root, 0, 1) == t->live);
  CHECK(nodes_seen == t->used);
}
static void queries(unsigned print) {
  const SpIndex *t = &index_a;
  invariant(t);
  for (unsigned k = 0; k < 120; k++) {
    SpBox q = {{0, 0, 0}, {0, 0, 0}};
    int32_t p[3] = {0, 0, 0};
    unsigned w[3] = {1, 1, 1};
    for (unsigned d = 0; d < t->dim; d++) {
      q.lo[d] = (int)(next() % 35) - 17;
      q.hi[d] = q.lo[d] + (int)(next() % 10);
      p[d] = (int)(next() % 35) - 17;
      w[d] = 1 + next() % 4;
    }
    uint32_t expected = 0;
    for (unsigned i = 0; i < 32; i++)
      if ((t->live & bit(i)) && hit(t->items[i].box, q, t->dim))
        expected |= bit(i);
    SpResult r;
    CHECK(sp_query(t, q, &r) == SP_OK);
    CHECK(r.mask == expected);
    CHECK(r.tests <= 32);
    CHECK(r.visits <= (t->kind == SP_GRID ? 256u : 128u));
    if (print)
      printf("Q %d %d %d %d %d %d %u %u %u\n", q.lo[0], q.lo[1], q.lo[2],
             q.hi[0], q.hi[1], q.hi[2], r.mask, r.visits, r.tests);
    if (t->kind == SP_KD || t->kind == SP_BVH) {
      SpNearest n;
      CHECK(sp_nearest(t, p, w, &n) == SP_OK);
      uint64_t best = UINT64_MAX;
      unsigned id = NIL;
      for (unsigned i = 0; i < 32; i++)
        if (t->live & bit(i)) {
          uint64_t sum = 0;
          for (unsigned d = 0; d < t->dim; d++) {
            int a = t->items[i].box.lo[d], b = t->items[i].box.hi[d],
                v = p[d] < a   ? a - p[d]
                    : p[d] > b ? p[d] - b
                               : 0;
            sum += (uint64_t)v * (unsigned)v * w[d];
          }
          if (sum < best) {
            best = sum;
            id = i;
          }
        }
      CHECK(n.id == id && n.distance == best);
      if (print)
        printf("N %d %d %d %u %u %u %u %llu\n", p[0], p[1], p[2], w[0], w[1],
               w[2], n.id, (unsigned long long)n.distance);
    }
  }
  if (t->kind == SP_GRID) {
    uint32_t rows[32];
    unsigned candidates;
    CHECK(sp_pairs(t, rows, &candidates) == SP_OK);
    CHECK(candidates <= 496);
    unsigned exact_candidates = 0;
    for (unsigned i = 0; i < 32; i++) {
      uint32_t expected = 0;
      for (unsigned j = i + 1; j < 32; j++)
        if ((t->live & bit(i)) && (t->live & bit(j))) {
          if (hit(t->items[i].box, t->items[j].box, t->dim))
            expected |= bit(j);
          unsigned share = 0;
          for (unsigned a = 0; a < t->used; a++)
            if (t->refs[a].item == i)
              for (unsigned b = 0; b < t->used; b++)
                if (t->refs[b].item == j &&
                    t->refs[a].cell[0] == t->refs[b].cell[0] &&
                    t->refs[a].cell[1] == t->refs[b].cell[1] &&
                    t->refs[a].cell[2] == t->refs[b].cell[2])
                  share = 1;
          exact_candidates += share;
        }
      CHECK(rows[i] == expected);
      if (print)
        printf("P %u %u\n", i, rows[i]);
    }
    CHECK(candidates == exact_candidates);
  }
}
static void dumpitems(void) {
  printf("S %u %u %u\n", index_a.kind, index_a.dim, index_a.count);
  for (unsigned i = 0; i < 32; i++)
    if (index_a.live & bit(i)) {
      SpItem x = index_a.items[i];
      printf("I %u %d %d %d %d %d %d\n", i, x.box.lo[0], x.box.lo[1],
             x.box.lo[2], x.box.hi[0], x.box.hi[1], x.box.hi[2]);
    }
}
static void exercise(unsigned kind, unsigned dim, unsigned print) {
  SpItem list[32];
  for (unsigned i = 0; i < 32; i++) {
    list[i] = point(i, (int)(next() % 24) - 12, (int)(next() % 24) - 12,
                    dim == 3 ? (int)(next() % 24) - 12 : 0);
    if (kind == SP_BVH || kind == SP_RTREE || kind == SP_GRID)
      for (unsigned d = 0; d < dim; d++)
        list[i].box.hi[d] += 1 + (int)(next() % 2);
  }
  CHECK(sp_build(&index_a, &scratch, kind, dim, list, 32,
                 kind == SP_GRID ? 256 : 128, 4) == SP_OK);
  if (print)
    dumpitems();
  queries(print);
  for (unsigned i = 0; i < 32; i += 3) {
    CHECK(sp_edit(&index_a, &scratch, 3, list[i]) == SP_OK);
    invariant(&index_a);
  }
  if (print)
    dumpitems();
  queries(print);
  for (unsigned i = 0; i < 32; i += 3) {
    CHECK(sp_edit(&index_a, &scratch, 1, list[i]) == SP_OK);
    invariant(&index_a);
  }
  if (print)
    dumpitems();
  queries(print);
  for (unsigned i = 0; i < 32; i++) {
    list[i].box.lo[0] += 1;
    list[i].box.hi[0] += 1;
    CHECK(sp_edit(&index_a, &scratch, 2, list[i]) == SP_OK);
    invariant(&index_a);
  }
  if (print)
    dumpitems();
  queries(print);
  saved = index_a;
  SpItem invalid = list[0];
  invalid.box.lo[0] = 1025;
  CHECK(sp_edit(&index_a, &scratch, 2, invalid) == SP_INVALID);
  CHECK(memcmp(&saved, &index_a, sizeof saved) == 0);
  for (unsigned i = 0; i < 32; i++) {
    CHECK(sp_edit(&index_a, &scratch, 3, list[i]) == SP_OK);
    invariant(&index_a);
  }
  SpResult r = {99, 99, 99};
  CHECK(sp_query(&index_a, list[0].box, &r) == SP_OK && r.mask == 0);
  CHECK(sp_build(&index_a, &scratch, kind, dim, NULL, 0, 0, 4) == SP_OK);
  saved = index_a;
  CHECK(sp_build(&index_a, &scratch, kind, dim, list, 1, 0, 4) == SP_FULL);
  CHECK(memcmp(&saved, &index_a, sizeof saved) == 0);
  CHECK(sp_build(&index_a, &scratch, kind, dim, list, 32,
                 kind == SP_GRID ? 256 : 128, 4) == SP_OK);
  unsigned used = index_a.used;
  saved = index_a;
  CHECK(used > 1);
  CHECK(sp_build(&index_a, &scratch, kind, dim, list, 32, used - 1, 4) ==
        SP_FULL);
  CHECK(memcmp(&saved, &index_a, sizeof saved) == 0);
}
static void boundaries(unsigned kind) {
  SpItem a[5] = {point(0, -1, 0, 0), point(1, 1, 0, 0), point(2, 0, 1, 0),
                 point(3, 0, 0, 0), point(4, 0, 0, 0)};
  CHECK(sp_build(&index_a, &scratch, kind, 2, a, 5, 128, 2) == SP_OK);
  invariant(&index_a);
  saved = index_a;
  CHECK(sp_edit(&index_a, &scratch, 1, a[0]) == SP_INVALID);
  SpItem missing = point(31, 0, 0, 0);
  CHECK(sp_edit(&index_a, &scratch, 2, missing) == SP_MISSING);
  CHECK(sp_edit(&index_a, &scratch, 3, missing) == SP_MISSING);
  CHECK(memcmp(&index_a, &saved, sizeof saved) == 0);
  SpResult untouched = {77, 88, 99};
  SpBox invalid = {{1, 0, 0}, {0, 0, 0}};
  CHECK(sp_query(&index_a, invalid, &untouched) == SP_INVALID);
  CHECK(untouched.mask == 77 && untouched.visits == 88 &&
        untouched.tests == 99);
  CHECK(sp_query(&index_a, a[0].box, NULL) == SP_INVALID);
  CHECK(sp_build(&index_a, &scratch, kind, 4, a, 5, 128, 2) == SP_INVALID);
  CHECK(memcmp(&index_a, &saved, sizeof saved) == 0);
  if (kind == SP_KD) {
    int32_t p[3] = {0, 0, 0};
    unsigned w[3] = {1, 1, 1};
    SpNearest n;
    CHECK(sp_nearest(&index_a, p, w, &n) == SP_OK && n.id == 3);
    a[0] = point(0, 2, 0, 0);
    a[1] = point(1, 0, 3, 0);
    CHECK(sp_build(&index_a, &scratch, kind, 2, a, 2, 128, 2) == SP_OK);
    w[0] = 4;
    CHECK(sp_nearest(&index_a, p, w, &n) == SP_OK && n.id == 1 &&
          n.distance == 9);
    SpItem line[32];
    for (unsigned i = 0; i < 32; i++)
      line[i] = point(i, (int)i - 16, 0, 0);
    CHECK(sp_build(&index_a, &scratch, SP_KD, 3, line, 32, 128, 0) == SP_OK);
    queries(0);
  }
  if (kind == SP_BVH) {
    SpBox q = {{-1, 0, 0}, {-1, 0, 0}};
    SpResult r;
    CHECK(sp_query(&index_a, q, &r) == SP_OK && r.mask == 1);
    a[0] = point(0, 1024, 1024, 0);
    CHECK(sp_refit(&index_a, &scratch, a[0]) == SP_OK);
    CHECK(sp_query(&index_a, a[0].box, &r) == SP_OK && r.mask == 1);
    invariant(&index_a);
  }
  if (kind == SP_QUAD) {
    SpResult r;
    CHECK(sp_query(&index_a, a[3].box, &r) == SP_OK &&
          r.mask == (bit(3) | bit(4)));
    a[0] = point(0, -16, -16, 0);
    a[1] = point(1, 15, 15, 0);
    CHECK(sp_build(&index_a, &scratch, kind, 2, a, 2, 128, 2) == SP_OK);
    CHECK(sp_query(&index_a, a[0].box, &r) == SP_OK && r.mask == 1);
    saved = index_a;
    a[1] = point(1, 16, 0, 0);
    CHECK(sp_build(&index_a, &scratch, kind, 2, a, 2, 128, 2) == SP_INVALID);
    CHECK(memcmp(&saved, &index_a, sizeof saved) == 0);
  }
  if (kind == SP_GRID) {
    invariant(&index_a);
    a[0].box = (SpBox){{-2, -2, 0}, {2, 2, 0}};
    a[1].box = a[0].box;
    CHECK(sp_build(&index_a, &scratch, kind, 2, a, 2, 128, 2) == SP_OK);
    uint32_t rows[32];
    unsigned candidates;
    CHECK(sp_pairs(&index_a, rows, &candidates) == SP_OK && rows[0] == 2 &&
          candidates == 1);
    a[0].box.lo[2] = a[1].box.lo[2] = -1;
    a[0].box.hi[2] = a[1].box.hi[2] = 1;
    for (unsigned width = 1; width <= 4; width *= 2) {
      CHECK(sp_build(&index_a, &scratch, kind, 3, a, 2, 256, width) == SP_OK);
      invariant(&index_a);
      CHECK(sp_pairs(&index_a, rows, &candidates) == SP_OK && rows[0] == 2 &&
            candidates == 1);
    }
    a[0].box = (SpBox){{0, 0, 0}, {31, 31, 31}};
    CHECK(sp_build(&index_a, &scratch, kind, 3, a, 1, 256, 16) == SP_OK);
    CHECK(index_a.used == 8);
    saved = index_a;
    CHECK(sp_build(&index_a, &scratch, kind, 3, a, 1, 256, 1) == SP_FULL);
    CHECK(memcmp(&saved, &index_a, sizeof saved) == 0);
    SpResult r;
    CHECK(sp_query(&index_a, a[0].box, &r) == SP_OK && r.mask == 1);
  }
}
static void refit_comparison(void) {
  SpItem items[32];
  for (unsigned i = 0; i < 32; i++)
    items[i] = point(i, (int)i * 8, 0, 0);
  CHECK(sp_build(&index_a, &scratch, SP_BVH, 3, items, 32, 128, 0) == SP_OK);
  unsigned topology[128][2];
  for (unsigned i = 0; i < index_a.used; i++)
    for (unsigned j = 0; j < 2; j++)
      topology[i][j] = index_a.nodes[i].child[j];
  for (unsigned i = 0; i < 32; i++) {
    items[i] = point(i, (int)((i * 13) % 32), 0, 0);
    items[i].box.hi[0]++;
    items[i].box.hi[2] = 1; /* Degenerate3D slab: y extent zero. */
    CHECK(sp_refit(&index_a, &scratch, items[i]) == SP_OK);
    for (unsigned n = 0; n < index_a.used; n++)
      for (unsigned j = 0; j < 2; j++)
        CHECK(topology[n][j] == index_a.nodes[n].child[j]);
  }
  invariant(&index_a);
  uint32_t expected[32];
  unsigned old_visits = 0, new_visits = 0;
  for (unsigned i = 0; i < 32; i++) {
    SpResult r;
    CHECK(sp_query(&index_a, items[i].box, &r) == SP_OK);
    expected[i] = 0;
    for (unsigned j = 0; j < 32; j++)
      if (hit(items[i].box, items[j].box, 3))
        expected[i] |= bit(j);
    CHECK(r.mask == expected[i]);
    old_visits += r.visits;
  }
  CHECK(sp_build(&index_a, &scratch, SP_BVH, 3, items, 32, 128, 0) == SP_OK);
  for (unsigned i = 0; i < 32; i++) {
    SpResult r;
    CHECK(sp_query(&index_a, items[i].box, &r) == SP_OK &&
          r.mask == expected[i]);
    new_visits += r.visits;
  }
  printf("M refit_visits %u rebuild_visits %u identical_queries 32\n",
         old_visits, new_visits);
}
int main(int argc, char **argv) {
  unsigned kind = argc > 1 ? (unsigned)strtoul(argv[1], NULL, 10) : 1;
  CHECK(kind >= 1 && kind <= 5);
  unsigned dump = argc > 2;
  boundaries(kind);
  if (kind == SP_BVH)
    refit_comparison();
  for (unsigned dim = 2; dim <= 3; dim++)
    for (unsigned seed = 0; seed < (dump ? 1u : 5u); seed++) {
      rng = 913 + seed;
      exercise(kind, dim, dump);
    }
  printf("SP_PASS:%u\n", kind);
  return 0;
}
