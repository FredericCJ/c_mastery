#ifndef ALLOCATION_DEPTH_H
#define ALLOCATION_DEPTH_H
#include "algo.h"
#include <stddef.h>
#include <stdint.h>

/*
 * Status codes shared by every allocator in this family.
 *
 * Error contract: unless a function says otherwise, a status other than AL_OK
 * leaves the allocator and every output parameter exactly as they were.
 * AL_CLEANUP is the deliberate exception where a callback failed after the
 * state change had already been made; each function documents it.
 */
enum
{
    AL_OK, /* operation done */
    AL_BAD, /* precondition violated: null, zero, range or alignment */
    AL_FULL, /* no capacity, or no fitting extent, for the request */
    AL_STALE, /* handle does not name a live object of this allocator */
    AL_CORRUPT, /* the allocator's own invariants no longer hold */
    AL_CLEANUP, /* a cleanup or copy callback reported failure */
    AL_RETIRED /* the 32-bit identity counter is exhausted */
};

/* Fixed capacities: every array below, and so every loop over it, is bounded. */
enum al_limits
{
    AL_ARENA_ITEMS = 16, /* live allocations in one arena */
    AL_ARENA_MARKS = 8, /* nested marks in one arena */
    AL_HEAP_BYTES = 256, /* backing store of one al_heap, in bytes */
    AL_HEAP_BLOCKS = 16, /* AL_HEAP_BYTES / the 16-byte minimum block */
    AL_BUDDY_NODES = 15, /* complete binary tree over 64 bytes */
    AL_CLASS_GROUPS = 3, /* size classes of 8, 16 and 32 bytes */
    AL_CLASS_SLOTS = 4, /* slots per size class */
    AL_CLASS_BYTES = 32, /* data bytes of the largest class */
    AL_STABLE_ENTRIES = 3, /* descriptors in one al_stable table */
    AL_EXTENT_MAX = 16 /* extents in one caller-owned free list */
};

/* Cleanup callback: returns 0 on success, nonzero on failure. */
typedef int (*al_cleanup_fn)(void *);

/* Names one live object. `owner` is the allocator that issued it. */
struct al_handle
{
    const void *owner;
    uint32_t id;
};

struct al_item
{
    size_t offset;
    size_t size;
    size_t before; /* arena cursor before this item's padding, in bytes */
    uint32_t id;
    al_cleanup_fn cleanup;
    void *context;
};

struct al_mark
{
    const void *owner;
    uint32_t id;
};

/*
 * Bump arena over caller-owned bytes. The arena never owns `bytes`: the caller
 * keeps that storage alive and aligned to `alignment`. Sizes are in bytes.
 * `serial` numbers items and marks and is never reused.
 */
struct al_arena
{
    unsigned char *bytes;
    size_t capacity;
    size_t alignment;
    size_t used; /* cursor: bytes consumed, padding included */
    size_t payload; /* sum of live item sizes */
    size_t peak; /* highest cursor ever reached */
    size_t epoch_peak; /* highest cursor since the last reset */
    struct al_item items[AL_ARENA_ITEMS];
    unsigned count;
    unsigned mark_count;
    uint32_t serial;

    struct
    {
        uint32_t id;
        unsigned count; /* item count when the mark was taken */
        size_t used; /* cursor when the mark was taken */
    } marks[AL_ARENA_MARKS];
};

/*
 * Pure placement arithmetic. Places `bytes` at the first multiple of
 * `alignment` (a power of two) that is at or after `cursor`.
 * Returns AL_OK and sets *offset and *end = *offset + bytes;
 * AL_BAD for null outputs, zero bytes, non-power-of-two alignment or
 * cursor > capacity; AL_FULL when padding plus bytes exceeds capacity.
 * Outputs are untouched on failure. Never overflows size_t.
 */
int al_extent(size_t cursor, size_t bytes, size_t alignment, size_t capacity,
              size_t *offset, size_t *end);

