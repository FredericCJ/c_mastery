#include "arena.h"

void bf_arena_init(struct bf_arena *a)
{
    (void)a;
}

int bf_arena_reserve(struct bf_arena *a, size_t n, size_t l,
                     struct bf_borrow *b)
{
    (void)a;
    (void)n;
    (void)l;
    (void)b;
    return B2_FAILED;
}

int bf_arena_get(const struct bf_arena *a, struct bf_borrow b, size_t i,
                 unsigned char *out)
{
    (void)a;
    (void)b;
    (void)i;
    (void)out;
    return B2_FAILED;
}

int bf_arena_set(struct bf_arena *a, struct bf_borrow b, size_t i,
                 unsigned char v)
{
    (void)a;
    (void)b;
    (void)i;
    (void)v;
    return B2_FAILED;
}

int bf_arena_mark(struct bf_arena *a, struct b2_mark *m)
{
    (void)a;
    (void)m;
    return B2_FAILED;
}

int bf_arena_register(struct bf_arena *a, bf_close_fn f, void *c)
{
    (void)a;
    (void)f;
    (void)c;
    return B2_FAILED;
}

int bf_arena_rollback(struct bf_arena *a, struct b2_mark m)
{
    (void)a;
    (void)m;
    return B2_FAILED;
}

int bf_arena_commit(struct bf_arena *a, struct b2_mark m)
{
    (void)a;
    (void)m;
    return B2_FAILED;
}
