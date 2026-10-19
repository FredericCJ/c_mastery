#ifndef CMASTERY_MPT_FAULT_ADAPTER_H
#define CMASTERY_MPT_FAULT_ADAPTER_H
#include "law_traces.h"
enum {MPT_SET,MPT_DROP,MPT_CHECK};
enum {MPT_NONE,MPT_HASH_LAW,MPT_ROLLBACK,MPT_CALLBACK,MPT_CLEANUP};
/* SET value codes: empty, A, a, space-A-space, B. CHECK ignores slot/value.
 * DROP ignores value. All unused fields must be zero in admitted traces. */
typedef struct {unsigned operation,slot,value;} MptStep;
typedef struct {MptStep steps[12];size_t count;unsigned fail_at;} MptTrace;
typedef int (*MptEq)(void *,MptView,MptView);
typedef uint64_t (*MptHash)(void *,MptView);
typedef struct {MptEq equal;MptHash hash;void *context;} MptLaws;
typedef struct {unsigned failure;size_t first_step;unsigned allocations,refusals,releases,live;} MptOutcome;
int mpt_replay(const MptTrace *,MptLaws,MptOutcome *);
int mpt_shrink(const MptTrace *,MptLaws,MptTrace *,unsigned *);
#endif
