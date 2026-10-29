#include "kernel.h"

enum ak_result ak_copy(struct ak_packet *p, size_t n, ak_copy_fn copy,
                       void *context)
{
    struct ak_packet next;
    size_t i, left;
    if (!p || !copy || !n || n > 32)
    {
        return AK_INVALID;
    }
    for (i = 0; i < 32; ++i)
    {
        next.bytes[i] = 0;
    }
    left = copy(context, next.bytes, n);
    if (left > n)
    {
        return AK_INVALID;
    }
#ifndef AK_MUTANT_COPY
    if (left)
    {
        return AK_FAULT;
    }
#endif
    next.length = n;
    *p = next;
    return AK_OK;
}

enum ak_result ak_init(struct ak_service *s, struct ak_backend b)
{
    if (!s || !b.map || !b.sync_device || !b.submit || !b.quiesce ||
        !b.sync_cpu || !b.unmap)
    {
        return AK_INVALID;
    }
    if (as_init(&s->queue, 4) != CMX_OK)
    {
        return AK_INVALID;
    }
    s->backend = b;
    s->state = 0;
    s->cookie = 0;
    return AK_OK;
}

enum ak_result ak_enqueue(struct ak_service *s, struct as_value v)
{
    enum cmx_result r;
    if (!s)
    {
        return AK_INVALID;
    }
    r = as_push(&s->queue, v, 0, 0);
    return r == CMX_OK ? AK_OK : r == CMX_FULL ? AK_BUSY : AK_INVALID;
}

enum ak_result ak_start(struct ak_service *s)
{
    struct ak_backend *b;
    unsigned cookie;
    if (!s)
    {
        return AK_INVALID;
    }
    if (s->state == 2)
    {
        return AK_BUSY;
    }
    if (!s->state)
    {
        if (as_pop(&s->queue, &s->pending, 0, 0) != CMX_OK)
        {
            return AK_EMPTY;
        }
        s->state = 1;
    }
    b = &s->backend;
    if (b->map(b->context, &s->pending, &cookie))
    {
        return AK_IO;
    }
    s->cookie = cookie;
#ifndef AK_MUTANT_SYNC
    b->sync_device(b->context, cookie);
#endif
    if (b->submit(b->context, cookie))
    {
        b->unmap(b->context, cookie);
        return AK_IO;
    }
    s->state = 2;
    return AK_OK;
}

enum ak_result ak_complete(struct ak_service *s, struct as_value *out)
{
    struct ak_backend *b;
    if (!s || !out)
    {
        return AK_INVALID;
    }
    if (s->state != 2)
    {
        return AK_BUSY;
    }
    b = &s->backend;
#ifndef AK_MUTANT_QUIESCE
    if (!b->quiesce(b->context, s->cookie))
    {
        return AK_BUSY;
    }
#endif
    b->sync_cpu(b->context, s->cookie);
    b->unmap(b->context, s->cookie);
    *out = s->pending;
    s->state = 0;
    return AK_OK;
}
