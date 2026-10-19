#ifndef CMASTERY_OBJECT_RUNTIME_VM_H
#define CMASTERY_OBJECT_RUNTIME_VM_H
#include "object_runtime.h"
#include "cvm.h"
#include "algo.h"

/* Pair-field offsets 0,1 and representable one-past 2; no raw heap address. */
typedef struct { CvmValue base; uint8_t offset; } ORVDerived;
typedef struct {
 Cvm *vm;
 algo_pool registrations;
 ORVDerived *roots[16];
 uint8_t paused, foreign;
} ORVM;
typedef struct {
 unsigned slots, root_copies, frames, live, moved, work;
 uint8_t forwarding[16];
} ORVStats;
typedef ORStatus (*ORVForeign)(ORVM *,ORVDerived *,ORVDerived *,void *,CvmValue *);
void orv_init(ORVM *,Cvm *);
ORStatus orv_pause(ORVM *,int paused);
ORStatus orv_register(ORVM *,ORVDerived *,uint64_t *ticket);
ORStatus orv_unregister(ORVM *,uint64_t ticket);
ORStatus orv_collect(ORVM *,unsigned budget,unsigned fault_cut,ORVStats *);
ORStatus orv_pair(ORVM *,ORVDerived *,ORVDerived *,int force_collection,
                  unsigned fault_cut,CvmValue *);
ORStatus orv_call(ORVM *,CvmValue,CvmValue,ORVForeign,void *,CvmValue *);
#endif
