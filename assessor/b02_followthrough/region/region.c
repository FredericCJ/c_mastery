#include "region.h"
#ifndef BF_REGION_MUTANT
#define BF_REGION_MUTANT 0
#endif
static void object_close(void *v)
{
    struct bf_object *o = v;
    o->live = 0;
    ++o->closes;
    o->owner->trace[o->owner->used++] = o->id;
}

static void reserved(void *v)
{
    (void)v;
}

void bf_cohort_init(struct bf_cohort *c)
{
    mb_region_init(&c->bytes);
    c->used = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        c->objects[i].value = 0;
        c->objects[i].id = i + 1;
        c->objects[i].live = 0;
        c->objects[i].opens = 0;
        c->objects[i].closes = 0;
        c->objects[i].owner = c;
    }
}

int bf_cohort_reset(struct bf_cohort *c)
{
    if (!c)
        return MB_INVALID;
    return mb_region_reset(&c->bytes);
}

int bf_cohort_batch(struct bf_cohort *c, const unsigned char *wire, size_t n,
                    unsigned fail_open, unsigned fail_register, unsigned *out)
{
    if (!c || !wire || !out || !n || wire[0] > 4 || !wire[0] ||
        n != 1 + 3 * (size_t)wire[0] || fail_open > wire[0] ||
        fail_register > wire[0])
        return MB_INVALID;
    if (c->bytes.busy || c->bytes.cursor || c->bytes.count)
        return MB_BUSY;
    for (unsigned i = 0; i < wire[0]; ++i)
        if (!wire[1 + 3 * i])
            return MB_INVALID;
    struct mb_span span;
    int rc = mb_region_alloc(&c->bytes, n, 1, &span);
    if (rc != MB_OK)
        return rc;
    for (size_t i = 0; i < n; ++i)
        c->bytes.bytes[span.offset + i] = wire[i];
    const unsigned char *encoded = c->bytes.bytes + span.offset;
    if (fail_register)
        for (unsigned i = 0; i < 5 - fail_register; ++i)
        {
            rc = mb_region_cleanup(&c->bytes, reserved, c);
            if (rc != MB_OK)
                return rc;
        }
    unsigned sum = 0;
    for (unsigned i = 0; i < encoded[0]; ++i)
    {
        if (fail_open == i + 1)
        {
            mb_region_reset(&c->bytes);
            return MB_FAILED;
        }
        struct bf_object *o = &c->objects[i];
        o->id = encoded[1 + 3 * i];
        o->value = (unsigned)encoded[2 + 3 * i] * 256u + encoded[3 + 3 * i];
        o->live = 1;
        ++o->opens;
        rc = mb_region_cleanup(&c->bytes, object_close, o);
        if (rc != MB_OK)
        {
            if (BF_REGION_MUTANT != 3)
                object_close(o);
            mb_region_reset(&c->bytes);
            return rc;
        }
        sum += o->value;
    }
    *out = sum;
    return MB_OK;
}

void bf_tree_init(struct bf_region_tree *t)
{
    mb_region_init(&t->parent);
    for (unsigned i = 0; i < 2; ++i)
        mb_region_init(&t->child[i]);
}

int bf_tree_child_reset(struct bf_region_tree *t, unsigned i)
{
    if (!t || i >= 2)
        return MB_INVALID;
    return mb_region_reset(&t->child[i]);
}

int bf_tree_reset(struct bf_region_tree *t)
{
    if (!t)
        return MB_INVALID;
    for (unsigned i = 2; i > 0; --i)
    {
        int rc = mb_region_reset(&t->child[i - 1]);
        if (rc != MB_OK)
            return rc;
    }
    return mb_region_reset(&t->parent);
}

int bf_slice_init(struct bf_slice *s, unsigned char *base, size_t n,
                  size_t begin, size_t capacity)
{
    if (!s || !base || n > 64 || begin > n || !capacity || capacity > n - begin)
        return MB_INVALID;
    *s = (struct bf_slice){base, begin, capacity, 0, 0, 0, 1};
    return MB_OK;
}

int bf_slice_alloc(struct bf_slice *s, size_t n, size_t align,
                   struct bf_slice_span *out)
{
    if (!s || !out || !n || !align || align > 8 || (align & (align - 1)))
        return MB_INVALID;
    if (s->generation > 3)
        return MB_FULL;
    size_t absolute = (BF_REGION_MUTANT == 4 ? 0 : s->begin) + s->cursor,
           pad = (align - absolute % align) % align;
    if (pad > s->capacity - s->cursor || n > s->capacity - s->cursor - pad)
        return MB_FULL;
    size_t offset = s->begin + s->cursor + pad;
    s->cursor += pad + n;
    s->payload += n;
    if (s->cursor > s->high)
        s->high = s->cursor;
    *out = (struct bf_slice_span){s, s->generation, offset, n};
    return MB_OK;
}

