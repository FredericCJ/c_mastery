#ifndef CHECKPOINT_VECTOR_H
#define CHECKPOINT_VECTOR_H
#include "checkpoint_numeric.h"
typedef enum { CPV_SCALAR, CPV_AUTO, CPV_AVX, CPV_COMPILER } CPVBackend;
typedef struct { CPVBackend backend; } CPVPlan;
typedef struct {
    void *context;
    int (*leaf1)(void *,uint32_t *);
    uint64_t (*xgetbv0)(void *);
} CPVProbe;
/* Probe callbacks are trusted teaching adapters; xgetbv0 is called only after OSXSAVE+AVX. */
int cpv_detect(CPVProbe);
int cpv_available(void);
CPNStatus cpv_plan(CPVPlan *,CPVBackend);
/* Plan must be initialized, remain unchanged and belong to this execution environment. */
CPNStatus cpv_run(const CPVPlan *,CPNDouble *,const double *,size_t,double *,size_t);
#endif
