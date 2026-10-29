#include "alternatives.h"
#ifndef CX_MUTANT
#define CX_MUTANT 0
#endif
static int selected(struct cr_unfused *f, uint32_t *out)
{
    for (;;)
    {
        uint32_t x;
        int r = co_next(&f->flow.source, &x);
        f->flow.pulls++;
        if (r)
        {
            return r;
        }
        f->predicates++;
        if (x % 2 == 0)
        {
            *out = x;
            return CO_OK;
        }
    }
}

static uint32_t mapped(struct cr_unfused *f, uint32_t x)
{
    f->maps++;
    return x * x;
}

int cr_unfused_next(struct cr_unfused *f, uint32_t *out)
{
    if (!f || !out)
    {
        return CO_INVALID;
    }
    if (CX_MUTANT != 1 && !f->flow.remaining)
    {
        return CO_DONE;
    }
    uint32_t x;
    int r = selected(f, &x);
    if (r)
    {
        return r;
    }
    *out = CX_MUTANT == 2 ? x : mapped(f, x);
    f->flow.remaining--;
    return CO_OK;
}

int cr_caller_init(struct cr_caller_generator *f, unsigned count)
{
    if (!f || count > 47)
    {
        return CO_INVALID;
    }
    *f = (struct cr_caller_generator){0, 1, 0, count};
    return CO_OK;
}

int cr_caller_next(struct cr_caller_generator *f, uint32_t *out)
{
    if (!f || !out)
    {
        return CO_INVALID;
    }
    if (CX_MUTANT == 3 ? f->index > f->count : f->index >= f->count)
    {
        return CO_DONE;
    }
    *out = f->a;
    f->index++;
    if (f->index < f->count)
    {
        uint32_t n = f->a + f->b;
        f->a = f->b;
        f->b = n;
    }
    if (CX_MUTANT == 4)
    {
        *out = 0;
    }
    return CO_OK;
}

int cr_push(struct cf_batch *f, size_t available, unsigned eof, cr_sink_fn sink,
            void *ctx, unsigned budget, size_t *emitted)
{
    if (!f || !sink || !emitted)
    {
        return CO_INVALID;
    }
    size_t count = 0;
    while (budget--)
    {
        struct cf_batch next = *f;
        unsigned key;
        uint32_t sum;
        int r = cf_batch_poll(&next, available, eof, 1, &key, &sum);
        if (r != CO_OK)
        {
            if (r == CO_PENDING || r == CO_DONE)
            {
                *f = next;
            }
            *emitted = count;
            return r;
        }
        int accepted = sink(ctx, key, sum);
        if (accepted == CO_PENDING)
        {
            if (CX_MUTANT == 5)
            {
                *f = next;
            }
            *emitted = count;
            return CO_PENDING;
        }
        if (accepted != CO_OK)
        {
            *emitted = count;
            return CO_IO;
        }
        *f = next;
        count++;
        if (CX_MUTANT == 6)
        {
            count++;
        }
    }
    *emitted = count;
    return CO_PENDING;
}
