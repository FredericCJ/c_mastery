#include "handoff.h"

int xi_wide_add(struct xi_wide *p, uint32_t n)
{
    if (!p)
        return XI_INVALID;
    uint32_t mask = xi_primask_enter(), low = p->low + n;
    unsigned carry = low < p->low;
    int rc = carry && p->high == UINT32_MAX ? XI_EXHAUSTED : XI_OK;
    if (!rc)
    {
        p->low = low;
        p->high += carry;
    }
    xi_primask_leave(mask);
    return rc;
}

struct xi_wide xi_wide_read(const struct xi_wide *p)
{
    uint32_t mask = xi_primask_enter();
    struct xi_wide value = *p;
    xi_primask_leave(mask);
    return value;
}
