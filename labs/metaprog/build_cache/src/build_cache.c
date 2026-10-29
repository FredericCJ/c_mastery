#include "build_cache.h"

int mpb_plan(const struct mpb_node *n, size_t c, unsigned f, struct mpb_plan *o)
{
    (void)n;
    (void)c;
    (void)f;
    (void)o;
    return MPB_INVALID;
}

int mpb_match(const struct mpb_entry *a, const struct mpb_entry *b)
{
    (void)a;
    (void)b;
    return 0;
}

int mpb_encode(const struct mpb_entry *e, unsigned i, unsigned char *b,
               size_t c, size_t *u)
{
    (void)e;
    (void)i;
    (void)b;
    (void)c;
    (void)u;
    return MPB_INVALID;
}

int mpb_begin(struct mpb_schedule *s, const struct mpb_plan *p)
{
    (void)s;
    (void)p;
    return MPB_INVALID;
}

int mpb_claim(struct mpb_schedule *s, struct mpb_ticket *t)
{
    (void)s;
    (void)t;
    return MPB_INVALID;
}

int mpb_finish(struct mpb_schedule *s, struct mpb_ticket t,
               const struct mpb_entry *e)
{
    (void)s;
    (void)t;
    (void)e;
    return MPB_INVALID;
}

int mpb_publish(const struct mpb_schedule *s, struct mpb_plan *o)
{
    (void)s;
    (void)o;
    return MPB_INVALID;
}
