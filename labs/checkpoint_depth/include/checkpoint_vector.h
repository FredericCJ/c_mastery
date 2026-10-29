#ifndef CHECKPOINT_VECTOR_H
#define CHECKPOINT_VECTOR_H
#include "checkpoint_numeric.h"
enum cpv_backend { CPV_SCALAR, CPV_AUTO, CPV_AVX, CPV_COMPILER };
struct cpv_plan { enum cpv_backend backend; };
struct cpv_probe {
    void *context;
    int (*leaf1)(void *,uint32_t *);
    uint64_t (*xgetbv0)(void *);
};
/* Probe callbacks are trusted teaching adapters; xgetbv0 is called only after OSXSAVE+AVX. */
int cpv_detect(struct cpv_probe);
int cpv_available(void);
enum cpn_status cpv_plan(struct cpv_plan *,enum cpv_backend);
/* Plan must be initialized, remain unchanged and belong to this execution environment. */
enum cpn_status cpv_run(const struct cpv_plan *,struct cpn_double *,const double *,size_t,double *,size_t);
#endif
