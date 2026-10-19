#include "index.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
  char kind;
  while (scanf(" %c", &kind) == 1) {
    unsigned n, q;
    if (kind == 'F') {
      uint32_t v[CI_CAP];
      if (scanf("%u", &n) != 1 || n > CI_CAP)
        return 2;
      for (unsigned i = 0; i < n; i++)
        if (scanf("%" SCNu32, &v[i]) != 1)
          return 2;
      CiFenwick f;
      CiStatus initial = ci_fenwick_init(&f, v, n);
      printf("%d\n", initial);
      if (initial != CI_OK)
        return 3;
      if (scanf("%u", &q) != 1)
        return 2;
      for (unsigned i = 0; i < q; i++) {
        char c;
        unsigned p;
        int64_t d;
        uint32_t x = 999;
        unsigned z = 999;
        if (scanf(" %c%u", &c, &p) != 2)
          return 2;
        CiStatus st;
        if (c == 'a') {
          if (scanf("%" SCNd64, &d) != 1)
            return 2;
          st = ci_fenwick_add(&f, p, d);
        } else if (c == 'p')
          st = ci_fenwick_prefix(&f, p, &x);
        else if (c == 's')
          st = ci_fenwick_select(&f, p, &z);
        else if (c == 'r')
          st = ci_fenwick_rescale(&f, p);
        else
          return 2;
        printf("%d %" PRIu32 " %u\n", st, x, z);
      }
    } else if (kind == 'S') {
      uint32_t v[CI_CAP];
      if (scanf("%u", &n) != 1 || n > CI_CAP)
        return 2;
      for (unsigned i = 0; i < n; i++)
        if (scanf("%" SCNu32, &v[i]) != 1)
          return 2;
      CiSegment s;
      CiStatus initial = ci_segment_init(&s, v, n);
      printf("%d\n", initial);
      if (initial != CI_OK)
        return 3;
      if (scanf("%u", &q) != 1)
        return 2;
      for (unsigned i = 0; i < q; i++) {
        char c;
        unsigned l, r;
        uint32_t a, b;
        CiAggregate out = {999, 999, 999, 999};
        if (scanf(" %c%u%u", &c, &l, &r) != 3)
          return 2;
        CiStatus st;
        if (c == 'a') {
          if (scanf("%" SCNu32 "%" SCNu32, &a, &b) != 2)
            return 2;
          st = ci_segment_apply(&s, l, r, a, b);
        } else
          st = ci_segment_query(&s, l, r, &out);
        printf("%d %" PRIu32 " %u\n", st, out.hash, out.length);
      }
    } else if (kind == 'I') {
      unsigned closed;
      if (scanf("%u%u", &closed, &q) != 2)
        return 2;
      CiIntervals t;
      CiStatus initial = ci_intervals_init(&t, closed);
      printf("%d\n", initial);
      if (initial != CI_OK)
        return 3;
      for (unsigned i = 0; i < q; i++) {
        char c;
        int32_t l, h;
        uint32_t id;
        CiStatus st;
        if (scanf(" %c%" SCNd32, &c, &l) != 2)
          return 2;
        if (c == 'a') {
          if (scanf("%" SCNd32 "%" SCNu32, &h, &id) != 2)
            return 2;
          CiInterval v = {l, h, id};
          st = ci_intervals_insert(&t, v);
          printf("%d\n", st);
        } else if (c == 'd') {
          if (scanf("%" SCNu32, &id) != 1)
            return 2;
          printf("%d\n", ci_intervals_remove(&t, l, id));
        } else {
          if (scanf("%" SCNd32, &h) != 1)
            return 2;
          CiInterval out[CI_CAP];
          unsigned got = 0;
          st = ci_intervals_all(&t, l, h, out, CI_CAP, &got);
          printf("%d %u", st, got);
          for (unsigned k = 0; k < got; k++)
            printf(" %" PRId32 " %" PRId32 " %" PRIu32, out[k].low, out[k].high,
                   out[k].id);
          puts("");
        }
      }
    } else if (kind == 'X') {
      unsigned char v[CI_CAP];
      if (scanf("%u", &n) != 1 || n > CI_CAP)
        return 2;
      for (unsigned i = 0; i < n; i++) {
        unsigned x;
        if (scanf("%u", &x) != 1 || x > 255)
          return 2;
        v[i] = (unsigned char)x;
      }
      CiSuffix s;
      CiStatus initial = ci_suffix_build(&s, v, n);
      printf("%d", initial);
      if (initial != CI_OK)
        return 3;
      for (unsigned i = 0; i <= n; i++)
        printf(" %u", s.order[i]);
      for (unsigned i = 0; i <= n; i++)
        printf(" %u", s.lcp[i]);
      puts("");
      if (scanf("%u", &q) != 1)
        return 2;
      for (unsigned i = 0; i < q; i++) {
        unsigned m;
        unsigned char p[CI_CAP];
        if (scanf("%u", &m) != 1 || m > CI_CAP)
          return 2;
        for (unsigned j = 0; j < m; j++) {
          unsigned x;
          if (scanf("%u", &x) != 1 || x > 255)
            return 2;
          p[j] = (unsigned char)x;
        }
        unsigned l = 999, h = 999;
        CiStatus st = ci_suffix_range(&s, p, m, &l, &h);
        printf("%d %u %u\n", st, l, h);
      }
    } else
      return 2;
  }
  return ferror(stdin) ? 2 : 0;
}
