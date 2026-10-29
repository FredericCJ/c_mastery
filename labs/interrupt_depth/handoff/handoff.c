#include "handoff.h"

/* Intentionally unfinished learner implementation. */
int xi_init(struct xi_queue *q, unsigned n)
{
    (void)q;
    (void)n;
    return XI_UNIMPLEMENTED;
}

int xi_reserve(struct xi_queue *q)
{
    (void)q;
    return XI_UNIMPLEMENTED;
}

int xi_fill(struct xi_queue *q, const struct xi_record *r)
{
    (void)q;
    (void)r;
    return XI_UNIMPLEMENTED;
}

int xi_commit(struct xi_queue *q)
{
    (void)q;
    return XI_UNIMPLEMENTED;
}

int xi_cancel(struct xi_queue *q)
{
    (void)q;
    return XI_UNIMPLEMENTED;
}

int xi_take(struct xi_queue *q, struct xi_record *r)
{
    (void)q;
    (void)r;
    return XI_UNIMPLEMENTED;
}

int xi_release(struct xi_queue *q)
{
    (void)q;
    return XI_UNIMPLEMENTED;
}

int xi_push(struct xi_queue *q, const struct xi_record *r)
{
    (void)q;
    (void)r;
    return XI_UNIMPLEMENTED;
}

int xi_pop(struct xi_queue *q, struct xi_record *r)
{
    (void)q;
    (void)r;
    return XI_UNIMPLEMENTED;
}

int xi_service_init(struct xi_service *s, struct xi_queue *q)
{
    (void)s;
    (void)q;
    return XI_UNIMPLEMENTED;
}

int xi_capture_begin(struct xi_service *s, uint32_t a, uint32_t b,
                     const unsigned char *p, uint32_t n)
{
    (void)s;
    (void)a;
    (void)b;
    (void)p;
    (void)n;
    return XI_UNIMPLEMENTED;
}

int xi_capture_end(struct xi_service *s)
{
    (void)s;
    return XI_UNIMPLEMENTED;
}

void xi_latest_init(struct xi_latest *q)
{
    (void)q;
}

int xi_latest_push(struct xi_latest *q, const struct xi_record *r)
{
    (void)q;
    (void)r;
    return XI_UNIMPLEMENTED;
}

int xi_latest_pop(struct xi_latest *q, struct xi_record *r)
{
    (void)q;
    (void)r;
    return XI_UNIMPLEMENTED;
}

int xi_mask_init(struct xi_mask *s, struct xf_priority_config c, unsigned a,
                 unsigned b)
{
    (void)s;
    (void)c;
    (void)a;
    (void)b;
    return XI_UNIMPLEMENTED;
}

int xi_mask_enter(struct xi_mask *s, unsigned n, uint32_t *t)
{
    (void)s;
    (void)n;
    (void)t;
    return XI_UNIMPLEMENTED;
}

int xi_mask_leave(struct xi_mask *s, uint32_t t)
{
    (void)s;
    (void)t;
    return XI_UNIMPLEMENTED;
}
