#include "checkpoint_vector.h"

int cpv_detect(struct cpv_probe p)
{
    (void)p;
    return 0;
}

int cpv_available(void)
{
    return 0;
}

enum cpn_status cpv_plan(struct cpv_plan *p, enum cpv_backend b)
{
    (void)p;
    (void)b;
    return CPN_INVALID;
}

enum cpn_status cpv_run(const struct cpv_plan *p, struct cpn_double *s,
                        const double *i, size_t n, double *o, size_t c)
{
    (void)p;
    (void)s;
    (void)i;
    (void)n;
    (void)o;
    (void)c;
    return CPN_INVALID;
}
