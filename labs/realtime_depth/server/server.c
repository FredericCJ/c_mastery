#include "server.h"

/* Learner scaffold: replace each refusal with the specified mechanism. */
int ss_invariant(const struct ss_budget *s)
{
    (void)s;
    return 0;
}

enum cmx_result ss_init(struct ss_budget *s, uint32_t q, uint32_t p, uint32_t t,
                        unsigned n, enum ss_merge m)
{
    (void)s;
    (void)q;
    (void)p;
    (void)t;
    (void)n;
    (void)m;
    return CMX_INVALID;
}

enum cmx_result ss_update(struct ss_budget *s, uint32_t t)
{
    (void)s;
    (void)t;
    return CMX_INVALID;
}

enum cmx_result ss_charge(struct ss_budget *s, uint32_t t, uint32_t n)
{
    (void)s;
    (void)t;
    (void)n;
    return CMX_INVALID;
}

enum cmx_result ss_compare(uint32_t q, uint32_t p, uint32_t b, uint32_t e,
                           const uint32_t *v, unsigned n,
                           struct ss_comparison *o)
{
    (void)q;
    (void)p;
    (void)b;
    (void)e;
    (void)v;
    (void)n;
    (void)o;
    return CMX_INVALID;
}

enum cmx_result ss_workload(const struct ss_workload *w, enum ss_policy p,
                            struct ss_trace *o)
{
    (void)w;
    (void)p;
    (void)o;
    return CMX_INVALID;
}
