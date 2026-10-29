#include "index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "CI_ASSERT:%s:%d\n", #x, __LINE__);                      \
      return 41;                                                               \
    }                                                                          \
  } while (0)
static int fenwick(void) {
  for (unsigned n = 0; n <= CI_CAP; n++) {
    uint32_t v[CI_CAP];
    for (unsigned i = 0; i < n; i++)
      v[i] = (i * 7u) % 5u;
    struct ci_fenwick f;
    CHECK(ci_fenwick_init(&f, v, n) == CI_OK);
    uint32_t total = 0;
    for (unsigned i = 0; i <= n; i++) {
      uint32_t got = 999;
      CHECK(ci_fenwick_prefix(&f, i, &got) == CI_OK && got == total);
      if (i < n)
        total += v[i];
    }
    for (uint32_t k = 0; k < total; k++) {
      unsigned p = 0;
      uint32_t sum = v[0];
      while (sum <= k)
        sum += v[++p];
      unsigned got = 999;
      CHECK(ci_fenwick_select(&f, k, &got) == CI_OK && got == p);
    }
    unsigned untouched = 999;
    CHECK(ci_fenwick_select(&f, total, &untouched) == CI_RANGE &&
          untouched == 999);
    struct ci_fenwick old = f;
    CHECK(ci_fenwick_add(&f, n, 1) == CI_RANGE && !memcmp(&f, &old, sizeof f));
    if (n) {
      CHECK(ci_fenwick_add(&f, 0, -1) == CI_RANGE &&
            !memcmp(&f, &old, sizeof f));
      CHECK(ci_fenwick_add(&f, 0, 3) == CI_OK);
      v[0] = 3;
    }
    CHECK(ci_fenwick_rescale(&f, 1) == CI_OK);
    total = 0;
    for (unsigned i = 0; i <= n; i++) {
      uint32_t got = 999;
      CHECK(ci_fenwick_prefix(&f, i, &got) == CI_OK && got == total);
      if (i < n)
        total += (v[i] + 1) / 2;
    }
  }
  uint32_t v[2] = {UINT32_MAX, 0};
  struct ci_fenwick f;
  CHECK(ci_fenwick_init(&f, v, 2) == CI_OK);
  struct ci_fenwick old = f;
  CHECK(ci_fenwick_add(&f, 1, 1) == CI_RANGE && !memcmp(&f, &old, sizeof f));
  v[1] = 1;
  CHECK(ci_fenwick_init(&f, v, 2) == CI_RANGE && !memcmp(&f, &old, sizeof f));
  return 0;
}
static uint32_t flat_hash(const uint32_t *v, unsigned l, unsigned r) {
  uint32_t h = 0;
  for (unsigned i = l; i < r; i++)
    h = (uint32_t)(((uint64_t)h * 257 + v[i]) % CI_MOD);
  return h;
}
static int segment(void) {
  for (unsigned n = 0; n <= CI_CAP; n++) {
    uint32_t v[CI_CAP];
    for (unsigned i = 0; i < n; i++)
      v[i] = i + 1;
    struct ci_segment s;
    CHECK(ci_segment_init(&s, v, n) == CI_OK);
    for (unsigned pass = 0; pass < 5; pass++) {
      unsigned lo = n ? pass % n : 0, hi = n;
      uint32_t a = pass == 3 ? 0 : pass + 2, b = pass + 7;
      CHECK(ci_segment_apply(&s, lo, hi, a, b) == CI_OK);
      for (unsigned i = lo; i < hi; i++)
        v[i] = (uint32_t)(((uint64_t)a * v[i] + b) % CI_MOD);
      for (unsigned l = 0; l <= n; l++)
        for (unsigned r = l; r <= n; r++) {
          struct ci_aggregate x;
          CHECK(ci_segment_query(&s, l, r, &x) == CI_OK &&
                x.hash == flat_hash(v, l, r) && x.length == r - l);
          uint32_t power = 1, geometric = 0;
          for (unsigned k = l; k < r; k++) {
            geometric = (geometric + power) % CI_MOD;
            power = (uint32_t)(((uint64_t)power * 257) % CI_MOD);
          }
          CHECK(x.power == power && x.geometric == geometric);
        }
    }
    struct ci_segment old = s;
    CHECK(ci_segment_apply(&s, 0, n + 1, 1, 0) == CI_RANGE &&
          !memcmp(&s, &old, sizeof s));
    struct ci_aggregate x = {17, 19, 23, 29}, prev = x;
    CHECK(ci_segment_query(&s, n + 1, n + 1, &x) == CI_RANGE &&
          !memcmp(&x, &prev, sizeof x));
  }
  return 0;
}
static int invariant(const struct ci_intervals *t, int p, int *height, int32_t *maximum,
                     unsigned *seen) {
  if (p < 0) {
    *height = 0;
    *maximum = INT32_MIN;
    return 1;
  }
  if ((unsigned)p >= CI_CAP || !t->node[p].live || seen[p])
    return 0;
  seen[p] = 1;
  const struct ci_interval_node *n = &t->node[p];
  int lh, rh;
  int32_t lm, rm;
  if (!invariant(t, n->left, &lh, &lm, seen) ||
      !invariant(t, n->right, &rh, &rm, seen))
    return 0;
  *height = 1 + (lh > rh ? lh : rh);
  *maximum = n->value.high;
  if (lm > *maximum)
    *maximum = lm;
  if (rm > *maximum)
    *maximum = rm;
  return n->height == *height && n->maximum == *maximum && lh - rh <= 1 &&
         rh - lh <= 1;
}
static int interval(void) {
  for (unsigned closed = 0; closed < 2; closed++) {
    struct ci_intervals t;
    CHECK(ci_intervals_init(&t, closed) == CI_OK);
    struct ci_interval values[CI_CAP];
    unsigned live[CI_CAP] = {0};
    for (unsigned stage = 0; stage < 2; stage++)
      for (unsigned k = 0; k < CI_CAP; k++) {
        unsigned j = stage ? k : (k * 17u) % CI_CAP;
        struct ci_interval v = {(int32_t)j - 32, (int32_t)j - 31 + (int32_t)(j % 9), j};
        values[j] = v;
        if (!stage) {
          CHECK(ci_intervals_insert(&t, v) == CI_OK);
          live[j] = 1;
        } else {
          CHECK(ci_intervals_remove(&t, v.low, v.id) == CI_OK);
          live[j] = 0;
        }
        unsigned seen[CI_CAP] = {0};
        int h;
        int32_t max;
        CHECK(invariant(&t, t.root, &h, &max, seen));
        unsigned count = 0;
        for (unsigned i = 0; i < CI_CAP; i++)
          count += seen[i];
        CHECK(count == t.count);
        unsigned free_count = 0;
        int free_node = t.free_head;
        while (free_node >= 0) {
          CHECK((unsigned)free_node < CI_CAP && !seen[free_node] &&
                !t.node[free_node].live);
          seen[free_node] = 1;
          free_count++;
          free_node = t.node[free_node].next;
        }
        CHECK(count + free_count == CI_CAP);
        for (int q = -34; q < 37; q++) {
          struct ci_interval out[CI_CAP], one;
          unsigned got = 999;
          CHECK(ci_intervals_all(&t, q, q + 1, out, CI_CAP, &got) == CI_OK);
          unsigned want = 0;
          for (unsigned i = 0; i < CI_CAP; i++)
            if (live[i] &&
                (closed ? values[i].low <= q + 1 && q <= values[i].high
                        : values[i].low < q + 1 && q < values[i].high)) {
              CHECK(want < got && out[want].id == i);
              want++;
            }
          CHECK(got == want);
          enum ci_status st = ci_intervals_find(&t, q, q + 1, &one);
          CHECK(st == (want ? CI_OK : CI_MISSING));
          if (want)
            CHECK(closed ? one.low <= q + 1 && q <= one.high
                         : one.low < q + 1 && q < one.high);
        }
      }
    struct ci_interval v = {0, closed ? 0 : 1, 99};
    CHECK(ci_intervals_insert(&t, v) == CI_OK);
    struct ci_intervals old = t;
    CHECK(ci_intervals_insert(&t, v) == CI_DUPLICATE &&
          !memcmp(&t, &old, sizeof t));
    struct ci_interval out = {0, 0, 0};
    unsigned got = 88;
    CHECK(ci_intervals_all(&t, 0, 1, &out, 0, &got) == CI_FULL && got == 88);
    CHECK(ci_intervals_remove(&t, 0, 100) == CI_MISSING &&
          !memcmp(&t, &old, sizeof t));
    struct ci_interval extreme = {INT32_MIN, INT32_MAX, 123};
    CHECK(ci_intervals_insert(&t, extreme) == CI_OK);
    CHECK(ci_intervals_find(&t, INT32_MIN, INT32_MIN + 1, &out) == CI_OK &&
          out.id == 123);
    CHECK(ci_intervals_find(&t, INT32_MAX - 1, INT32_MAX, &out) == CI_OK &&
          out.id == 123);
  }
  return 0;
}
static int suffix(void) {
  for (unsigned n = 0; n <= CI_CAP; n++) {
    unsigned char b[CI_CAP];
    for (unsigned i = 0; i < n; i++)
      b[i] = (unsigned char)((i * 17 + i / 3) % 4);
    struct ci_suffix s;
    CHECK(ci_suffix_build(&s, b, n) == CI_OK);
    unsigned seen[CI_CAP + 1] = {0};
    for (unsigned i = 0; i <= n; i++) {
      unsigned p = s.order[i];
      CHECK(p <= n && !seen[p]);
      seen[p] = 1;
      if (i) {
        unsigned q = s.order[i - 1], h = 0;
        while (q + h < n && p + h < n && b[q + h] == b[p + h])
          h++;
        CHECK(s.lcp[i] == h);
        CHECK(q + h == n || (p + h < n && b[q + h] < b[p + h]));
      }
    }
    for (unsigned p = 0; p <= n; p++)
      for (unsigned m = 0; m <= n - p; m++) {
        unsigned lo = 999, hi = 999;
        CHECK(ci_suffix_range(&s, b + p, m, &lo, &hi) == CI_OK);
        unsigned count = 0;
        for (unsigned j = 0; j <= n; j++)
          if (j + m <= n && !memcmp(b + j, b + p, m))
            count++;
        CHECK(hi - lo == count);
        for (unsigned i = lo; i < hi; i++)
          CHECK(s.order[i] + m <= n && !memcmp(b + s.order[i], b + p, m));
      }
    for (unsigned i = 0; i <= n; i++)
      for (unsigned j = 0; j <= n; j++) {
        unsigned h = 0, got;
        while (i + h < n && j + h < n && b[i + h] == b[j + h])
          h++;
        CHECK(ci_suffix_lce(&s, i, j, &got) == CI_OK && got == h);
      }
    for (unsigned disjoint = 0; disjoint < 2; disjoint++) {
      unsigned want = 0, wa = 0, wb = 0, a, z, got;
      for (unsigned i = 0; i < n; i++)
        for (unsigned j = i + 1; j < n; j++) {
          unsigned h = 0;
          while (i + h < n && j + h < n && b[i + h] == b[j + h] &&
                 (!disjoint || i + h < j))
            h++;
          if (h > want) {
            want = h;
            wa = i;
            wb = j;
          }
        }
      CHECK(ci_suffix_longest(&s, disjoint, &a, &z, &got) == CI_OK &&
            got == want && a == wa && z == wb);
    }
    struct ci_suffix old = s;
    CHECK(ci_suffix_build(&s, b, CI_CAP + 1) == CI_RANGE &&
          !memcmp(&old, &s, sizeof s));
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  int r = 0;
  switch (atoi(argv[1])) {
  case 1:
    r = fenwick();
    break;
  case 2:
    r = segment();
    break;
  case 3:
    r = interval();
    break;
  case 4:
    r = suffix();
    break;
  default:
    return 2;
  }
  if (!r)
    puts("CI_PASS");
  return r;
}
