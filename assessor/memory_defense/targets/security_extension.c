#include "security_extension.h"
#include <limits.h>
#ifndef MDE_MUTANT
#define MDE_MUTANT 0
#endif
int mde_deliver(struct mde_object *o, struct mde_event e)
{
    if (!o || !o->live || (MDE_MUTANT != 1 && e.generation != o->generation) ||
        e.owner != o->owner)
    {
        return 0;
    }
    o->value = e.value;
    return 1;
}

int mde_retire(struct mde_object *o)
{
    if (!o || !o->live)
    {
        return 0;
    }
    o->live = 0;
    return 1;
}

int mde_recreate(struct mde_object *o, unsigned owner, unsigned value)
{
    if (!o || o->live || o->generation == UINT32_MAX)
    {
        return 0;
    }
    o->generation++;
    o->owner = owner;
    o->value = value;
    o->live = 1;
    return 1;
}

int mde_log(const unsigned char *p, size_t n, char *out, size_t cap)
{
    static const char h[] = "0123456789ABCDEF";
    if (!out || !cap || n > (cap - 1) / 4 || (!p && n))
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        out[4 * i] = '\\';
        out[4 * i + 1] = 'x';
        out[4 * i + 2] = h[p[i] >> 4];
        out[4 * i + 3] = h[p[i] & 15];
    }
    out[4 * n] = 0;
    return 1;
}

int mde_destroy(struct mde_owner *o)
{
    if (!o || o->state != 1 || !o->release)
    {
        return 0;
    }
    void *a = o->first, *b = o->second;
    o->first = o->second = NULL;
    if (MDE_MUTANT != 2)
    {
        o->state = 0;
    }
    if (b)
    {
        o->release(o->context, b);
    }
    if (a)
    {
        o->release(o->context, a);
    }
    o->state = 0;
    return 1;
}
