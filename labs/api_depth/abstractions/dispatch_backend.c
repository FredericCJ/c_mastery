#include "dispatch.h"

/* Implement the three supplied quota contracts without importing an I/O library. */
ptrdiff_t ad_one(struct ad_sink *s, const unsigned char *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
    return -1;
}

ptrdiff_t ad_four(struct ad_sink *s, const unsigned char *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
    return -1;
}

ptrdiff_t ad_all(struct ad_sink *s, const unsigned char *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
    return -1;
}

const struct ad_ops *ad_operations(unsigned k)
{
    (void)k;
    return 0;
}
