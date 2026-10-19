#ifndef CMASTERY_JUMP_SCOPE_H
#define CMASTERY_JUMP_SCOPE_H
#include "errors.h"
#include "ownership.h"
enum { EJD_OK,EJD_INVALID,EJD_NOT_IMPLEMENTED };
/* cut0..6 raises before/after each of three inner registrations; cut7/code0
   returns normally. Other cuts require an admitted non-OK er_code. */
typedef struct {unsigned cut;er_code code;} EjdPlan;
typedef struct {
 er_result result,inner;
 unsigned stage,effects_after,trace_count,trace[5],remaining[5];
 unsigned held,scope_finishes,scope_empty,frames_inactive,inner_entered,outer_live;
} EjdReport;
/* All storage live, disjoint, serialized; allocator cannot jump or reenter.
   published has at least3increments of headroom; report published after drain. */
int ejd_run(V4Allocator allocator,EjdPlan plan,unsigned *published,EjdReport *report);
#endif
