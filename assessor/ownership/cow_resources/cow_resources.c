#include "cow_resources.h"
#include <limits.h>
#ifndef OCR_MUTATION
#define OCR_MUTATION 0
#endif
struct ocr_block
{
    unsigned refs;
    struct ocr_resource resource[3];
};

static void discard(struct ocr_memory m, struct ocr_ops ops,
                    struct ocr_block *b)
{
    for (unsigned i = 3; i; i--)
    {
        ops.drop(ops.context, &b->resource[i - 1]);
    }
    m.release(m.context, b);
}

static void drop(struct ocr_store *s, struct ocr_block *b)
{
    if (--b->refs == 0)
    {
        discard(s->memory, s->ops, b);
    }
}

static enum ow_status copy(struct ocr_memory m, struct ocr_ops ops,
                           const struct ocr_resource src[3],
                           struct ocr_block **out)
{
    struct ocr_block *b = m.take(m.context, sizeof(*b));
    if (!b)
    {
        return OW_NOMEM;
    }
    *b = (struct ocr_block){0};
    b->refs = 1;
    for (unsigned i = 0; i < 3; ++i)
    {
        enum ow_status q = ops.clone(ops.context, &src[i], &b->resource[i]);
        if (q != OW_OK || !b->resource[i].data ||
            b->resource[i].length != src[i].length)
        {
            if (OCR_MUTATION == 2)
            {
                b->resource[i] = (struct ocr_resource){0};
            }
            discard(m, ops, b);
            return q == OW_OK ? OW_INVALID : q;
        }
    }
    *out = b;
    return OW_OK;
}

enum ow_status ocr_init(struct ocr_store *s, struct ocr_memory m,
                        struct ocr_ops ops, const struct ocr_resource seed[3])
{
    if (!s || !seed || !m.take || !m.release || !ops.clone || !ops.drop)
    {
        return OW_INVALID;
    }
    for (unsigned i = 0; i < 3; ++i)
    {
        if (!seed[i].data || !seed[i].length || seed[i].length > 16)
        {
            return OW_INVALID;
        }
    }
    struct ocr_block *b = NULL;
    enum ow_status q = copy(m, ops, seed, &b);
    if (q != OW_OK)
    {
        return q;
    }
    if (pthread_mutex_init(&s->mutex, NULL) != 0)
    {
        discard(m, ops, b);
        return OW_INVALID;
    }
    s->memory = m;
    s->ops = ops;
    s->current = b;
    for (unsigned i = 0; i < 4; ++i)
    {
        s->loan[i] = (struct ocr_loan){0};
    }
    s->closed = 0;
    s->ready = 1;
    return OW_OK;
}

static struct ocr_loan *find(struct ocr_store *s, struct ocr_snapshot token)
{
    if (token.owner != s || token.slot >= 4)
    {
        return NULL;
    }
    struct ocr_loan *p = &s->loan[token.slot];
    return p->active && (OCR_MUTATION == 3 || p->serial == token.serial) ? p
                                                                         : NULL;
}

enum ow_status ocr_snapshot(struct ocr_store *s, struct ocr_snapshot *out)
{
    if (!s || !out || !s->ready)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&s->mutex);
    enum ow_status q = OW_FULL;
    if (s->closed && OCR_MUTATION != 4)
    {
        q = OW_DEAD;
    }
    else if (!s->current)
    {
        q = OW_DEAD;
    }
    else if (s->current->refs == UINT_MAX)
    {
        q = OW_FULL;
    }
    else
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            if (!s->loan[i].active && !s->loan[i].retired)
            {
                struct ocr_loan *p = &s->loan[i];
                if (p->serial == UINT_MAX)
                {
                    p->retired = 1;
                    continue;
                }
                ++p->serial;
                p->active = 1;
                p->block = s->current;
                ++p->block->refs;
                *out = (struct ocr_snapshot){s, i, p->serial};
                q = OW_OK;
                break;
            }
        }
    }
    pthread_mutex_unlock(&s->mutex);
    return q;
}

enum ow_status ocr_read(struct ocr_snapshot token, size_t resource,
                        size_t element, int32_t *out)
{
    struct ocr_store *s = token.owner;
    if (!s || !s->ready || !out)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&s->mutex);
    struct ocr_loan *p = find(s, token);
    enum ow_status q = OW_STALE;
    if (p)
    {
        if (resource >= 3 || element >= p->block->resource[resource].length)
        {
            q = OW_INVALID;
        }
        else
        {
            *out = p->block->resource[resource].data[element];
            q = OW_OK;
        }
    }
    pthread_mutex_unlock(&s->mutex);
    return q;
}

enum ow_status ocr_release(struct ocr_snapshot token)
{
    struct ocr_store *s = token.owner;
    if (!s || !s->ready)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&s->mutex);
    struct ocr_loan *p = find(s, token);
    enum ow_status q = OW_STALE;
    if (p)
    {
        struct ocr_block *b = p->block;
        p->active = 0;
        p->block = NULL;
        if (p->serial == UINT_MAX)
        {
            p->retired = 1;
        }
        drop(s, b);
        q = OW_OK;
    }
    pthread_mutex_unlock(&s->mutex);
    return q;
}

enum ow_status ocr_write(struct ocr_store *s, size_t resource, size_t element,
                         int32_t value)
{
    if (!s || !s->ready)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&s->mutex);
    enum ow_status q = OW_OK;
    struct ocr_block *old = s->current;
    if (s->closed || !old)
    {
        q = OW_DEAD;
    }
    else if (resource >= 3 || element >= old->resource[resource].length)
    {
        q = OW_INVALID;
    }
    else if (old->refs == 1 || OCR_MUTATION == 1)
    {
        old->resource[resource].data[element] = value;
    }
    else
    {
        struct ocr_block *fresh = NULL;
        q = copy(s->memory, s->ops, old->resource, &fresh);
        if (q == OW_OK)
        {
            fresh->resource[resource].data[element] = value;
            s->current = fresh;
            drop(s, old);
        }
    }
    pthread_mutex_unlock(&s->mutex);
    return q;
}

enum ow_status ocr_close(struct ocr_store *s)
{
    if (!s || !s->ready)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&s->mutex);
    if (!s->closed)
    {
        s->closed = 1;
        if (OCR_MUTATION != 4)
        {
            struct ocr_block *old = s->current;
            s->current = NULL;
            drop(s, old);
        }
    }
    pthread_mutex_unlock(&s->mutex);
    return OW_OK;
}

enum ow_status ocr_destroy(struct ocr_store *s)
{
    if (!s || !s->ready)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&s->mutex);
    for (unsigned i = 0; i < 4; ++i)
    {
        if (s->loan[i].active)
        {
            pthread_mutex_unlock(&s->mutex);
            return OW_BUSY;
        }
    }
    if (s->current)
    {
        struct ocr_block *old = s->current;
        s->current = NULL;
        drop(s, old);
    }
    s->closed = 1;
    pthread_mutex_unlock(&s->mutex);
    if (pthread_mutex_destroy(&s->mutex) != 0)
    {
        return OW_INVALID;
    }
    s->ready = 0;
    return OW_OK;
}
