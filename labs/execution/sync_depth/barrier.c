#include "barrier.h"

/* Intentionally incomplete learner scaffold. */
int cdb_init(struct cd_barrier *b, cmx_kernel *k, uint32_t m, unsigned n)
{
    (void)b;
    (void)k;
    (void)m;
    (void)n;
    return CMX_UNIMPLEMENTED;
}

int cdb_register(struct cd_barrier *b, unsigned t, struct cdb_member *o)
{
    (void)b;
    (void)t;
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdb_arrive(struct cdb_member h, cmx_deadline d, struct cdb_ticket *o)
{
    (void)h;
    (void)d;
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdb_result(struct cdb_ticket t, unsigned *o)
{
    (void)t;
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdb_cancel(struct cdb_ticket t)
{
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cdb_stage(struct cd_barrier *b, uint32_t m)
{
    (void)b;
    (void)m;
    return CMX_UNIMPLEMENTED;
}

int cdb_release(struct cdb_member *h)
{
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdb_apply(struct cd_barrier *b)
{
    (void)b;
    return CMX_UNIMPLEMENTED;
}

int cdb_close(struct cd_barrier *b)
{
    (void)b;
    return CMX_UNIMPLEMENTED;
}

int cdb_destroy(struct cd_barrier *b)
{
    (void)b;
    return CMX_UNIMPLEMENTED;
}
