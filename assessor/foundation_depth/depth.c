#include "depth.h"

enum fd_class fd_verdict(enum fd_case q)
{
    static const enum fd_class answers[FD_CASE_COUNT] = {
        FD_DEFINED,
        FD_DEFINED,
        FD_UNDEFINED,
        FD_UNSPECIFIED,
        FD_IMPLEMENTATION_DEFINED,
        FD_UNDEFINED,
        FD_UNDEFINED,
        FD_CONSTRAINT,
        FD_UNDEFINED,
        FD_DEFINED};
#ifdef FD_MUTATE_SEMANTIC
    if (q == FD_UNSEQUENCED)
    {
        return FD_UNSPECIFIED;
    }
#endif
    return (unsigned)q < FD_CASE_COUNT ? answers[q] : FD_UNKNOWN;
}

enum fd_status fd_repair_step(int *value, int *previous)
{
    if (!value || !previous || value == previous)
    {
        return FD_INVALID;
    }
    if (*value == INT_MAX)
    {
        return FD_RANGE;
    }
    *previous = *value;
    *value += 1;
    return FD_OK;
}

enum fd_status fd_extent(size_t offset, size_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (!out)
    {
        return FD_INVALID;
    }
#ifdef FD_MUTATE_ZERO_OFFSET
    if (count == 0)
    {
        *out = offset;
        return FD_OK;
    }
#endif
#ifdef FD_MUTATE_EARLY_EXTENT
    *out = offset;
#endif
#ifdef FD_MUTATE_EXTENT
    size_t wrapped = offset + count * width;
    if (wrapped > limit)
    {
        return FD_RANGE;
    }
    *out = wrapped;
#else
    if (offset > limit || (width && count > (limit - offset) / width))
    {
        return FD_RANGE;
    }
    *out = offset + count * width;
#endif
    return FD_OK;
}

enum fd_status fd_from_signed(intmax_t value, size_t *out)
{
    if (!out)
    {
        return FD_INVALID;
    }
    if (value < 0 || (uintmax_t)value > SIZE_MAX)
    {
        return FD_RANGE;
    }
    *out = (size_t)value;
    return FD_OK;
}

static int cursor_valid(const struct fd_cursor *c)
{
    return c && c->position <= c->length && (c->base || c->length == 0);
}

enum fd_status fd_take(struct fd_cursor *c, size_t n, const unsigned char **out)
{
    if (!out || !cursor_valid(c))
    {
        return FD_INVALID;
    }
    if (n > c->length - c->position)
    {
#ifdef FD_MUTATE_CURSOR
        c->position = c->length;
#endif
        return FD_RANGE;
    }
    const unsigned char *p = c->base ? c->base + c->position : 0;
    c->position += n;
    *out = p;
    return FD_OK;
}

enum fd_status fd_subview(const struct fd_cursor *c, size_t offset, size_t n,
                          struct fd_cursor *out)
{
    if (!out || !cursor_valid(c))
    {
        return FD_INVALID;
    }
    size_t remaining = c->length - c->position;
    if (offset > remaining || n > remaining - offset)
    {
        return FD_RANGE;
    }
    const unsigned char *p = c->base ? c->base + c->position + offset : 0;
    out->base = p;
    out->length = n;
    out->position = 0;
    return FD_OK;
}

enum fd_status fd_encode(struct fd_record r, unsigned char *out,
                         size_t capacity)
{
    if (!out || (r.flags & ~3u))
    {
        return FD_INVALID;
    }
    if (capacity != 8)
    {
        return FD_RANGE;
    }
    uint32_t bits = (uint32_t)r.value;
    out[0] = 1;
    out[1] = r.flags;
    out[2] = (unsigned char)(r.count >> 8);
    out[3] = (unsigned char)r.count;
    for (unsigned i = 0; i < 4; i++)
    {
        out[4 + i] = (unsigned char)(bits >> (24 - 8 * i));
    }
    return FD_OK;
}

enum fd_status fd_decode(const unsigned char *wire, size_t length,
                         struct fd_record *out)
{
    if (!out || !wire)
    {
        return FD_INVALID;
    }
    size_t extent;
    if (fd_extent(4, 1, 4, length, &extent) != FD_OK || extent != length)
    {
        return FD_RANGE;
    }
    struct fd_cursor c = {wire, length, 0};
    const unsigned char *header, *payload;
    if (fd_take(&c, 4, &header) != FD_OK || fd_take(&c, 4, &payload) != FD_OK)
    {
        return FD_RANGE;
    }
    if (header[0] != 1 || (header[1] & ~3u))
    {
        return FD_INVALID;
    }
    uint32_t bits = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        bits = (bits << 8) | payload[i];
    }
#ifdef FD_MUTATE_CODEC
    bits &= UINT32_C(0x7fffffff);
#endif
    int32_t value =
        bits <= INT32_MAX ? (int32_t)bits : -1 - (int32_t)(UINT32_MAX - bits);
    out->flags = header[1];
    out->count = (uint16_t)(((uint16_t)header[2] << 8) | header[3]);
    out->value = value;
    return FD_OK;
}

enum fd_status fd_slot_acquire(struct fd_slot *s, struct fd_allocator a)
{
    if (!s || !a.allocate || !a.deallocate)
    {
        return FD_INVALID;
    }
    if (s->state != FD_EMPTY)
    {
        return FD_STATE;
    }
    void *p = a.allocate(a.context, sizeof(struct fd_value),
                         _Alignof(struct fd_value));
    if (!p)
    {
        return FD_NOMEM;
    }
    s->storage = p;
    s->allocator = a;
    s->state = FD_ACQUIRED;
    s->destroy = 0;
    s->callback_context = 0;
    return FD_OK;
}

