#include "rational.h"

uint32_t efr_gcd(uint32_t a, uint32_t b, unsigned *s)
{
    (void)a;
    (void)b;
    (void)s;
    return 0;
}

er_code efr_consume(er_fraction a, uint32_t *q)
{
    (void)a;
    (void)q;
    return ER_NOT_IMPLEMENTED;
}

er_code efr_add_naive(er_fraction a, er_fraction b, er_fraction *o)
{
    (void)a;
    (void)b;
    (void)o;
    return ER_NOT_IMPLEMENTED;
}

er_code efr_add(er_fraction a, er_fraction b, er_fraction *o)
{
    (void)a;
    (void)b;
    (void)o;
    return ER_NOT_IMPLEMENTED;
}

er_code efr_signed_make(int32_t n, uint32_t d, struct efr_signed *o)
{
    (void)n;
    (void)d;
    (void)o;
    return ER_NOT_IMPLEMENTED;
}

er_code efr_signed_add(struct efr_signed a, struct efr_signed b,
                       struct efr_signed *o)
{
    (void)a;
    (void)b;
    (void)o;
    return ER_NOT_IMPLEMENTED;
}

er_code efr_signed_consume(struct efr_signed a, int32_t *q)
{
    (void)a;
    (void)q;
    return ER_NOT_IMPLEMENTED;
}
