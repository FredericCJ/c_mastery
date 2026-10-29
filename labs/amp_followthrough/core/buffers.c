#include "buffers.h"

int af_buffers_init(struct af_buffers *a, const struct xd_fence *b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_fill(struct af_buffers *a, unsigned b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_submit(struct af_buffers *a, struct pd_io b, unsigned c,
              const struct pd_extent *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AF_UNIMPLEMENTED;
}

int af_timeout(struct af_buffers *a, struct pd_io b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_complete(struct af_buffers *a, struct pd_io b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_consume(struct af_buffers *a, unsigned b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}

int af_release(struct af_buffers *a, unsigned b)
{
    (void)a;
    (void)b;
    return AF_UNIMPLEMENTED;
}