void fd_slot_reset(struct fd_slot *s)
{
    if (!s || s->state == FD_EMPTY)
    {
        return;
    }
    if ((s->state == FD_INITIALIZED || s->state == FD_PUBLISHED) && s->destroy)
    {
        s->destroy(s->callback_context, s->storage);
    }
    s->allocator.deallocate(s->allocator.context, s->storage);
    s->storage = 0;
    s->state = FD_EMPTY;
    s->allocator.context = 0;
    s->allocator.allocate = 0;
    s->allocator.deallocate = 0;
    s->destroy = 0;
    s->callback_context = 0;
}

enum fd_status fd_slot_initialize(struct fd_slot *s,
                                  fd_initialize_fn initialize,
                                  fd_destroy_fn destroy, void *context)
{
    if (!s || !initialize)
    {
        return FD_INVALID;
    }
    if (s->state != FD_ACQUIRED)
    {
        return FD_STATE;
    }
    if (initialize(context, s->storage))
    {
#ifndef FD_MUTATE_LIFECYCLE
        fd_slot_reset(s);
#endif
        return FD_INIT_FAILED;
    }
    s->destroy = destroy;
    s->callback_context = context;
    s->state = FD_INITIALIZED;
    return FD_OK;
}

enum fd_status fd_slot_publish(struct fd_slot *s, const struct fd_value **out)
{
    if (!s || !out)
    {
        return FD_INVALID;
    }
    if (s->state != FD_INITIALIZED && s->state != FD_PUBLISHED)
    {
        return FD_STATE;
    }
    *out = s->storage;
    s->state = FD_PUBLISHED;
    return FD_OK;
}

void fd_transform(unsigned char *restrict d, const unsigned char *restrict s,
                  size_t n, unsigned char mask)
{
    for (size_t i = 0; i < n; i++)
    {
        d[i] = (unsigned char)(s[i] ^ mask);
    }
}

enum fd_status fd_move(unsigned char *base, size_t length, size_t dst,
                       size_t src, size_t count)
{
    if (!base && length)
    {
        return FD_INVALID;
    }
    if (dst > length || src > length || count > length - dst ||
        count > length - src)
    {
        return FD_RANGE;
    }
#ifndef FD_MUTATE_OVERLAP
    if (dst > src)
    {
        for (size_t i = count; i > 0; i--)
        {
            base[dst + i - 1] = base[src + i - 1];
        }
    }
    else
#endif
    {
        for (size_t i = 0; i < count; i++)
        {
            base[dst + i] = base[src + i];
        }
    }
    return FD_OK;
}

void fd_pool_init(struct fd_pool *p)
{
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        p->entries[i].generation = 1;
        p->entries[i].live = 0;
        p->entries[i].retired = 0;
    }
}

enum fd_status fd_pool_acquire(struct fd_pool *p, struct fd_value value,
                               struct fd_handle *out)
{
    if (!p || !out)
    {
        return FD_INVALID;
    }
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        struct fd_entry *e = &p->entries[i];
        if (!e->live && !e->retired)
        {
            e->value = value;
            e->live = 1;
            out->owner = p;
            out->index = i;
            out->generation = e->generation;
            return FD_OK;
        }
    }
    return FD_FULL;
}

static int handle_valid(const struct fd_pool *p, struct fd_handle h)
{
    return p && h.owner == p && h.index < FD_POOL_CAPACITY &&
           p->entries[h.index].live &&
           p->entries[h.index].generation == h.generation;
}

enum fd_status fd_pool_get(const struct fd_pool *p, struct fd_handle h,
                           struct fd_value *out)
{
    if (!p || !out)
    {
        return FD_INVALID;
    }
    if (!handle_valid(p, h))
    {
        return FD_STALE;
    }
    *out = p->entries[h.index].value;
    return FD_OK;
}

enum fd_status fd_pool_release(struct fd_pool *p, struct fd_handle h)
{
    if (!p)
    {
        return FD_INVALID;
    }
    if (!handle_valid(p, h))
    {
        return FD_STALE;
    }
    struct fd_entry *e = &p->entries[h.index];
    e->live = 0;
#ifdef FD_MUTATE_HANDLES
    e->generation = e->generation == FD_GENERATION_MAX ? 1 : e->generation + 1;
#else
    if (e->generation == FD_GENERATION_MAX)
    {
        e->retired = 1;
    }
    else
    {
        e->generation++;
    }
#endif
    return FD_OK;
}

enum fd_status fd_apply(const unsigned char *wire, size_t length,
                        int authorized, int32_t *counter)
{
    if (!counter)
    {
        return FD_INVALID;
    }
    struct fd_record r;
    enum fd_status status = fd_decode(wire, length, &r);
    if (status != FD_OK)
    {
        return status;
    }
    int64_t result = (int64_t)*counter + (int64_t)r.count * r.value;
    if (result < INT32_MIN || result > INT32_MAX)
    {
        return FD_RANGE;
    }
#ifdef FD_MUTATE_DEFENSE
    *counter = (int32_t)result;
#endif
#ifdef FD_MUTATE_FLAG_AUTHORITY
    if (!authorized && !(r.flags & 1u))
    {
        return FD_DENIED;
    }
#else
    if (!authorized)
    {
        return FD_DENIED;
    }
#endif
    *counter = (int32_t)result;
    return FD_OK;
}
