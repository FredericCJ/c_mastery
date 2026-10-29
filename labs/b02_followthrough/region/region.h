#ifndef BF_REGION_H
#define BF_REGION_H
#include "bridge.h"
#include "arena.h"
struct bf_cohort;

struct bf_object
{
    _Alignas(32) unsigned value;
    unsigned id, live, opens, closes;
    struct bf_cohort *owner;
};

struct bf_cohort
{
    struct mb_region bytes;
    struct bf_object objects[4];
    unsigned trace[16], used;
};

void bf_cohort_init(struct bf_cohort *);
int bf_cohort_batch(struct bf_cohort *, const unsigned char *, size_t, unsigned,
                    unsigned, unsigned *);
int bf_cohort_reset(struct bf_cohort *);

struct bf_region_tree
{
    struct mb_region parent, child[2];
};

void bf_tree_init(struct bf_region_tree *);
int bf_tree_child_reset(struct bf_region_tree *, unsigned);
int bf_tree_reset(struct bf_region_tree *);

struct bf_slice
{
    unsigned char *base;
    size_t begin, capacity, cursor, payload, high;
    unsigned generation;
};

struct bf_slice_span
{
    const struct bf_slice *owner;
    unsigned generation;
    size_t offset, length;
};

int bf_slice_init(struct bf_slice *, unsigned char *, size_t, size_t, size_t);
int bf_slice_alloc(struct bf_slice *, size_t, size_t, struct bf_slice_span *);
int bf_slice_read(const struct bf_slice *, struct bf_slice_span, size_t,
                  unsigned char *);
int bf_slice_reset(struct bf_slice *);

struct bf_chunk
{
    _Alignas(8) unsigned char bytes[32];
    size_t cursor;
};
typedef struct bf_chunk *(*bf_acquire_chunk_fn)(void *);
typedef void (*bf_release_chunk_fn)(void *, struct bf_chunk *);

struct bf_growth
{
    struct bf_chunk *chunks[4];
    size_t count, payload, occupied, high, capacity_high;
    unsigned generation, busy;
    bf_acquire_chunk_fn acquire;
    bf_release_chunk_fn release;
    void *context;
};

struct bf_growth_span
{
    const struct bf_growth *owner;
    unsigned generation;
    size_t chunk, offset, length;
};

int bf_growth_init(struct bf_growth *, struct bf_chunk *, bf_acquire_chunk_fn,
                   bf_release_chunk_fn, void *);
int bf_growth_alloc(struct bf_growth *, size_t, size_t,
                    struct bf_growth_span *);
int bf_growth_read(const struct bf_growth *, struct bf_growth_span, size_t,
                   unsigned char *);
int bf_growth_reset(struct bf_growth *);

struct bf_retry_region
{
    struct bf_arena arena;
    struct b2_mark mark;
    unsigned active;
};

void bf_retry_init(struct bf_retry_region *);
int bf_retry_reserve(struct bf_retry_region *, size_t, size_t,
                     struct bf_borrow *);
int bf_retry_register(struct bf_retry_region *, bf_close_fn, void *);
int bf_retry_reset(struct bf_retry_region *);
#endif
