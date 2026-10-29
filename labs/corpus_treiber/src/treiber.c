#include "treiber.h"

/* Implement all operations from the public contract; no hidden reference import. */
enum ts_status ts_init(struct ts_stack *s, unsigned n)
{
    (void)s;
    (void)n;
    return TS_UNSUPPORTED;
}

enum ts_status ts_push(struct ts_stack *s, uint32_t n)
{
    (void)s;
    (void)n;
    return TS_UNSUPPORTED;
}

enum ts_status ts_begin(struct ts_stack *s, unsigned p, struct ts_pop *v)
{
    (void)s;
    (void)p;
    (void)v;
    return TS_UNSUPPORTED;
}

enum ts_status ts_commit(struct ts_pop *v, uint32_t *out)
{
    (void)v;
    (void)out;
    return TS_UNSUPPORTED;
}

enum ts_status ts_cancel(struct ts_pop *v)
{
    (void)v;
    return TS_UNSUPPORTED;
}

enum ts_status ts_pop(struct ts_stack *s, unsigned p, uint32_t *out)
{
    (void)s;
    (void)p;
    (void)out;
    return TS_UNSUPPORTED;
}

enum ts_status ts_scan(struct ts_stack *s, unsigned n, struct ts_retired *r)
{
    (void)s;
    (void)n;
    (void)r;
    return TS_UNSUPPORTED;
}

enum ts_status ts_reclaim(struct ts_retired *r)
{
    (void)r;
    return TS_UNSUPPORTED;
}

unsigned ts_collect(struct ts_stack *s)
{
    (void)s;
    return 0;
}
