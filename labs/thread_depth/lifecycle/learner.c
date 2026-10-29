#include "lifecycle.h"
#include <errno.h>

struct tl_pool *tl_pool_start(unsigned n, unsigned f, int a, int *e)
{
    (void)n;
    (void)f;
    (void)a;
    if (e)
    {
        *e = ENOSYS;
    }
    return 0;
}

int tl_pool_collect(struct tl_pool *p, tl_join_fn j, void *c,
                    struct tl_stats *o)
{
    (void)p;
    (void)j;
    (void)c;
    (void)o;
    return ENOSYS;
}

int tl_pool_dispose(struct tl_pool **p)
{
    (void)p;
    return ENOSYS;
}

int tl_monitor_init(struct tl_monitor *m)
{
    (void)m;
    return ENOSYS;
}

int tl_monitor_destroy(struct tl_monitor *m)
{
    (void)m;
    return ENOSYS;
}

void *tl_detached_creator(void *a)
{
    (void)a;
    return 0;
}
