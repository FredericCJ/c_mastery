#include "bridge.h"

/* Intentionally unfinished learner adapter. */
int p5_bridge_init(struct p5_bridge *p, int fd, int h)
{
    (void)p;
    (void)fd;
    (void)h;
    return -1;
}

int p5_bridge_offer(struct p5_bridge *p, int32_t v)
{
    (void)p;
    (void)v;
    return -1;
}

int p5_bridge_begin(struct p5_bridge *p, uint32_t *t)
{
    (void)p;
    (void)t;
    return -1;
}

int p5_bridge_cancel(struct p5_bridge *p, uint32_t t)
{
    (void)p;
    (void)t;
    return -1;
}

int p5_bridge_observe(struct p5_bridge *p, uint32_t t)
{
    (void)p;
    (void)t;
    return -1;
}

int p5_bridge_wait(struct p5_bridge *p, const struct timespec *t)
{
    (void)p;
    (void)t;
    return -1;
}

int p5_bridge_release_gate(struct p5_bridge *p)
{
    (void)p;
    return -1;
}

int p5_bridge_reap(struct p5_bridge *p)
{
    (void)p;
    return -1;
}

int p5_bridge_destroy(struct p5_bridge *p)
{
    (void)p;
    return -1;
}
