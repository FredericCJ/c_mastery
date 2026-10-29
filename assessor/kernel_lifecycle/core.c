#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

unsigned kl_unwind(unsigned mask)
{
    unsigned trace = 0;
    int i;
    if (mask > 7)
        return 0;
    for (i = 2; i >= 0; --i)
        if (mask & (1u << (unsigned)i))
            trace = trace * 10u + (unsigned)i + 1u;
    if (B06_MUTATION == 1 && mask == 3)
        return 12;
    return trace;
}

int kl_copy(unsigned char *dst, size_t cap, const unsigned char *src, size_t n,
            size_t fault_after)
{
    size_t i, k = n < fault_after ? n : fault_after;
    if (n > cap || n > INT_MAX)
        return -1;
    if (B06_MUTATION == 2)
        k = n;
    for (i = 0; i < k; ++i)
        dst[i] = src[i];
    return (int)k;
}

int kl_ref(struct kl_ref *r, unsigned op)
{
    if (op > 1 || r->dead || r->refs == 0)
        return 0;
    if (op == 0)
    {
        if (r->refs == UINT_MAX)
            return 0;
        ++r->refs;
    }
    else
    {
        --r->refs;
        if (r->refs == 0 && B06_MUTATION != 3)
            r->dead = 1;
    }
    return 1;
}

int kl_lock_allowed(unsigned context, unsigned kind, unsigned rt)
{
    if (context > 2 || kind > 2 || rt > 1)
        return 0;
    if (kind == 0)
        return context == 0;
    if (kind == 2)
        return 1;
    return !rt || context == 0 || B06_MUTATION == 4;
}

int kl_work(struct kl_work *w, unsigned e)
{
    if (e == 0)
    {
        if (w->closed || w->queued || w->running)
            return 0;
        w->queued = 1;
    }
    else if (e == 1)
    {
        if (!w->queued)
            return 0;
        w->queued = 0;
        w->running = 1;
    }
    else if (e == 2)
    {
        if (!w->running)
            return 0;
        w->running = 0;
    }
    else if (e == 3)
    {
        w->closed = 1;
    }
    else if (e == 4)
    {
        if (!w->closed || w->queued || (w->running && B06_MUTATION != 5) ||
            w->released)
            return 0;
        w->released = 1;
    }
    else
        return 0;
    return 1;
}

int kl_grace(struct kl_grace *g, unsigned e, unsigned id)
{
    unsigned bit;
    if (id > 7)
        return 0;
    bit = 1u << id;
    if (e == 0)
    {
        if (g->retired || (g->readers & bit))
            return 0;
        g->readers |= bit;
    }
    else if (e == 1)
    {
        if (!(g->readers & bit))
            return 0;
        g->readers &= ~bit;
    }
    else if (e == 2)
        g->retired = 1;
    else if (e == 3)
    {
        if (!g->retired || (g->readers && B06_MUTATION != 6))
            return 0;
        g->retired = 0;
    }
    else
        return 0;
    return 1;
}

int kl_decode(const unsigned char *p, size_t n, unsigned *out)
{
    if (n != 4 || p[0] != 1 || (p[1] != 2 && B06_MUTATION != 7))
        return 0;
    *out = (unsigned)p[2] + 256u * (unsigned)p[3];
    return 1;
}

int kl_can_unload(unsigned h, unsigned c, unsigned closed)
{
    return closed == 1 && h == 0 && (c == 0 || B06_MUTATION == 8);
}
