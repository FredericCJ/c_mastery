#ifndef CMASTERY_MESSAGING_NURSERY_FOLLOW_H
#define CMASTERY_MESSAGING_NURSERY_FOLLOW_H
#include "messaging.h"
/* Linux/POSIX adapter. See NURSERY.md; these representations are not opaque. */
typedef void (*MfnDrop)(void *);
typedef struct { MsgTask run; MfnDrop drop; void *ctx; } MfnTask;
typedef enum { MFN_COMPLETE, MFN_DEADLINE, MFN_EXTERNAL_CANCEL,
               MFN_IO_FAILURE } MfnStop;
typedef struct {
  int errors[4], primary;
  bool admitted[4], spawn_failed;
  MfnStop stop;
} MfnOutcome;
typedef struct MfnTree MfnTree;
typedef struct { MfnTree *tree; unsigned index; } MfnLink;
struct MfnTree {
  MsgNursery outer, inner[2];
  MfnTask tasks[4];
  MfnLink group[2], leaf[4];
  int results[4], pipefd[2];
  bool admitted[4], group_failed[2], start_failed, active, waited;
  MfnStop stop;
};
/* fail_outer and fail_inner[]: one-based admission failure, zero disables. */
MsgStatus mfn_start(MfnTree *, const MfnTask[4], size_t, const size_t[2]);
void mfn_cancel(MfnTree *);
/* Single owner. cancel_fd borrowed, deadline absolute CLOCK_MONOTONIC ns.
 * MSG_BUSY retains the entire tree for join retry. out unchanged on failure. */
MsgStatus mfn_wait(MfnTree *, int cancel_fd, long long deadline_ns, MfnOutcome *);
typedef struct { const int *values; size_t count, workers; long sum; } MfnPool;
int mfn_pool_task(void *, const atomic_bool *);
#endif
