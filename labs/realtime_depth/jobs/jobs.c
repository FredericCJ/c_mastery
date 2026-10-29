#include "jobs.h"

/* Unsolved learner implementation. */
enum sj_status sj_order(const struct sj_task *t, unsigned n, unsigned d,
                        unsigned *o)
{
    (void)t;
    (void)n;
    (void)d;
    (void)o;
    return SJ_UNIMPLEMENTED;
}

enum sj_status sj_generate(const struct sj_task *t, unsigned n,
                           const uint32_t *p, const uint32_t *j, unsigned k,
                           struct sj_job *o, unsigned c, unsigned *z)
{
    (void)t;
    (void)n;
    (void)p;
    (void)j;
    (void)k;
    (void)o;
    (void)c;
    (void)z;
    return SJ_UNIMPLEMENTED;
}

enum sj_status sj_simulate(const struct sj_task *t, unsigned n,
                           const struct sj_job *j, unsigned k, enum sj_policy p,
                           enum sj_overload v, unsigned c, unsigned h,
                           unsigned x, struct sj_result *o)
{
    (void)t;
    (void)n;
    (void)j;
    (void)k;
    (void)p;
    (void)v;
    (void)c;
    (void)h;
    (void)x;
    (void)o;
    return SJ_UNIMPLEMENTED;
}

enum sj_status sj_rm_bound(const struct sj_task *t, unsigned n, unsigned *o)
{
    (void)t;
    (void)n;
    (void)o;
    return SJ_UNIMPLEMENTED;
}
