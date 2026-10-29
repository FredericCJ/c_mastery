#include "secrets.h"

int ss_compare(const unsigned char t[4], const unsigned char *c, size_t n,
               int e, struct ss_compare *o)
{
    (void)t;
    (void)c;
    (void)n;
    (void)e;
    (void)o;
    return -1;
}

int ss_indexed(const unsigned char t[4], const unsigned char c[4],
               volatile const unsigned char x[256], unsigned a[8])
{
    (void)t;
    (void)c;
    (void)x;
    (void)a;
    return -1;
}

int ss_acquire(struct ss_pool *p, const unsigned char t[4])
{
    (void)p;
    (void)t;
    return -1;
}

int ss_release(struct ss_pool *p)
{
    (void)p;
    return -1;
}
