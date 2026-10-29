#include "events.h"

/* Deliberately incomplete learner scaffold. Implement EVENTS.md. */
int cde_init(struct cd_events *e, cmx_kernel *k)
{
    (void)e;
    (void)k;
    return CMX_INVALID;
}

int cde_wait(struct cd_events *e, unsigned t, uint32_t m, unsigned a,
             unsigned c, cmx_deadline d)
{
    (void)e;
    (void)t;
    (void)m;
    (void)a;
    (void)c;
    (void)d;
    return CMX_INVALID;
}

int cde_result(struct cd_events *e, unsigned t, uint32_t *o)
{
    (void)e;
    (void)t;
    (void)o;
    return CMX_INVALID;
}

int cde_set(struct cd_events *e, uint32_t b)
{
    (void)e;
    (void)b;
    return CMX_INVALID;
}

int cde_clear(struct cd_events *e, uint32_t b)
{
    (void)e;
    (void)b;
    return CMX_INVALID;
}

int cde_close(struct cd_events *e)
{
    (void)e;
    return CMX_INVALID;
}

int cde_destroy(struct cd_events *e)
{
    (void)e;
    return CMX_INVALID;
}

void cde_sequence_init(struct cde_sequence *s)
{
    (void)s;
}

int cde_sequence_publish(struct cde_sequence *s)
{
    (void)s;
    return CMX_INVALID;
}

int cde_sequence_read(const struct cde_sequence *s, struct cde_cursor *c,
                      uint64_t *d, uint64_t *m)
{
    (void)s;
    (void)c;
    (void)d;
    (void)m;
    return CMX_INVALID;
}
