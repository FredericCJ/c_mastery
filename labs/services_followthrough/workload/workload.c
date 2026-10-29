#include "workload.h"

void sfw_init(struct sf_work *s)
{
    (void)s;
}

int sfw_offer(struct sf_work *s, unsigned n)
{
    (void)s;
    (void)n;
    return AF_UNIMPLEMENTED;
}

int sfw_work(struct sf_work *s)
{
    (void)s;
    return AF_UNIMPLEMENTED;
}

int sfw_quiescent(struct sf_work *s, unsigned p, unsigned d)
{
    (void)s;
    (void)p;
    (void)d;
    return AF_UNIMPLEMENTED;
}

int sfw_check(const struct sf_work *s)
{
    (void)s;
    return AF_UNIMPLEMENTED;
}

int sfw_run(struct sf_work *s, unsigned n)
{
    (void)s;
    (void)n;
    return AF_UNIMPLEMENTED;
}
