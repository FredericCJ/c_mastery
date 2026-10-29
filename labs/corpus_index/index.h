#ifndef CMASTERY_CORPUS_INDEX_H
#define CMASTERY_CORPUS_INDEX_H
#include <stddef.h>
#include <stdint.h>
#define CI_CAP 64u
#define CI_MOD 65521u

enum ci_status
{
    CI_OK,
    CI_INVALID,
    CI_RANGE,
    CI_FULL,
    CI_DUPLICATE,
    CI_MISSING
};

struct ci_fenwick
{
    unsigned n;
    uint32_t bit[CI_CAP + 1], total;
};
enum ci_status ci_fenwick_init(struct ci_fenwick *, const uint32_t *, unsigned);
enum ci_status ci_fenwick_add(struct ci_fenwick *, unsigned, int64_t);
enum ci_status ci_fenwick_prefix(const struct ci_fenwick *, unsigned,
                                 uint32_t *);
enum ci_status ci_fenwick_select(const struct ci_fenwick *, uint32_t,
                                 unsigned *);
enum ci_status ci_fenwick_rescale(struct ci_fenwick *, unsigned);

struct ci_aggregate
{
    uint32_t hash, power, geometric;
    unsigned length;
};

struct ci_segment_node
{
    struct ci_aggregate value;
    uint32_t a, b;
};

struct ci_segment
{
    unsigned n;
    struct ci_segment_node node[4 * CI_CAP];
};
enum ci_status ci_segment_init(struct ci_segment *, const uint32_t *, unsigned);
enum ci_status ci_segment_apply(struct ci_segment *, unsigned, unsigned,
                                uint32_t, uint32_t);
enum ci_status ci_segment_query(const struct ci_segment *, unsigned, unsigned,
                                struct ci_aggregate *);

struct ci_interval
{
    int32_t low, high;
    uint32_t id;
};

struct ci_interval_node
{
    struct ci_interval value;
    int left, right, height, next;
    int32_t maximum;
    unsigned live;
};

struct ci_intervals
{
    struct ci_interval_node node[CI_CAP];
    int root, free_head;
    unsigned count, closed;
};
enum ci_status ci_intervals_init(struct ci_intervals *, unsigned);
enum ci_status ci_intervals_insert(struct ci_intervals *, struct ci_interval);
enum ci_status ci_intervals_remove(struct ci_intervals *, int32_t, uint32_t);
enum ci_status ci_intervals_find(const struct ci_intervals *, int32_t, int32_t,
                                 struct ci_interval *);
enum ci_status ci_intervals_all(const struct ci_intervals *, int32_t, int32_t,
                                struct ci_interval *, unsigned, unsigned *);

struct ci_suffix
{
    unsigned n, order[CI_CAP + 1], lcp[CI_CAP + 1];
    unsigned char bytes[CI_CAP];
};
enum ci_status ci_suffix_build(struct ci_suffix *, const unsigned char *,
                               unsigned);
enum ci_status ci_suffix_range(const struct ci_suffix *, const unsigned char *,
                               unsigned, unsigned *, unsigned *);
enum ci_status ci_suffix_lce(const struct ci_suffix *, unsigned, unsigned,
                             unsigned *);
enum ci_status ci_suffix_longest(const struct ci_suffix *, unsigned, unsigned *,
                                 unsigned *, unsigned *);
#endif
