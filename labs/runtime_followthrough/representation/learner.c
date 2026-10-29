#include "representation.h"

int rv_import(struct cvm_value v, struct rv_value *o)
{
    (void)v;
    (void)o;
    return RV_UNIMPLEMENTED;
}

int rv_equal(struct rv_value a, struct rv_value b, int *o)
{
    (void)a;
    (void)b;
    (void)o;
    return RV_UNIMPLEMENTED;
}

int rv_arithmetic(unsigned p, struct rv_value a, struct rv_value b,
                  struct rv_value *o)
{
    (void)p;
    (void)a;
    (void)b;
    (void)o;
    return RV_UNIMPLEMENTED;
}

int rv_decode(const uint8_t *b, size_t n, struct rv_image *o)
{
    (void)b;
    (void)n;
    (void)o;
    return RV_UNIMPLEMENTED;
}

int rv_encode(const struct rv_image *i, uint8_t *b, size_t n, size_t *o)
{
    (void)i;
    (void)b;
    (void)n;
    (void)o;
    return RV_UNIMPLEMENTED;
}
