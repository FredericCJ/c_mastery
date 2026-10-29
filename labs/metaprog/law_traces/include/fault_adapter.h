#ifndef CMASTERY_MPT_FAULT_ADAPTER_H
#define CMASTERY_MPT_FAULT_ADAPTER_H
#include "law_traces.h"
enum {MPT_SET,MPT_DROP,MPT_CHECK};
enum {MPT_NONE,MPT_HASH_LAW,MPT_ROLLBACK,MPT_CALLBACK,MPT_CLEANUP};
/* SET value codes: empty, A, a, space-A-space, B. CHECK ignores slot/value.
 * DROP ignores value. All unused fields must be zero in admitted traces. */
struct mpt_step {unsigned operation,slot,value;};
struct mpt_trace {struct mpt_step steps[12];size_t count;unsigned fail_at;};
typedef int (*mpt_eq_fn)(void *,struct mpt_view,struct mpt_view);
typedef uint64_t (*mpt_hash_fn)(void *,struct mpt_view);
struct mpt_laws {mpt_eq_fn equal;mpt_hash_fn hash;void *context;};
struct mpt_outcome {unsigned failure;size_t first_step;unsigned allocations,refusals,releases,live;};
int mpt_replay(const struct mpt_trace *,struct mpt_laws,struct mpt_outcome *);
int mpt_shrink(const struct mpt_trace *,struct mpt_laws,struct mpt_trace *,unsigned *);
#endif
