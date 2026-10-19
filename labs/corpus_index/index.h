#ifndef CMASTERY_CORPUS_INDEX_H
#define CMASTERY_CORPUS_INDEX_H
#include <stddef.h>
#include <stdint.h>
#define CI_CAP 64u
#define CI_MOD 65521u
typedef enum {
  CI_OK,
  CI_INVALID,
  CI_RANGE,
  CI_FULL,
  CI_DUPLICATE,
  CI_MISSING
} CiStatus;
typedef struct {
  unsigned n;
  uint32_t bit[CI_CAP + 1], total;
} CiFenwick;
CiStatus ci_fenwick_init(CiFenwick *, const uint32_t *, unsigned);
CiStatus ci_fenwick_add(CiFenwick *, unsigned, int64_t);
CiStatus ci_fenwick_prefix(const CiFenwick *, unsigned, uint32_t *);
CiStatus ci_fenwick_select(const CiFenwick *, uint32_t, unsigned *);
CiStatus ci_fenwick_rescale(CiFenwick *, unsigned);
typedef struct {
  uint32_t hash, power, geometric;
  unsigned length;
} CiAggregate;
typedef struct {
  CiAggregate value;
  uint32_t a, b;
} CiSegmentNode;
typedef struct {
  unsigned n;
  CiSegmentNode node[4 * CI_CAP];
} CiSegment;
CiStatus ci_segment_init(CiSegment *, const uint32_t *, unsigned);
CiStatus ci_segment_apply(CiSegment *, unsigned, unsigned, uint32_t, uint32_t);
CiStatus ci_segment_query(const CiSegment *, unsigned, unsigned, CiAggregate *);
typedef struct {
  int32_t low, high;
  uint32_t id;
} CiInterval;
typedef struct {
  CiInterval value;
  int left, right, height, next;
  int32_t maximum;
  unsigned live;
} CiIntervalNode;
typedef struct {
  CiIntervalNode node[CI_CAP];
  int root, free_head;
  unsigned count, closed;
} CiIntervals;
CiStatus ci_intervals_init(CiIntervals *, unsigned);
CiStatus ci_intervals_insert(CiIntervals *, CiInterval);
CiStatus ci_intervals_remove(CiIntervals *, int32_t, uint32_t);
CiStatus ci_intervals_find(const CiIntervals *, int32_t, int32_t, CiInterval *);
CiStatus ci_intervals_all(const CiIntervals *, int32_t, int32_t, CiInterval *,
                          unsigned, unsigned *);
typedef struct {
  unsigned n, order[CI_CAP + 1], lcp[CI_CAP + 1];
  unsigned char bytes[CI_CAP];
} CiSuffix;
CiStatus ci_suffix_build(CiSuffix *, const unsigned char *, unsigned);
CiStatus ci_suffix_range(const CiSuffix *, const unsigned char *, unsigned,
                         unsigned *, unsigned *);
CiStatus ci_suffix_lce(const CiSuffix *, unsigned, unsigned, unsigned *);
CiStatus ci_suffix_longest(const CiSuffix *, unsigned, unsigned *, unsigned *,
                           unsigned *);
#endif
