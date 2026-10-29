#include "public_error.h"

enum ae_code ae_capture(unsigned b, int c, unsigned o, unsigned s,
                        struct ae_public *r, unsigned char d[8])
{
    (void)b;
    (void)c;
    (void)o;
    (void)s;
    (void)r;
    (void)d;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_decode(const unsigned char *b, size_t n, unsigned *x, int *c,
                       unsigned *o, unsigned *s)
{
    (void)b;
    (void)n;
    (void)x;
    (void)c;
    (void)o;
    (void)s;
    return AE_UNIMPLEMENTED;
}