/*
 * Arena lifecycle. Unless stated, a non-AL_OK status changes nothing.
 *
 * al_arena_init: AL_BAD for null arena, null bytes with nonzero capacity, or
 *   an alignment that is zero or not a power of two. Resets all counters.
 * al_arena_take: reserves `size` bytes at `alignment` <= the arena's. The
 *   optional `cleanup(context)` runs when the item is rolled back, released or
 *   reset; `context` must outlive the item. AL_FULL: no space or 16 items;
 *   AL_RETIRED: serial exhausted.
 * al_arena_view: borrows the item's bytes and length; the pointer is valid
 *   until the item is released, rolled back or the arena is reset. AL_STALE
 *   for a foreign, released or rolled-back handle.
 * al_arena_mark/rollback/commit: marks nest strictly. Only the newest mark may
 *   be rolled back or committed (AL_STALE otherwise). Rollback pops items in
 *   reverse order, running cleanups; every cleanup runs even if one fails, and
 *   the result is AL_CLEANUP (the arena is still rewound). Commit discards the
 *   mark and transfers its items' cleanup duty to the enclosing scope.
 * al_arena_release: releases only the newest item (AL_BAD otherwise) and not
 *   an item older than the newest mark. Runs its cleanup (AL_CLEANUP on
 *   failure, item still released).
 * al_arena_reset: pops everything in reverse order, clears marks and the
 *   epoch peak; `peak` is kept. AL_CLEANUP if any cleanup failed.
 */
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

/*
 * Boundary-tag heap in a fixed 256-byte store. The 8-byte-aligned block chain
 * is the single source of truth; `free_offsets` and `live` are indexes that
 * al_heap_check cross-checks against the chain before every mutation.
 */
struct al_heap
{
    _Alignas(8) unsigned char bytes[AL_HEAP_BYTES];
    unsigned capacity;
    unsigned free_count;
    unsigned free_offsets[AL_HEAP_BLOCKS];

    struct
    {
        uint32_t id;
        unsigned offset;
    } live[AL_HEAP_BLOCKS];

    uint32_t serial;
    unsigned policy; /* 0 first fit, 1 best fit */
    unsigned deferred; /* 1: free blocks merge only in al_heap_collect */
    unsigned instrument; /* 1: count search steps */
    size_t payload;
    size_t charged;
    size_t peak_payload;
    size_t peak_charged;
    size_t search_steps;
};

/*
 * Heap operations. Every one except init first runs al_heap_check and
 * returns AL_CORRUPT without touching the heap if it fails.
 *
 * al_heap_init: capacity is 16..256 bytes in multiples of 8; the three flags
 *   are 0 or 1. AL_BAD otherwise.
 * al_heap_take: payload 1..244 bytes at alignment 1, 2, 4 or 8. AL_FULL when
 *   no free block fits or 16 objects are live. Transactional.
 * al_heap_view: borrows the payload; valid until that object is released or
 *   resized. AL_STALE for an unknown handle.
 * al_heap_resize: always returns a NEW handle (the old one becomes stale)
 *   and may move the payload. On the moving path only, `fail_acquire`
 *   simulates acquisition failure (AL_FULL) and `fail_copy_after` makes the
 *   copy fail once it must move more than that many bytes (AL_CLEANUP;
 *   SIZE_MAX never fails). On any failure the heap is unchanged.
 * al_heap_resize_copy: as resize, with a caller copy callback; a nonzero
 *   callback result gives AL_CLEANUP with the heap unchanged.
 * al_heap_collect: merges adjacent free blocks. al_heap_stats: free bytes in
 *   total and in the largest free block, header overhead included.
 */
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

/*
 * Pool of three int32 slots layered on the algorithms-family producer.
 * Slots 3..15 of the producer are permanently retired. A slot is retired
 * after its third generation. Handles carry the pool that issued them.
 */
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

/*
 * Binary buddy allocator over 64 bytes in 8-byte leaves. State per node:
 * 0 free, 1 split, 2 allocated. `steps` counts nodes visited by the last
 * successful operation (a bound on its work, not a timing).
 */
struct al_buddy
{
    unsigned char state[AL_BUDDY_NODES];
    uint32_t ids[AL_BUDDY_NODES];
    uint32_t serial;
    unsigned steps;
};

