#ifndef CMASTERY_JUMP_SCOPE_H
#define CMASTERY_JUMP_SCOPE_H
#include "errors.h"
#include "ownership.h"
enum { EJD_OK,EJD_INVALID,EJD_NOT_IMPLEMENTED };
/* cut0..6 raises before/after each of three inner registrations; cut7/code0
   returns normally. Other cuts require an admitted non-OK er_code. */
struct ejd_plan {unsigned cut;er_code code;};
struct ejd_report {
 er_result result,inner;
 unsigned stage,effects_after,trace_count,trace[5],remaining[5];
 unsigned held,scope_finishes,scope_empty,frames_inactive,inner_entered,outer_live;
};
/* All storage live, disjoint, serialized; allocator cannot jump or reenter.
   published has at least3increments of headroom; report published after drain. */
int ejd_run(struct v4_allocator allocator,struct ejd_plan plan,unsigned *published,struct ejd_report *report);
#endif
