#include "limbs.h"

enum bl_status bl_add(struct bl_view a, struct bl_view b, struct bl_output *o,
                      uint16_t *s, size_t n)
{
    (void)a;
    (void)b;
    (void)o;
    (void)s;
    (void)n;
    return BL_INVALID;
}

enum bl_status bl_sub(struct bl_view a, struct bl_view b, struct bl_output *o,
                      uint16_t *s, size_t n)
{
    (void)a;
    (void)b;
    (void)o;
    (void)s;
    (void)n;
    return BL_INVALID;
}

enum bl_status bl_mul(struct bl_view a, struct bl_view b, struct bl_output *o,
                      uint16_t *s, size_t n)
{
    (void)a;
    (void)b;
    (void)o;
    (void)s;
    (void)n;
    return BL_INVALID;
}

enum bl_status bl_div(struct bl_view a, struct bl_view b, struct bl_output *q,
                      struct bl_output *r, uint16_t *s, size_t n)
{
    (void)a;
    (void)b;
    (void)q;
    (void)r;
    (void)s;
    (void)n;
    return BL_INVALID;
}

enum bl_status bl_signed_add(struct bl_signed a, struct bl_signed b,
                             struct bl_signed_output *o, uint16_t *s, size_t n)
{
    (void)a;
    (void)b;
    (void)o;
    (void)s;
    (void)n;
    return BL_INVALID;
}

enum bl_status bl_signed_div(struct bl_signed a, struct bl_signed b,
                             struct bl_signed_output *q,
                             struct bl_signed_output *r, uint16_t *s, size_t n)
{
    (void)a;
    (void)b;
    (void)q;
    (void)r;
    (void)s;
    (void)n;
    return BL_INVALID;
}

enum bl_status bl_owner_init(struct bl_owner *o, void *c, bl_allocate_fn a,
                             bl_free_fn f)
{
    (void)o;
    (void)c;
    (void)a;
    (void)f;
    return BL_INVALID;
}

enum bl_status bl_owner_set(struct bl_owner *o, struct bl_view v, size_t n)
{
    (void)o;
    (void)v;
    (void)n;
    return BL_INVALID;
}

void bl_owner_drop(struct bl_owner *o)
{
    (void)o;
}
