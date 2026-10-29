#include "wait.h"

/* Deliberately unsolved wrapper: complete the generational CMX wait contract.
 */
void cw_init(struct cw_world *w, cmx_kernel *k)
{
    (void)w;
    (void)k;
}

int cw_object_init(struct cw_world *w, unsigned i, unsigned t, unsigned n,
                   unsigned m)
{
    (void)w;
    (void)i;
    (void)t;
    (void)n;
    (void)m;
    return CMX_UNIMPLEMENTED;
}

int cw_begin(struct cw_world *w, unsigned i, unsigned t, cmx_deadline d,
             unsigned s, struct cw_token *o)
{
    (void)w;
    (void)i;
    (void)t;
    (void)d;
    (void)s;
    (void)o;
    return CMX_UNIMPLEMENTED;
}

int cw_result(struct cw_token t, int *r, unsigned *c)
{
    (void)t;
    (void)r;
    (void)c;
    return CMX_UNIMPLEMENTED;
}

int cw_post(struct cw_world *w, unsigned i)
{
    (void)w;
    (void)i;
    return CMX_UNIMPLEMENTED;
}

int cw_ready_callback(struct cw_token t)
{
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cw_timeout(struct cw_token t, uint32_t n)
{
    (void)t;
    (void)n;
    return CMX_UNIMPLEMENTED;
}

int cw_cancel(struct cw_token t)
{
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cw_unlock(struct cw_world *w, unsigned i, unsigned t)
{
    (void)w;
    (void)i;
    (void)t;
    return CMX_UNIMPLEMENTED;
}

int cw_close(struct cw_world *w, unsigned i)
{
    (void)w;
    (void)i;
    return CMX_UNIMPLEMENTED;
}

int cw_exit_reap(struct cw_world *w, unsigned t)
{
    (void)w;
    (void)t;
    return CMX_UNIMPLEMENTED;
}
