#include "fault_adapter.h"

int mpt_replay(const struct mpt_trace *a, struct mpt_laws b,
               struct mpt_outcome *c)
{
    (void)a;
    (void)b;
    (void)c;
    return MPT_INVALID;
}

int mpt_shrink(const struct mpt_trace *a, struct mpt_laws b,
               struct mpt_trace *c, unsigned *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return MPT_INVALID;
}
