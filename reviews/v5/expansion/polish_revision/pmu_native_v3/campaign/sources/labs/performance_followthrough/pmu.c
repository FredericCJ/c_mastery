/* Learner scaffold. Implement PMU_PROTOCOL.md; OS bindings are supplied. */
#include "pmu.h"

enum pf_pmu_state pf_pmu_open(struct pf_pmu *p, struct pf_pmu_os os,
                              const struct pf_pmu_event *e, size_t n)
{
    (void)os;
    (void)e;
    (void)n;
    if (p)
    {
        *p = (struct pf_pmu){.state = PF_PMU_UNIMPLEMENTED};
    }
    return PF_PMU_UNIMPLEMENTED;
}

enum pf_pmu_state pf_pmu_start(struct pf_pmu *p)
{
    (void)p;
    return PF_PMU_UNIMPLEMENTED;
}

enum pf_pmu_state pf_pmu_stop(struct pf_pmu *p)
{
    (void)p;
    return PF_PMU_UNIMPLEMENTED;
}

void pf_pmu_close(struct pf_pmu *p)
{
    (void)p;
}

int pf_ring_drain(const unsigned char *r, size_t n, uint64_t h, uint64_t *t,
                  size_t b, struct pf_ring_stats *s,
                  void (*f)(void *, const struct pf_sample *), void *c)
{
    (void)r;
    (void)n;
    (void)h;
    (void)t;
    (void)b;
    (void)s;
    (void)f;
    (void)c;
    return PF_RING_UNIMPLEMENTED;
}
