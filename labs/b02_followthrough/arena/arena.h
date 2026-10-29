#ifndef BF_ARENA_H
#define BF_ARENA_H
#include "depth.h"

enum
{
    BF_ARENA_BUSY = 7,
    BF_ARENA_CLEANUP = 8
};

typedef int (*bf_close_fn)(void *);
struct bf_arena;

struct bf_borrow
{
    const struct bf_arena *owner;
    unsigned id;
};

struct bf_arena
{
    struct b2_arena raw;
    _Alignas(8) unsigned char bytes[64];

    struct
    {
        unsigned id;
        size_t offset, length;
    } objects[16];

    struct
    {
        bf_close_fn close;
        void *context;
        unsigned done;
    } cleanup[8];

    size_t object_saved[4], cleanup_saved[4], count, cleanup_count;
    unsigned next_object, busy, pending;
    struct b2_mark pending_mark;
};

/* Initialize fresh storage; never reset a live owner by reinitializing it. */
void bf_arena_init(struct bf_arena *);
int bf_arena_reserve(struct bf_arena *, size_t, size_t, struct bf_borrow *);
int bf_arena_get(const struct bf_arena *, struct bf_borrow, size_t,
                 unsigned char *);
int bf_arena_set(struct bf_arena *, struct bf_borrow, size_t, unsigned char);
int bf_arena_mark(struct bf_arena *, struct b2_mark *);
int bf_arena_register(struct bf_arena *, bf_close_fn, void *);
int bf_arena_rollback(struct bf_arena *, struct b2_mark);
int bf_arena_commit(struct bf_arena *, struct b2_mark);
#endif
