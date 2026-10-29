#include "shared.h"

int mc_shared_init(struct mc_shared *a, struct mc_allocator b)
{
    (void)a;
    (void)b;
    return -1;
}

int mc_shared_retain(struct mc_shared *a, struct mc_shared **b)
{
    (void)a;
    (void)b;
    return -1;
}

int mc_shared_add(struct mc_shared *a, unsigned b)
{
    (void)a;
    (void)b;
    return -1;
}

int mc_shared_read(struct mc_shared *a, unsigned *b)
{
    (void)a;
    (void)b;
    return -1;
}

int mc_shared_drop(struct mc_shared **a)
{
    (void)a;
    return -1;
}

int mc_shared_finish(struct mc_shared *a)
{
    (void)a;
    return -1;
}
