#include "rwlock.h"

/* Intentionally unsolved learner adapter. */
int cdr_init(struct cd_rw *o, cmx_kernel *k, unsigned n)
{
    (void)o;
    (void)k;
    (void)n;
    return CMX_UNIMPLEMENTED;
}

int cdr_register(struct cd_rw *o, unsigned t, struct cdr_handle *h)
{
    (void)o;
    (void)t;
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdr_begin(struct cdr_handle h, unsigned m, cmx_deadline d,
              struct cdr_ticket *t)
{
    (void)h;
    (void)m;
    (void)d;
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cdr_result(struct cdr_ticket t)
{
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cdr_cancel(struct cdr_ticket t)
{
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cdr_unlock(struct cdr_handle h)
{
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdr_close(struct cd_rw *o)
{
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cdr_release(struct cdr_handle *h)
{
    (void)h;
    return CMX_UNIMPLEMENTED;
}

int cdr_destroy(struct cd_rw *o)
{
    (void)o;
    return CMX_UNIMPLEMENTED;
}
