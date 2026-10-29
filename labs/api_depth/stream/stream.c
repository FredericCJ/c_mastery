#include "stream.h"

void sf_init(struct sf_stream *s)
{
    if (s)
        *s = (struct sf_stream){0};
}

int sf_start(struct sf_stream *a, const unsigned char *b, size_t c,
             struct sf_handle *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return SF_UNIMPLEMENTED;
}

int sf_pump(struct sf_handle a, sf_write_fn b, void *c, unsigned d, size_t *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return SF_UNIMPLEMENTED;
}

int sf_cancel(struct sf_handle a)
{
    (void)a;
    return SF_UNIMPLEMENTED;
}
