#include "writer.h"

int cwf_byte_step(algo_ring *a, struct cwf_byte *b, f_writer_fn c, void *d,
                  unsigned e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return -1;
}

int cwf_byte_recover(struct cwf_byte *a)
{
    (void)a;
    return -1;
}

int cwf_span_begin(struct cwf_span *a, const unsigned char *b, size_t c,
                   unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int cwf_span_take(algo_ring *a, struct cwf_span *b, size_t c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

enum cpw_status cwf_span_step(struct cwf_span *a, cpw_write_fn b, void *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CPW_INVALID;
}

enum cpw_status cwf_span_recover(struct cwf_span *a)
{
    (void)a;
    return CPW_INVALID;
}

int cwf_error(const struct cpw_transfer *a, struct cwf_error *b)
{
    (void)a;
    (void)b;
    return -1;
}
