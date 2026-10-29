#include "law_traces.h"

int mpt_assign(struct mpt_owner *a, struct mpt_view b, struct mpt_allocator c)
{
    (void)a;
    (void)b;
    (void)c;
    return MPT_INVALID;
}

void mpt_drop(struct mpt_owner *a)
{
    (void)a;
}

struct mpt_view mpt_view(const struct mpt_owner *a)
{
    (void)a;
    return (struct mpt_view){0};
}

int mpt_equal(struct mpt_view a, struct mpt_view b, unsigned c, unsigned *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return MPT_INVALID;
}

int mpt_hash(struct mpt_view a, unsigned b, uint64_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return MPT_INVALID;
}