/*
 * al_buddy_take: request 1..64 bytes, rounded up to a power of two >= 8;
 * *offset is relative to the managed region and *charged the rounded size.
 * AL_FULL leaves the tree unchanged. al_buddy_release merges free siblings
 * upward. take, release and check validate the whole tree (AL_CORRUPT).
 */
int al_buddy_init(struct al_buddy *);
int al_buddy_take(struct al_buddy *, size_t, struct al_handle *, size_t *offset,
                  size_t *charged);
int al_buddy_release(struct al_buddy *, struct al_handle);
int al_buddy_check(const struct al_buddy *);

struct al_class_slot
{
    unsigned char data[AL_CLASS_BYTES];
    uint32_t id;
    unsigned requested;
    unsigned live;
    al_cleanup_fn cleanup;
    void *context;
};

/*
 * Three size classes (8, 16, 32 bytes) of four slots each. `retained[g]` is 1
 * once class g has ever been used: its whole 4-slot region stays reserved.
 */
struct al_classes
{
    struct al_class_slot slots[AL_CLASS_GROUPS][AL_CLASS_SLOTS];
    uint32_t serial;
    unsigned retained[AL_CLASS_GROUPS];
};

/*
 * al_classes_take: size 1..32. `initialize(context, data, size)` fills a
 * scratch buffer; nonzero means failure and gives AL_CLEANUP with the table
 * unchanged. The optional `cleanup(context)` runs on release; its failure is
 * AL_CLEANUP but the slot is already freed and zeroed.
 * al_classes_stats reports requested bytes, charged class bytes and retained
 * bytes (4 slots of every used class).
 */
int al_classes_init(struct al_classes *);
int al_classes_take(struct al_classes *, size_t,
                    int (*initialize)(void *, unsigned char *, size_t),
                    al_cleanup_fn, void *, struct al_handle *);
int al_classes_release(struct al_classes *, struct al_handle);
int al_classes_stats(const struct al_classes *, size_t *payload,
                     size_t *charged, size_t *retained);

struct al_extent
{
    size_t offset;
    size_t size;
};

/*
 * Free-list policy helpers over a caller-owned array of at most 16 extents;
 * `*count` is the number in use.
 *
 * al_extent_choose: every extent must have size > 0, and the extents must be
 *   sorted by offset and non-overlapping. AL_BAD for that, for bytes == 0,
 *   minimum == 0, best_fit > 1, a null argument or *count > 16. First fit,
 *   or best fit when `best_fit` is 1 (smallest sufficient, earliest on
 *   ties). A remainder smaller than `minimum` is absorbed, so *charged may
 *   exceed `bytes`. AL_FULL if nothing fits; the array is changed only on
 *   AL_OK.
 * al_extent_coalesce: accepts any order; sorts a copy, merges touching
 *   extents and writes the result back. AL_BAD for an empty extent, one
 *   beyond `capacity` or an overlap, with the array unchanged.
 */
int al_extent_choose(struct al_extent *, size_t *count, size_t bytes,
                     size_t minimum, unsigned best_fit, size_t *offset,
                     size_t *charged);
int al_extent_coalesce(struct al_extent *, size_t *count, size_t capacity);

/* Type-erased allocator interface; `owner` is passed back to each callback. */
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

/*
 * Stable handles over a heap: a client holds an (index, generation) pair that
 * survives the heap moving the payload on resize. An entry is retired after
 * its third generation. The table does not own the heap.
 */
struct al_stable
{
    struct al_heap *heap;

    struct
    {
        struct al_handle inner;
        unsigned generation;
        unsigned state; /* 0 free, 1 live, 2 retired */
    } entries[AL_STABLE_ENTRIES];
};

struct al_stable_handle
{
    const struct al_stable *owner;
    unsigned index;
    unsigned generation;
};

int al_stable_init(struct al_stable *, struct al_heap *);
int al_stable_take(struct al_stable *, size_t, struct al_stable_handle *);
int al_stable_view(struct al_stable *, struct al_stable_handle,
                   unsigned char **, size_t *);
int al_stable_resize(struct al_stable *, struct al_stable_handle, size_t, int);
int al_stable_release(struct al_stable *, struct al_stable_handle);
#endif
