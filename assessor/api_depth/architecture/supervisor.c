#include "arch.h"
#include <limits.h>
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_supervise(struct ar_supervisor *s, unsigned event,
                 int (*log)(void *, unsigned), void *ctx)
{
    if (!s || !log || event > 2)
    {
        return AR_INVALID;
    }
    if (s->mode == 2)
    {
        return AR_STATE;
    }
    if (event == 0)
    {
        s->streak = 0;
        s->mode = 0;
    }
    else if (event == 1)
    {
        if (s->streak < 2)
        {
            ++s->streak;
        }
        if (s->streak == 2)
        {
            s->mode = 1;
        }
    }
    else
    {
        s->mode = 2;
        s->streak = 0;
    }
    if (log(ctx, event) != AR_OK && s->lost_logs < UINT_MAX)
    {
        ++s->lost_logs;
    }
    if (AR_MUTANT == 6 && s->lost_logs)
    {
        s->mode = 0;
    }
    return AR_OK;
}

int ar_recover(struct ar_supervisor *s, struct ar_recovery r)
{
    if (!s || !r.init_a || !r.init_b || !r.drop_a)
    {
        return AR_INVALID;
    }
    if (s->attempts == 2 || s->mode == 0)
    {
        return AR_STATE;
    }
    ++s->attempts;
    if (r.init_a(r.context) != AR_OK)
    {
        return AR_FAILED;
    }
    if (r.init_b(r.context) != AR_OK)
    {
        r.drop_a(r.context);
        return AR_FAILED;
    }
    s->mode = s->streak = 0;
    return AR_OK;
}
