#ifndef CMASTERY_MESSAGING_NURSERY_FOLLOW_H
#define CMASTERY_MESSAGING_NURSERY_FOLLOW_H
#include "messaging.h"
/* Linux/POSIX adapter. See NURSERY.md; these representations are not opaque. */
typedef void (*mfn_drop_fn)(void *);

struct mfn_task
{
    msg_task_fn run;
    mfn_drop_fn drop;
    void *ctx;
};

enum mfn_stop
{
    MFN_COMPLETE,
    MFN_DEADLINE,
    MFN_EXTERNAL_CANCEL,
    MFN_IO_FAILURE
};

struct mfn_outcome
{
    int errors[4], primary;
    bool admitted[4], spawn_failed;
    enum mfn_stop stop;
};
struct mfn_tree;

struct mfn_link
{
    struct mfn_tree *tree;
    unsigned index;
};

struct mfn_tree
{
    struct msg_nursery outer, inner[2];
    struct mfn_task tasks[4];
    struct mfn_link group[2], leaf[4];
    int results[4], pipefd[2];
    bool admitted[4], group_failed[2], start_failed, active, waited;
    enum mfn_stop stop;
};
/* fail_outer and fail_inner[]: one-based admission failure, zero disables. */
enum msg_status mfn_start(struct mfn_tree *, const struct mfn_task[4], size_t,
                          const size_t[2]);
void mfn_cancel(struct mfn_tree *);
/* Single owner. cancel_fd borrowed, deadline absolute CLOCK_MONOTONIC ns.
 * MSG_BUSY retains the entire tree for join retry. out unchanged on failure. */
enum msg_status mfn_wait(struct mfn_tree *, int cancel_fd,
                         long long deadline_ns, struct mfn_outcome *);

struct mfn_pool
{
    const int *values;
    size_t count, workers;
    long sum;
};

int mfn_pool_task(void *, const atomic_bool *);
#endif
