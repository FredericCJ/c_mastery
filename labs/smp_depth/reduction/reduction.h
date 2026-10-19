#ifndef SMP_REDUCTION_H
#define SMP_REDUCTION_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
enum {SR_OK,SR_INVALID,SR_BUSY,SR_OVERFLOW,SR_FAILED,SR_INCOMPLETE};
enum {SR_UNSIGNED,SR_SIGNED,SR_BYTES,SR_WORKERS=4,SR_BINS=16};
typedef struct {size_t begin,end;} SrRange;
typedef struct {uint64_t unsigned_sum,hash,power,bins[SR_BINS];int64_t signed_sum;size_t count;} SrResult;
typedef struct {atomic_uint state;unsigned error;SrResult value;} SrSlot;
typedef struct {const void *input;atomic_uint *visits;size_t count;unsigned workers,mode;SrSlot slot[SR_WORKERS];} SrWorld;
/* Input is respectively uint64_t[], int64_t[], or uint8_t[] (values0..15).
 * The real input and atomic visit-array extents cover count elements. They and
 * the world/output are disjoint, live and stable until every admitted caller
 * has finished. Initialization/reuse requires whole-world quiescence; no stale
 * worker invocation may survive it. Each worker claims its slot at most once.
 * count0 permits NULL input/visits. Invalid admission preserves outputs. */
int sr_partition(size_t count,unsigned workers,unsigned worker,SrRange *);
int sr_init(SrWorld *,const void *,atomic_uint *,size_t,unsigned workers,unsigned mode);
/* fail_after is a local processed-element count; SIZE_MAX disables injection.
 * Failure publishes a terminal error, never an accepted global result. */
int sr_work(SrWorld *,unsigned worker,size_t fail_after);
/* INCOMPLETE while any participant is not terminal. It acquire-observes every
 * terminal slot and combines in worker/range order. Any failure/overflow or
 * coverage error preserves output. Successful repeated reads are stable. */
int sr_finish(const SrWorld *,SrResult *);
#endif