int bf_slice_read(const struct bf_slice *s, struct bf_slice_span span, size_t i,
                  unsigned char *out)
{
    if (!s || !out)
        return MB_INVALID;
    if (span.owner != s || span.generation != s->generation ||
        s->generation > 3)
        return MB_STALE;
    if (span.offset < s->begin || span.offset - s->begin > s->cursor ||
        span.length > s->cursor - (span.offset - s->begin) || i >= span.length)
        return MB_INVALID;
    *out = s->base[span.offset + i];
    return MB_OK;
}

int bf_slice_reset(struct bf_slice *s)
{
    if (!s)
        return MB_INVALID;
    s->cursor = s->payload = 0;
    if (s->generation <= 3)
        ++s->generation;
    return MB_OK;
}

int bf_growth_init(struct bf_growth *g, struct bf_chunk *first,
                   bf_acquire_chunk_fn acquire, bf_release_chunk_fn release,
                   void *context)
{
    if (!g || !first || !acquire || !release)
        return MB_INVALID;
    g->chunks[0] = first;
    first->cursor = 0;
    g->count = 1;
    g->payload = g->occupied = g->high = 0;
    g->capacity_high = 32;
    g->generation = 1;
    g->busy = 0;
    g->acquire = acquire;
    g->release = release;
    g->context = context;
    return MB_OK;
}

int bf_growth_alloc(struct bf_growth *g, size_t n, size_t align,
                    struct bf_growth_span *out)
{
    if (!g || !out || !n || !align || align > 8 || (align & (align - 1)))
        return MB_INVALID;
    if (g->busy)
        return MB_BUSY;
    if (g->generation > 3 || n > 32)
        return MB_FULL;
    struct bf_chunk *c = g->chunks[g->count - 1];
    size_t pad = (align - c->cursor % align) % align;
    if (pad > 32 - c->cursor || n > 32 - c->cursor - pad)
    {
        if (g->count == 4)
            return MB_FULL;
        g->busy = 1;
        struct bf_chunk *fresh = g->acquire(g->context);
        g->busy = 0;
        if (!fresh)
        {
            if (BF_REGION_MUTANT == 5)
                ++g->payload;
            return MB_FULL;
        }
        fresh->cursor = 0;
        g->chunks[g->count++] = fresh;
        c = fresh;
        pad = 0;
        if (g->count * 32 > g->capacity_high)
            g->capacity_high = g->count * 32;
    }
    size_t offset = c->cursor + pad;
    c->cursor = offset + n;
    g->payload += n;
    g->occupied += pad + n;
    if (g->occupied > g->high)
        g->high = g->occupied;
    *out = (struct bf_growth_span){g, g->generation, g->count - 1, offset, n};
    return MB_OK;
}

int bf_growth_read(const struct bf_growth *g, struct bf_growth_span s, size_t i,
                   unsigned char *out)
{
    if (!g || !out)
        return MB_INVALID;
    if (s.owner != g || s.generation != g->generation || g->generation > 3)
        return MB_STALE;
    if (s.chunk >= g->count || s.offset > g->chunks[s.chunk]->cursor ||
        s.length > g->chunks[s.chunk]->cursor - s.offset || i >= s.length)
        return MB_INVALID;
    *out = g->chunks[s.chunk]->bytes[s.offset + i];
    return MB_OK;
}

int bf_growth_reset(struct bf_growth *g)
{
    if (!g)
        return MB_INVALID;
    if (g->busy)
        return MB_BUSY;
    g->busy = 1;
    while (g->count > 1)
    {
        g->release(g->context, g->chunks[g->count - 1]);
        --g->count;
    }
    g->chunks[0]->cursor = 0;
    g->occupied = g->payload = 0;
    if (g->generation <= 3)
        ++g->generation;
    g->busy = 0;
    return MB_OK;
}

void bf_retry_init(struct bf_retry_region *r)
{
    bf_arena_init(&r->arena);
    r->active = bf_arena_mark(&r->arena, &r->mark) == B2_OK;
}

int bf_retry_reserve(struct bf_retry_region *r, size_t n, size_t align,
                     struct bf_borrow *out)
{
    if (!r || !out)
        return B2_INVALID;
    if (!r->active)
        return B2_FULL;
    return bf_arena_reserve(&r->arena, n, align, out);
}

int bf_retry_register(struct bf_retry_region *r, bf_close_fn f, void *c)
{
    if (!r || !f)
        return B2_INVALID;
    if (!r->active)
        return B2_FULL;
    return bf_arena_register(&r->arena, f, c);
}

int bf_retry_reset(struct bf_retry_region *r)
{
    if (!r)
        return B2_INVALID;
    if (!r->active)
        return B2_OK;
    int rc = bf_arena_rollback(&r->arena, r->mark);
    if (BF_REGION_MUTANT == 6 && rc == BF_ARENA_CLEANUP)
    {
        r->active = 0;
        return B2_OK;
    }
    if (rc != B2_OK)
        return rc;
    r->active = bf_arena_mark(&r->arena, &r->mark) == B2_OK;
    return B2_OK;
}
