#include "variant.h"

int nv_sum(const unsigned char *p, size_t n, struct nv_sum *o)
{
    (void)p;
    (void)n;
    (void)o;
    return NV_UNIMPLEMENTED;
}

int nv_combine(struct nv_sum a, struct nv_sum b, struct nv_sum *o)
{
    (void)a;
    (void)b;
    (void)o;
    return NV_UNIMPLEMENTED;
}

int nv_length(const unsigned char *p, size_t n, uint32_t m, uint32_t c,
              uint32_t *o)
{
    (void)p;
    (void)n;
    (void)m;
    (void)c;
    (void)o;
    return NV_UNIMPLEMENTED;
}

int nv_date(unsigned a, uint32_t d, uint32_t s, char *o, size_t c, size_t *n)
{
    (void)a;
    (void)d;
    (void)s;
    (void)o;
    (void)c;
    (void)n;
    return NV_UNIMPLEMENTED;
}

int nv_stamp(struct nv_stamp s, uint32_t d, char *o, size_t c, size_t *n)
{
    (void)s;
    (void)d;
    (void)o;
    (void)c;
    (void)n;
    return NV_UNIMPLEMENTED;
}
