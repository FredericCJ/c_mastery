#include "epoch.h"

int ke_enter(struct k_epoch *s, unsigned i)
{
    (void)s;
    (void)i;
    return 0;
}

int ke_leave(struct k_epoch *s, unsigned i)
{
    (void)s;
    (void)i;
    return 0;
}

int ke_replace(struct k_epoch *s)
{
    (void)s;
    return 0;
}

int ke_reclaim(struct k_epoch *s)
{
    (void)s;
    return 0;
}

int ke_close(struct k_epoch *s)
{
    (void)s;
    return 0;
}

int ke_requeue(const struct k_epoch *s, uint32_t g)
{
    (void)s;
    (void)g;
    return 0;
}
