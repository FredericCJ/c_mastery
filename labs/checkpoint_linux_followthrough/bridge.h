#ifndef P5_BRIDGE_H
#define P5_BRIDGE_H
#include "integration.h"
#include <pthread.h>
#include <time.h>

/*
 * Original local teaching adapter (hosted Linux, POSIX threads). The owner
 * serializes API calls, except that the worker thread runs its callback
 * concurrently. All struct f_async, ring and callback state is protected by
 * `mutex`. Never copy an initialized bridge; the external lifetime outlasts
 * join and destroy.
 *
 * Field groups: `fd` and `wake` are descriptors the bridge owns after init;
 * `initialized`, `started`, `cancelled`, `quiescent`, `hold_ack` and
 * `release_ack` are state flags; `callbacks`, `discarded` and `joins` are
 * counters that tests inspect; `bytes[0..used)` is the data the worker read.
 */
struct p5_bridge
{
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    pthread_t worker;
    struct f_async operation;
    algo_ring queue;
    uint32_t ticket;
    int fd;
    int wake[2];
    int initialized;
    int started;
    int cancelled;
    int quiescent;
    int hold_ack;
    int release_ack;
    int io_error;
    int cleanup_error;
    unsigned callbacks;
    unsigned discarded;
    unsigned joins;
    unsigned char bytes[32];
    size_t used;
};

/*
 * fd is borrowed only during init; a duplicate is owned by the bridge. It must
 * already be nonblocking. hold_ack is a deterministic laboratory gate that
 * withholds the worker's quiescence acknowledgement until
 * p5_bridge_release_gate(). Returns 0, or -1 on failure with nothing acquired.
 */
int p5_bridge_init(struct p5_bridge *, int fd, int hold_ack);

/*
 * Queues a value while no worker is started (before begin, or again after
 * reap). Returns 0, or -1 if full, started or invalid.
 */
int p5_bridge_offer(struct p5_bridge *, int32_t);

/* Starts the worker; stores the new ticket. -1 leaves *ticket unchanged. */
int p5_bridge_begin(struct p5_bridge *, uint32_t *);

/*
 * Requests cancellation of the matching ticket; a request is not an
 * acknowledgement. Returns 1 while cancellation is pending, 0 if the
 * operation was already observed complete and not yet reaped (the queue is
 * emptied), and -1 for a stale or invalid ticket. It also returns -1 when
 * the wake-pipe write fails, after the request has been recorded.
 */
int p5_bridge_cancel(struct p5_bridge *, uint32_t);

/*
 * 0 observed matching quiescence/discard, 1 pending, -1 stale/invalid/error.
 * A repeated or stale callback observation never increments the release count.
 */
int p5_bridge_observe(struct p5_bridge *, uint32_t);

/*
 * Waits until quiescent or until the CLOCK_MONOTONIC deadline. Returns 0
 * quiescent, 1 deadline expired, -1 error.
 */
int p5_bridge_wait(struct p5_bridge *,
                   const struct timespec *monotonic_deadline);
/* Lets a gated worker acknowledge quiescence. Returns 0, or -1 on error. */
int p5_bridge_release_gate(struct p5_bridge *);

/*
 * Join is separate from quiescence; deadline expiration never authorizes
 * free. Returns 0 joined, 1 not yet joinable, -1 error.
 */
int p5_bridge_reap(struct p5_bridge *);

/*
 * Once joined, discards any never-submitted items and closes owned
 * descriptors. Returns 0, 1 when it is not yet safe to destroy, or -1 on
 * error. After a Linux late close error (recorded in cleanup_error) the -1
 * still leaves the bridge destroyed (initialized 0, descriptors -1); a
 * descriptor whose close failed is never retried. A -1 from in_shutdown
 * leaves the bridge untouched and initialized. A -1 from a failed
 * condition-variable or mutex destroy leaves it initialized, with the
 * descriptors already closed (-1).
 */
int p5_bridge_destroy(struct p5_bridge *);
#endif
