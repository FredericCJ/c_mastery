#include "variants.h"

/* Deliberately unsolved: implement the named delayed variants. */
void sv_init(struct sv_stream *s)
{
    (void)s;
}

int sv_start(struct sv_stream *s, const unsigned char *b, size_t n,
             struct sv_handle *h)
{
    (void)s;
    (void)b;
    (void)n;
    (void)h;
    return SF_UNIMPLEMENTED;
}

int sv_pump(struct sv_handle h, sv_write_fn w, void *c, unsigned b, size_t *n)
{
    (void)h;
    (void)w;
    (void)c;
    (void)b;
    (void)n;
    return SF_UNIMPLEMENTED;
}

int sv_pumpv(struct sv_handle h, sv_write_v_fn w, void *c, unsigned b,
             size_t *n)
{
    (void)h;
    (void)w;
    (void)c;
    (void)b;
    (void)n;
    return SF_UNIMPLEMENTED;
}

int sv_cancel(struct sv_handle h)
{
    (void)h;
    return SF_UNIMPLEMENTED;
}

void svq_init(struct sv_queue *q)
{
    (void)q;
}

int svq_submit(struct sv_queue *q, const unsigned char *b, size_t n,
               struct sv_handle *h)
{
    (void)q;
    (void)b;
    (void)n;
    (void)h;
    return SF_UNIMPLEMENTED;
}

int svq_pump(struct sv_queue *q, sv_write_fn w, void *c, unsigned b,
             struct sv_event *e)
{
    (void)q;
    (void)w;
    (void)c;
    (void)b;
    (void)e;
    return SF_UNIMPLEMENTED;
}

int svq_cancel(struct sv_queue *q, struct sv_handle h, struct sv_event *e)
{
    (void)q;
    (void)h;
    (void)e;
    return SF_UNIMPLEMENTED;
}

int svq_reset_transport(struct sv_queue *q, unsigned v)
{
    (void)q;
    (void)v;
    return SF_UNIMPLEMENTED;
}
