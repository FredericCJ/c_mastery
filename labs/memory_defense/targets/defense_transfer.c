#include "defense_transfer.h"

int dt_extent(uint64_t c, uint64_t w, uint64_t h, uint64_t a, uint64_t t,
              uint64_t l, uint64_t *o)
{
    (void)c;
    (void)w;
    (void)h;
    (void)a;
    (void)t;
    (void)l;
    (void)o;
    return 0;
}

int dt_read(const struct dt_owner *o, struct dt_view v, size_t i,
            unsigned char *r)
{
    (void)o;
    (void)v;
    (void)i;
    (void)r;
    return 0;
}

int dt_equal(const unsigned char *a, size_t n, const unsigned char *b, size_t m,
             size_t *s)
{
    (void)a;
    (void)n;
    (void)b;
    (void)m;
    (void)s;
    return -1;
}
