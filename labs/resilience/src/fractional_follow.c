#include "fractional.h"

int rzf_bucket_init(struct rzf_bucket *b, uint64_t c, uint64_t p, uint64_t q,
                    uint64_t n)
{
    (void)b;
    (void)c;
    (void)p;
    (void)q;
    (void)n;
    return RZ_INVALID;
}

int rzf_bucket_consume(struct rzf_bucket *b, uint64_t n, uint64_t c)
{
    (void)b;
    (void)n;
    (void)c;
    return RZ_INVALID;
}

int rzf_bucket_pair(struct rzf_bucket *a, struct rzf_bucket *p, uint64_t n,
                    uint64_t c)
{
    (void)a;
    (void)p;
    (void)n;
    (void)c;
    return RZ_INVALID;
}

int rzf_credit_export(const struct rzf_bucket *b, uint64_t n,
                      struct rzf_credit *o)
{
    (void)b;
    (void)n;
    (void)o;
    return RZ_INVALID;
}

int rzf_credit_import(const struct rzf_credit *c, uint64_t n, uint64_t d,
                      enum rzf_downtime p, struct rzf_bucket *o)
{
    (void)c;
    (void)n;
    (void)d;
    (void)p;
    (void)o;
    return RZ_INVALID;
}
