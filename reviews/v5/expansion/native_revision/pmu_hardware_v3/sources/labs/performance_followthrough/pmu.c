/* Learner scaffold. Implement PMU_PROTOCOL.md; OS bindings are supplied. */
#include "pmu.h"
PFPmuState pf_pmu_open(PFPmu *p, PFPmuOS os, const PFPmuEvent *e, size_t n) {
    (void)os; (void)e; (void)n;
    if (p) *p = (PFPmu){.state = PF_PMU_UNIMPLEMENTED};
    return PF_PMU_UNIMPLEMENTED;
}
PFPmuState pf_pmu_start(PFPmu *p) { (void)p; return PF_PMU_UNIMPLEMENTED; }
PFPmuState pf_pmu_stop(PFPmu *p) { (void)p; return PF_PMU_UNIMPLEMENTED; }
void pf_pmu_close(PFPmu *p) { (void)p; }
int pf_ring_drain(const unsigned char *r, size_t n, uint64_t h, uint64_t *t,
                  size_t b, PFRingStats *s,
                  void (*f)(void *,const PFSample *),void *c) {
    (void)r; (void)n; (void)h; (void)t; (void)b; (void)s; (void)f; (void)c;
    return PF_RING_UNIMPLEMENTED;
}
