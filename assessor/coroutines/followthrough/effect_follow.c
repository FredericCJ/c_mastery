#include "effect_follow.h"
#ifndef CH_MUTANT
#define CH_MUTANT 0
#endif
static int handlers(const struct ch_handler *h)
{
    const struct ch_handler *seen[4];
    unsigned count = 0;
    while (h)
    {
        if (count == 4 || !h->answer || h->questions > 3)
        {
            return 0;
        }
        for (unsigned i = 0; i < count; i++)
        {
            if (seen[i] == h)
            {
                return 0;
            }
        }
        seen[count++] = h;
        h = h->parent;
    }
    return 1;
}

static void drop(struct ch_effect *f)
{
    if (f->resource)
    {
        unsigned char *p = f->resource;
        f->resource = NULL;
        f->allocator.release(f->allocator.context, p);
    }
}

int ch_create(struct co_allocator a, const struct ch_handler *h,
              unsigned dynamic, const unsigned char bytes[4],
              struct ch_effect **out)
{
    if (!out || !bytes || !a.allocate || !a.release || dynamic > 1 ||
        !handlers(h))
    {
        return CO_INVALID;
    }
    struct ch_effect *f = a.allocate(a.context, sizeof *f);
    if (!f)
    {
        return CO_RANGE;
    }
    unsigned char *p = a.allocate(a.context, 4);
    if (!p)
    {
        a.release(a.context, f);
        return CO_RANGE;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = bytes[i];
    }
    *f = (struct ch_effect){.allocator = a,
                            .lexical = h,
                            .dynamic = dynamic,
                            .active = 1,
                            .resource = p};
    co_effect_start(&f->core, &f->token);
    *out = f;
    return CO_OK;
}

int ch_step(struct ch_effect *f, const struct ch_handler *dynamic,
            uint32_t *out)
{
    if (!f || !out)
    {
        return CO_INVALID;
    }
    if (!f->active)
    {
        return CO_STATE;
    }
    const struct ch_handler *h = f->dynamic ? dynamic : f->lexical;
    if (!handlers(h))
    {
        return CO_INVALID;
    }
    unsigned q = f->token.question;
    while (h && !(h->questions & (1u << (q - 1))))
    {
        h = h->parent;
    }
    if (!h)
    {
        return CO_STATE;
    }
    uint32_t answer = 0;
    int r = h->answer(h->context, q, &answer);
    if (r == CO_PENDING)
    {
        return r;
    }
    if (r)
    {
        return CO_IO;
    }
    struct co_continuation next = {0};
    uint32_t value = 0;
    r = co_effect_resume(&f->token, answer, &next, &value);
    if (r == CO_REQUEST)
    {
        f->token = next;
    }
    else if (r == CO_DONE)
    {
        f->active = 0;
        drop(f);
        *out = value;
    }
    return r;
}

int ch_clone(const struct ch_effect *from, struct ch_effect **out)
{
    if (!from || !out)
    {
        return CO_INVALID;
    }
    if (!from->active || !from->resource)
    {
        return CO_STATE;
    }
    struct co_allocator a = from->allocator;
    struct ch_effect *f = a.allocate(a.context, sizeof *f);
    if (!f)
    {
        return CO_RANGE;
    }
    unsigned char *p =
        CH_MUTANT == 1 ? from->resource : a.allocate(a.context, 4);
    if (!p)
    {
        a.release(a.context, f);
        return CO_RANGE;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = from->resource[i];
    }
    *f = *from;
    f->resource = p;
    f->token.owner = &f->core;
    *out = f;
    return CO_OK;
}

int ch_abort(struct ch_effect *f)
{
    if (!f)
    {
        return CO_INVALID;
    }
    if (!f->active)
    {
        return CO_OK;
    }
    int r = co_effect_abort(&f->token);
    if (r)
    {
        return r;
    }
    f->active = 0;
    if (CH_MUTANT != 2)
    {
        drop(f);
    }
    return CO_OK;
}

int ch_destroy(struct ch_effect **owner)
{
    if (!owner)
    {
        return CO_INVALID;
    }
    if (!*owner)
    {
        return CO_OK;
    }
    struct ch_effect *f = *owner;
    int r = ch_abort(f);
    if (r)
    {
        return r;
    }
    struct co_allocator a = f->allocator;
    *owner = NULL;
    a.release(a.context, f);
    return CO_OK;
}
