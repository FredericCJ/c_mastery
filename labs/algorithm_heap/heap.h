#ifndef CMASTERY_ALGORITHM_HEAP_H
#define CMASTERY_ALGORITHM_HEAP_H
#include "algo.h"
#include "containers.h"
#include <stddef.h>
_Static_assert(sizeof(int) == 4 && sizeof(unsigned) == 4 &&
                   sizeof(struct cd_heap_item) == 16,
               "selected heap assembly layout");
_Static_assert(offsetof(struct cd_heap_item, priority) == 0 &&
                   offsetof(struct cd_heap_item, identity) == 4 &&
                   offsetof(struct cd_heap_item, slot) == 8 &&
                   offsetof(struct cd_heap_item, sequence) == 12,
               "record offsets");
_Static_assert(offsetof(struct cd_heap, index) == 256 &&
                   offsetof(struct cd_heap, count) == 448 &&
                   offsetof(struct cd_heap, max_heap) == 452 &&
                   offsetof(struct cd_heap, comparisons) == 464,
               "heap offsets");

struct ah_stats
{
    uint32_t comparisons, swaps;
};

struct ah_trace
{
    uint32_t used, position[12], state[12][1024];
};
extern struct ah_stats ah_stats;
extern struct ah_trace ah_observed;
void ah_snapshot(const uint32_t *, uint32_t, uint32_t);
void ah_instrumented_sift(uint32_t *, uint32_t, uint32_t);
int ah_trace(uint32_t *, uint32_t, uint32_t, uint32_t, unsigned, unsigned,
             struct ah_stats *, struct ah_trace *);
void ah_cd_sink(struct cd_heap *, unsigned);
void ah_heap_init(struct cd_heap *, unsigned);
enum cd_status ah_heap_insert(struct cd_heap *, int, unsigned,
                              struct cd_heap_handle *);
enum cd_status ah_heap_take(struct cd_heap *, int *, unsigned *);
enum cd_status ah_heap_get(const struct cd_heap *, struct cd_heap_handle, int *,
                           unsigned *);
enum cd_status ah_heap_change(struct cd_heap *, struct cd_heap_handle, int);
enum cd_status ah_heap_build(struct cd_heap *, const int *, size_t, unsigned);
uint32_t ah_cd_probe(struct cd_heap *, unsigned, uint32_t, uint32_t *);
uint32_t algo_abi_sift_probe(uint32_t *, uint32_t, uint32_t, uint32_t *);
int ah_test(unsigned);
int ah_failure(const char *, unsigned);
void ah_emit(unsigned, const uint32_t *, unsigned, const struct ah_stats *,
             const struct ah_trace *);
#endif
