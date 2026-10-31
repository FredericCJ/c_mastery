#ifndef TL_LIFECYCLE_H
#define TL_LIFECYCLE_H
#include <pthread.h>
#include <stddef.h>
struct tl_pool;

/* Published by tl_pool_collect only after every started thread is joined. */
struct tl_stats
{
    unsigned created;
    unsigned joined;
    unsigned completed;
    unsigned sum;
    int create_error;
};

/* Replacement for pthread_join: returns 0 or a pthread error number, and must
 * not consume the handle when it reports an error. */
typedef int (*tl_join_fn)(pthread_t, void **, void *);

/* Pool operations: single owner, cancellation disabled, see CONTRACTS.md.
 * tl_pool_start returns NULL with *error set on failure, otherwise a heap pool
 * the caller must collect and dispose. tl_pool_collect returns 0, EINVAL for
 * null arguments, or the join error without changing the output.
 * tl_pool_dispose returns 0, EINVAL for
 * null arguments or EBUSY while joins are pending, and clears *pool on 0. */
struct tl_pool *tl_pool_start(unsigned workers, unsigned fail_ordinal,
                              int fail_allocation, int *error);
int tl_pool_collect(struct tl_pool *, tl_join_fn, void *, struct tl_stats *);
int tl_pool_dispose(struct tl_pool **);

/* Every field is protected by `mutex`. `changed` is broadcast only for
 * creator_waiting, completed, live_refs/freed (a reference release) and, by a
 * caller, release_worker. Setting created and the initial live_refs while the
 * worker is started, and tl_monitor_init, change fields without a broadcast. */
struct tl_monitor
{
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    unsigned created;
    unsigned creator_waiting;
    unsigned release_worker;
    unsigned completed;
    unsigned live_refs;
    unsigned freed;
    unsigned sum;
};

/* Input to tl_detached_creator. `input` is borrowed only for the duration of
 * the call and `error` is written by the creator before it returns. */
struct tl_creator
{
    struct tl_monitor *monitor;
    const unsigned char *input;
    size_t length;
    int wait_for_completion;
    int error;
};

int tl_monitor_init(struct tl_monitor *);
int tl_monitor_destroy(struct tl_monitor *);
void *tl_detached_creator(void *);
#endif
