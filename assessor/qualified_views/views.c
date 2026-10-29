#include "views.h"
#ifndef FV_MUTANT
#define FV_MUTANT 0
#endif
struct fv_owner
{
    struct fv_record record;
};
static struct fv_owner sole_owner;

struct fv_view fv_initialize(struct fv_record *out)
{
    if (!out)
        return (struct fv_view){NULL, 0};
    /* Copy values while the literal lives; the returned pointer designates caller storage. */
    *out = (struct fv_record){.values = {[1] = 7, [3] = 9}, .link = NULL};
    return fv_view(out);
}

enum fv_status fv_reset(struct fv_record *r)
{
    if (!r)
        return FV_INVALID;
    *r = (struct fv_record){0};
    if (FV_MUTANT == 1)
        r->values[3] = 1;
    return FV_OK;
}

enum fv_status fv_update(struct fv_record *r, size_t i, int v)
{
    if (!r)
        return FV_INVALID;
    if (i >= 4)
        return FV_RANGE;
    r->values[i] = v;
    return FV_OK;
}

struct fv_view fv_view(const struct fv_record *r)
{
    return r ? (struct fv_view){r->values, 4} : (struct fv_view){NULL, 0};
}

enum fv_status fv_copy(struct fv_destination d, struct fv_view s)
{
    if ((s.length && !s.data) || (d.capacity && !d.data))
        return FV_INVALID;
    if (d.capacity < s.length)
    {
        if (FV_MUTANT == 2 && d.capacity)
            d.data[0] = 0;
        return FV_FULL;
    }
    int *restrict out = d.data;
    const int *restrict in = s.data;
    for (size_t i = 0; i < s.length; ++i)
        out[i] = in[i];
    return FV_OK;
}

enum fv_status fv_move(int *base, size_t extent, size_t d, size_t s, size_t n)
{
    if (extent && !base)
        return FV_INVALID;
    if (d > extent || s > extent || n > extent - d || n > extent - s)
        return FV_RANGE;
    if (FV_MUTANT != 3 && d > s && d - s < n)
    {
        for (size_t i = n; i > 0; --i)
            base[d + i - 1] = base[s + i - 1];
    }
    else
    {
        for (size_t i = 0; i < n; ++i)
            base[d + i] = base[s + i];
    }
    return FV_OK;
}

struct fv_owner *fv_owner(void)
{
    return &sole_owner;
}

struct fv_view fv_owner_view(const struct fv_owner *p)
{
    return p ? fv_view(&p->record) : (struct fv_view){NULL, 0};
}

int *fv_owner_mutable(struct fv_owner *p)
{
    return p ? p->record.values : NULL;
}

unsigned fv_device_twice(volatile unsigned *status)
{
    unsigned first = *status;
    unsigned second = FV_MUTANT == 4 ? first : *status;
    return first ^ second;
}
