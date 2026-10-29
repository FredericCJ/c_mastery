#include "mutable_views.h"

enum adv_status adv_geometry(size_t a, size_t b, size_t c, size_t d, size_t e,
                             struct adv_geometry *o)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)o;
    return ADV_INVALID;
}

enum adv_status adv_view(uint32_t *p, size_t a, size_t b, size_t c, size_t d,
                         size_t e, struct adv_view *o)
{
    (void)p;
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)o;
    return ADV_INVALID;
}

struct ad_view adv_readonly(struct adv_view v)
{
    (void)v;
    return (struct ad_view){0};
}

enum adv_status adv_set(struct adv_view v, size_t r, size_t c, uint32_t x)
{
    (void)v;
    (void)r;
    (void)c;
    (void)x;
    return ADV_INVALID;
}

enum adv_status adv_transpose(struct adv_view v, struct adv_view *o)
{
    (void)v;
    (void)o;
    return ADV_INVALID;
}

enum adv_status adv_subview(struct adv_view v, size_t r, size_t c, size_t n,
                            size_t m, struct adv_view *o)
{
    (void)v;
    (void)r;
    (void)c;
    (void)n;
    (void)m;
    (void)o;
    return ADV_INVALID;
}

enum adv_status adv_reduce(struct ad_view v, enum adv_mode m,
                           struct adv_reduction *o)
{
    (void)v;
    (void)m;
    (void)o;
    return ADV_INVALID;
}
