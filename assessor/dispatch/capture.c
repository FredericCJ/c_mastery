#include "capture.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && INT_MAX == INT32_MAX,
               "This capture codec targets eight-bit bytes and 32-bit int");
#ifndef DS_CAPTURE_MUTANT
#define DS_CAPTURE_MUTANT 0
#endif
static unsigned char *allocate_bytes(void *context, size_t n)
{
    struct ow_allocator *pool = context;
    if (!n || n > sizeof(pool->storage[0]))
        return NULL;
    struct v4_allocator allocator = ow_allocator(pool);
    int32_t *p = allocator.take(allocator.context,
                                (n + sizeof(int32_t) - 1) / sizeof(int32_t));
    if (!p)
        return NULL;
    for (unsigned i = 0; i < 8; ++i)
        if (p == pool->storage[i])
            return (unsigned char *)(void *)&pool->storage[i];
    return NULL;
}

static void release_bytes(void *context, unsigned char *bytes)
{
    struct ow_allocator *pool = context;
    struct v4_allocator allocator = ow_allocator(pool);
    for (unsigned i = 0; i < 8; ++i)
        if (bytes == (unsigned char *)(void *)&pool->storage[i])
        {
            allocator.release(allocator.context, pool->storage[i]);
            return;
        }
    ++pool->invalid_releases;
}

struct d_allocator ds_counted_bytes(struct ow_allocator *pool)
{
    return (struct d_allocator){pool, allocate_bytes, release_bytes};
}

int ds_borrowed_i32(const int32_t *p, size_t n, int arg, int *out)
{
    if (!out || !p || !n || n > 8)
        return DS_INVALID;
    int64_t sum = arg;
    for (size_t i = 0; i < n; ++i)
        sum += p[i];
    if (sum < INT_MIN || sum > INT_MAX)
        return DS_RANGE;
    *out = (int)sum;
    return DS_OK;
}

static int decode_call(const unsigned char *p, size_t n, int arg, int *out)
{
    if (!p || !n || n > 32 || n % 4)
        return DS_INVALID;
    int32_t values[8];
    for (size_t i = 0; i < n / 4; ++i)
    {
        uint32_t word = 0;
        for (unsigned byte = 0; byte < 4; ++byte)
            word |= (uint32_t)p[4 * i + byte] << (8 * byte);
        values[i] = word <= INT32_MAX ? (int32_t)word
                                      : -1 - (int32_t)(UINT32_MAX - word);
        if (DS_CAPTURE_MUTANT == 1 && values[i] < 0)
            values[i] = 0;
    }
    return ds_borrowed_i32(values, n / 4, arg, out);
}

int ds_capture_i32(struct ds_erased *owner, const int32_t *p, size_t n,
                   void (*destroy)(void *), void *resource)
{
    if (!owner || !p || !n || n > 8)
        return DS_INVALID;
    unsigned char bytes[32];
    for (size_t i = 0; i < n; ++i)
    {
        uint32_t word = (uint32_t)p[i];
        for (unsigned b = 0; b < 4; ++b)
            bytes[4 * i + b] = (unsigned char)(word >> (8 * b));
    }
    if (DS_CAPTURE_MUTANT == 2 && n > 4)
    {
        owner->destroy = destroy;
        owner->resource = resource;
        owner->active = 1;
        return DS_FAILED;
    }
    return ds_erased_assign(owner, bytes, n * 4, decode_call, destroy,
                            resource);
}

void ds_typed_init(struct ds_typed_capture *c, struct v4_allocator allocator)
{
    *c = (struct ds_typed_capture){0};
    c->allocator = allocator;
}

int ds_typed_assign(struct ds_typed_capture *c, const int32_t *p, size_t n)
{
    if (!c || !p || !n || n > 8 || c->count)
        return DS_INVALID;
    if (n <= 4)
    {
        for (size_t i = 0; i < n; ++i)
            c->data.local[i] = p[i];
        c->count = n;
        return DS_OK;
    }
    if (!c->allocator.take || !c->allocator.release)
        return DS_FULL;
    int32_t *copy = c->allocator.take(c->allocator.context, n);
    if (!copy)
        return DS_FAILED;
    for (size_t i = 0; i < n; ++i)
        copy[i] = p[i];
    c->data.heap = copy;
    c->count = n;
    c->spilled = 1;
    return DS_OK;
}

int ds_typed_call(const struct ds_typed_capture *c, int arg, int *out)
{
    if (!c || !out)
        return DS_INVALID;
    if (!c->count)
        return DS_EMPTY;
    return ds_borrowed_i32(c->spilled ? c->data.heap : c->data.local, c->count,
                           arg, out);
}

int ds_typed_move(struct ds_typed_capture *to, struct ds_typed_capture *from)
{
    if (!to || !from || to == from || to->count || !from->count)
        return DS_INVALID;
    *to = *from;
    if (DS_CAPTURE_MUTANT != 3)
    {
        from->count = 0;
        from->spilled = 0;
        from->data.heap = NULL;
    }
    return DS_OK;
}

void ds_typed_drop(struct ds_typed_capture *c)
{
    if (c->spilled && DS_CAPTURE_MUTANT != 4)
        c->allocator.release(c->allocator.context, c->data.heap);
    c->count = 0;
    c->spilled = 0;
    c->data.heap = NULL;
}
