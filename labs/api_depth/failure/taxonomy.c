#include "taxonomy.h"

/* These are intentionally unfinished learner mechanisms. */
enum ae_code ae_map(unsigned b, int c)
{
    (void)b;
    (void)c;
    return AE_UNIMPLEMENTED;
}

enum ae_outcome ae_outcome(enum ae_code c)
{
    (void)c;
    return AE_TERMINAL;
}

enum ae_code ae_first(struct ae_error *e, enum ae_code c, unsigned o,
                      unsigned s, unsigned b, int x)
{
    (void)e;
    (void)c;
    (void)o;
    (void)s;
    (void)b;
    (void)x;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_secondary(struct ae_error *e, unsigned s, int c)
{
    (void)e;
    (void)s;
    (void)c;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_boundary(const unsigned char *p, size_t n, unsigned c,
                         unsigned a, unsigned d, unsigned *o)
{
    (void)p;
    (void)n;
    (void)c;
    (void)a;
    (void)d;
    (void)o;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_admit_digit(const unsigned char *p, size_t n,
                            struct ae_digit *o)
{
    (void)p;
    (void)n;
    (void)o;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_trusted_digit(struct ae_digit d, unsigned *o)
{
    (void)d;
    (void)o;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_submit(struct ae_counter *c, unsigned i, unsigned d,
                       enum ae_transport_mode m)
{
    (void)c;
    (void)i;
    (void)d;
    (void)m;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_write_result(size_t o, size_t a, enum ae_code c,
                             struct ae_write_result *r)
{
    (void)o;
    (void)a;
    (void)c;
    (void)r;
    return AE_UNIMPLEMENTED;
}

struct ae_lookup ae_lookup(unsigned k)
{
    (void)k;
    return (struct ae_lookup){0, 0};
}

enum ae_code ae_fields(const struct av_text *f, size_t n, struct ae_fields *o)
{
    (void)f;
    (void)n;
    (void)o;
    return AE_UNIMPLEMENTED;
}
