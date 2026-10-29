#include "owning_option.h"

int eop_scalar_make(uint32_t v, unsigned p, struct eop_scalar *o)
{
    (void)v;
    (void)p;
    (void)o;
    return EOP_INVALID;
}

int eop_scalar_get(const struct eop_scalar *o, uint32_t *v)
{
    (void)o;
    (void)v;
    return EOP_INVALID;
}

int eop_result_get(er_result r, uint32_t *v, size_t *e)
{
    (void)r;
    (void)v;
    (void)e;
    return EOP_INVALID;
}

int eop_create(struct eop_owner *o, struct v4_allocator a, uint32_t i,
               int32_t v)
{
    (void)o;
    (void)a;
    (void)i;
    (void)v;
    return EOP_INVALID;
}

int eop_clone(const struct eop_owner *s, struct eop_owner *d)
{
    (void)s;
    (void)d;
    return EOP_INVALID;
}

int eop_move(struct eop_owner *d, struct eop_owner *s)
{
    (void)d;
    (void)s;
    return EOP_INVALID;
}

int eop_read(const struct eop_owner *s, uint32_t *i, int32_t *v)
{
    (void)s;
    (void)i;
    (void)v;
    return EOP_INVALID;
}

int eop_drop(struct eop_owner *o)
{
    (void)o;
    return EOP_INVALID;
}

int eop_unit(enum eop_unit u)
{
    (void)u;
    return EOP_INVALID;
}
