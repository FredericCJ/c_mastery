#ifndef ALLOCATION_DEPTH_H
#define ALLOCATION_DEPTH_H
#include "algo.h"
#include <stddef.h>
#include <stdint.h>

enum
{
    AL_OK,
    AL_BAD,
    AL_FULL,
    AL_STALE,
    AL_CORRUPT,
    AL_CLEANUP,
    AL_RETIRED
};

typedef int (*al_cleanup_fn)(void *);

struct al_handle
{
    const void *owner;
    uint32_t id;
};

struct al_item
{
    size_t offset, size, before;
    uint32_t id;
    al_cleanup_fn cleanup;
    void *context;
};

struct al_mark
{
    const void *owner;
    uint32_t id;
};

struct al_arena
{
    unsigned char *bytes;
    size_t capacity, alignment, used, payload, peak, epoch_peak;
    struct al_item items[16];
    unsigned count, mark_count;
    uint32_t serial;

    struct
    {
        uint32_t id;
        unsigned count;
        size_t used;
    } marks[8];
};

int al_extent(size_t cursor, size_t bytes, size_t alignment, size_t capacity,
              size_t *offset, size_t *end);
int al_arena_init(struct al_arena *, unsigned char *, size_t, size_t);
int al_arena_take(struct al_arena *, size_t, size_t, al_cleanup_fn, void *,
                  struct al_handle *);
int al_arena_view(struct al_arena *, struct al_handle, unsigned char **,
                  size_t *);
int al_arena_mark(struct al_arena *, struct al_mark *);
int al_arena_rollback(struct al_arena *, struct al_mark);
int al_arena_commit(struct al_arena *, struct al_mark);
int al_arena_release(struct al_arena *, struct al_handle);
int al_arena_reset(struct al_arena *);

struct al_heap
{
    _Alignas(8) unsigned char bytes[256];
    unsigned capacity, free_count, free_offsets[16];

    struct
    {
        uint32_t id;
        unsigned offset;
    } live[16];

    uint32_t serial;
    unsigned policy, deferred, instrument;
    size_t payload, charged, peak_payload, peak_charged, search_steps;
};

int al_heap_init(struct al_heap *, unsigned capacity, unsigned best_fit,
                 unsigned deferred, unsigned instrument);
int al_heap_check(const struct al_heap *);
int al_heap_take(struct al_heap *, size_t, size_t, struct al_handle *);
int al_heap_release(struct al_heap *, struct al_handle);
int al_heap_resize(struct al_heap *, struct al_handle, size_t, int fail_acquire,
                   size_t fail_copy_after, struct al_handle *);
int al_heap_view(struct al_heap *, struct al_handle, unsigned char **,
                 size_t *);
int al_heap_collect(struct al_heap *);
int al_heap_stats(const struct al_heap *, size_t *total_free,
                  size_t *largest_free);

struct al_pool
{
    algo_pool producer;
};

struct al_pool_handle
{
    const struct al_pool *owner;
    uint64_t raw;
};

void al_pool_init(struct al_pool *);
int al_pool_take(struct al_pool *, int32_t, struct al_pool_handle *);
int al_pool_get(const struct al_pool *, struct al_pool_handle, int32_t *);
int al_pool_release(struct al_pool *, struct al_pool_handle);

struct al_buddy
{
    unsigned char state[15];
    uint32_t ids[15], serial;
    unsigned steps;
};

int al_buddy_init(struct al_buddy *);
int al_buddy_take(struct al_buddy *, size_t, struct al_handle *, size_t *offset,
                  size_t *charged);
int al_buddy_release(struct al_buddy *, struct al_handle);
int al_buddy_check(const struct al_buddy *);

struct al_class_slot
{
    unsigned char data[32];
    uint32_t id;
    unsigned requested, live;
    al_cleanup_fn cleanup;
    void *context;
};

struct al_classes
{
    struct al_class_slot slots[3][4];
    uint32_t serial;
    unsigned retained[3];
};

int al_classes_init(struct al_classes *);
int al_classes_take(struct al_classes *, size_t,
                    int (*initialize)(void *, unsigned char *, size_t),
                    al_cleanup_fn, void *, struct al_handle *);
int al_classes_release(struct al_classes *, struct al_handle);
int al_classes_stats(const struct al_classes *, size_t *payload,
                     size_t *charged, size_t *retained);

struct al_extent
{
    size_t offset, size;
};

int al_extent_choose(struct al_extent *, size_t *count, size_t bytes,
                     size_t minimum, unsigned best_fit, size_t *offset,
                     size_t *charged);
int al_extent_coalesce(struct al_extent *, size_t *count, size_t capacity);

struct al_allocator
{
    void *owner;
    size_t max_alignment;
    int (*allocate)(void *, size_t, size_t, struct al_handle *);
    int (*release)(void *, struct al_handle);
    int (*resize)(void *, struct al_handle, size_t, struct al_handle *);
};

int al_heap_api(struct al_heap *, struct al_allocator *);
int al_api_allocate(const struct al_allocator *, size_t, size_t,
                    struct al_handle *);
typedef int (*al_copy_fn)(void *, unsigned char *, const unsigned char *,
                          size_t);
int al_heap_resize_copy(struct al_heap *, struct al_handle, size_t, int,
                        al_copy_fn, void *, struct al_handle *);

struct al_stable
{
    struct al_heap *heap;

    struct
    {
        struct al_handle inner;
        unsigned generation, state;
    } entries[3];
};

struct al_stable_handle
{
    const struct al_stable *owner;
    unsigned index, generation;
};

int al_stable_init(struct al_stable *, struct al_heap *);
int al_stable_take(struct al_stable *, size_t, struct al_stable_handle *);
int al_stable_view(struct al_stable *, struct al_stable_handle,
                   unsigned char **, size_t *);
int al_stable_resize(struct al_stable *, struct al_stable_handle, size_t, int);
int al_stable_release(struct al_stable *, struct al_stable_handle);
#endif
