#include "once.h"

/* Intentionally unsolved learner adapter. */
int cdo_init(struct cd_once *o, cmx_kernel *k, cdo_allocate_fn a, cdo_drop_fn d,
             void *c)
{
    (void)o;
    (void)k;
    (void)a;
    (void)d;
    (void)c;
    return CMX_UNIMPLEMENTED;
}

int cdo_register(struct cd_once *o, unsigned t, struct cdo_handle *h)
{
    (void)o;
    (void)t;
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdo_enter(struct cdo_handle h, cmx_deadline d, struct cdo_ticket *t)
{
    (void)h;
    (void)d;
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cdo_allocate(struct cdo_handle h)
{
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdo_write(struct cdo_handle h, unsigned i, uint32_t v)
{
    (void)h;
    (void)i;
    (void)v;
    return CMX_UNIMPLEMENTED;
}

int cdo_seal(struct cdo_handle h)
{
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdo_finish(struct cdo_handle h, unsigned s)
{
    (void)h;
    (void)s;
    return CMX_UNIMPLEMENTED;
}

int cdo_result(struct cdo_ticket t, struct cdo_table *o)
{
    (void)t;
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdo_read(struct cdo_handle h, struct cdo_table *o)
{
    (void)h;
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdo_cancel(struct cdo_ticket t)
{
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cdo_prepare_reset(struct cd_once *o)
{
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdo_release(struct cdo_handle *h)
{
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdo_reset(struct cd_once *o)
{
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdo_close(struct cd_once *o)
{
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdo_destroy(struct cd_once *o)
{
    (void)o;
    return CMX_UNIMPLEMENTED;
}
