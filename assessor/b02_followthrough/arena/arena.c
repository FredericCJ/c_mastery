#include "arena.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
void bf_arena_init(struct bf_arena *a)
{
    b2_arena_init(&a->raw);
    a->count = 0;
    a->cleanup_count = 0;
    a->next_object = 1;
    a->busy = 0;
    a->pending = 0;
}

int bf_arena_reserve(struct bf_arena *a, size_t n, size_t align,
                     struct bf_borrow *out)
{
    if (!a || !out)
    {
        return B2_INVALID;
    }
    if (a->busy || a->pending)
    {
        return BF_ARENA_BUSY;
    }
    if (a->count == 16 || a->next_object > 31)
    {
        return B2_FULL;
    }
    size_t offset;
    int rc = b2_reserve(&a->raw, n, align, &offset);
    if (rc != B2_OK)
    {
        return rc;
    }
    size_t i = a->count++;
    a->objects[i].id = a->next_object++;
    a->objects[i].offset = offset;
    a->objects[i].length = n;
    for (size_t j = 0; j < n; ++j)
    {
        a->bytes[offset + j] = 0;
    }
    out->owner = a;
    out->id = a->objects[i].id;
    return B2_OK;
}

static int locate(const struct bf_arena *a, struct bf_borrow b, size_t index,
                  size_t *out)
{
    if (!a)
    {
        return B2_INVALID;
    }
    if (b.owner != a || !b.id)
    {
        return B2_STALE;
    }
    for (size_t i = 0; i < a->count; ++i)
    {
        if (a->objects[i].id == b.id)
        {
            if (index >= a->objects[i].length)
            {
                return B2_RANGE;
            }
            *out = a->objects[i].offset + index;
            return B2_OK;
        }
    }
    return B2_STALE;
}

int bf_arena_get(const struct bf_arena *a, struct bf_borrow b, size_t i,
                 unsigned char *out)
{
    if (!out)
    {
        return B2_INVALID;
    }
    size_t at;
    int rc = locate(a, b, i, &at);
    if (rc == B2_OK)
    {
        *out = a->bytes[at];
    }
    return rc;
}

int bf_arena_set(struct bf_arena *a, struct bf_borrow b, size_t i,
                 unsigned char v)
{
    if (!a)
    {
        return B2_INVALID;
    }
    if (a->busy || a->pending)
    {
        return BF_ARENA_BUSY;
    }
    size_t at;
    int rc = locate(a, b, i, &at);
    if (rc == B2_OK)
    {
        a->bytes[at] = v;
    }
    return rc;
}

int bf_arena_mark(struct bf_arena *a, struct b2_mark *out)
{
    if (!a || !out)
    {
        return B2_INVALID;
    }
    if (a->busy || a->pending)
    {
        return BF_ARENA_BUSY;
    }
    int rc = b2_mark(&a->raw, out);
    if (rc != B2_OK)
    {
        return rc;
    }
    a->object_saved[a->raw.depth - 1] = a->count;
    a->cleanup_saved[a->raw.depth - 1] = a->cleanup_count;
    return B2_OK;
}

int bf_arena_register(struct bf_arena *a, bf_close_fn close, void *context)
{
    if (!a || !close)
    {
        return B2_INVALID;
    }
    if (a->busy || a->pending)
    {
        return BF_ARENA_BUSY;
    }
    if (!a->raw.depth)
    {
        return B2_INVALID;
    }
    if (a->cleanup_count == 8)
    {
        return B2_FULL;
    }
    size_t i = a->cleanup_count++;
    a->cleanup[i].close = close;
    a->cleanup[i].context = context;
    a->cleanup[i].done = 0;
    return B2_OK;
}

static int top(const struct bf_arena *a, struct b2_mark m)
{
    return m.owner == &a->raw && a->raw.depth && m.depth == a->raw.depth &&
           m.id == a->raw.ids[a->raw.depth - 1];
}

int bf_arena_rollback(struct bf_arena *a, struct b2_mark m)
{
    if (!a)
    {
        return B2_INVALID;
    }
    if (a->busy)
    {
        return BF_ARENA_BUSY;
    }
    if (!top(a, m))
    {
        return a->pending ? BF_ARENA_BUSY : B2_STALE;
    }
    size_t depth = a->raw.depth - 1, first = a->cleanup_saved[depth];
    unsigned failed = 0;
    a->busy = 1;
    for (size_t i = a->cleanup_count; i > first; --i)
    {
        if (a->cleanup[i - 1].done && BF_MUTANT != 3)
        {
            continue;
        }
        int rc = a->cleanup[i - 1].close(a->cleanup[i - 1].context);
        if (rc)
        {
            failed = 1;
        }
        else
        {
            a->cleanup[i - 1].done = 1;
        }
    }
    a->busy = 0;
    if (failed && BF_MUTANT != 2)
    {
        a->pending = 1;
        a->pending_mark = m;
        return BF_ARENA_CLEANUP;
    }
    int rc = b2_rollback(&a->raw, m);
    if (rc != B2_OK)
    {
        return rc;
    }
    a->count = a->object_saved[depth];
    a->cleanup_count = first;
    a->pending = 0;
    return B2_OK;
}

int bf_arena_commit(struct bf_arena *a, struct b2_mark m)
{
    if (!a)
    {
        return B2_INVALID;
    }
    if (a->busy || a->pending)
    {
        return BF_ARENA_BUSY;
    }
    if (!top(a, m))
    {
        return B2_STALE;
    }
    if (a->raw.depth < 2)
    {
        return B2_INVALID;
    }
    if (BF_MUTANT == 4)
    {
        a->raw.cursor = a->raw.saved[a->raw.depth - 1];
    }
    --a->raw.depth;
    return B2_OK;
}
