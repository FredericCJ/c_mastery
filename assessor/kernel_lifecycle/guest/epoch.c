#include "epoch.h"

int ke_enter(struct k_epoch *s, unsigned i)
{
    if (i >= 8 || s->closed || s->reader[i])
    {
        return 0;
    }
    s->reader[i] = s->current;
    return 1;
}

int ke_leave(struct k_epoch *s, unsigned i)
{
    if (i >= 8 || !s->reader[i])
    {
        return 0;
    }
    s->reader[i] = 0;
    return 1;
}

int ke_replace(struct k_epoch *s)
{
    if (s->closed || s->retired || s->current == UINT32_MAX)
    {
        return 0;
    }
    s->retired = s->current++;
    return 1;
}

int ke_reclaim(struct k_epoch *s)
{
    if (!s->retired)
    {
        return 0;
    }
    for (unsigned i = 0; i < 8; i++)
    {
#ifdef KE_WAIT_ALL
        if (s->reader[i])
        {
            return 0;
        }
#else
        if (s->reader[i] == s->retired)
        {
            return 0;
        }
#endif
    }
    s->retired = 0;
    return 1;
}

int ke_close(struct k_epoch *s)
{
    if (s->closed)
    {
        return 1;
    }
    if (s->current == UINT32_MAX)
    {
        return 0;
    }
    s->closed = 1;
    ++s->current;
    return 1;
}

int ke_requeue(const struct k_epoch *s, uint32_t g)
{
#ifdef KE_STALE
    (void)g;
    return !s->closed;
#else
    return !s->closed && g == s->current;
#endif
}
