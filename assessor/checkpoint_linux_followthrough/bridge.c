/*
 * Reference implementation of bridge.h: one worker thread reads a nonblocking
 * descriptor under `mutex`, a self-pipe wakes it for cancellation, and
 * quiescence is acknowledged only after the worker stops touching the
 * descriptor and the payload buffer.
 *
 * Lock order: only p->mutex is ever held; no call blocks while holding it
 * except pthread_cond_wait / pthread_cond_timedwait, which release it.
 * p5_bridge_begin does call pthread_create under the mutex. The design also
 * assumes the caller keeps O_NONBLOCK on the shared open file description:
 * the bridge owns a duplicate descriptor, not a separate description.
 *
 * Mutants (author campaign only, -DP5_MUTANT=n): 1 treats a cancel request as
 * quiescence, 2 accepts a stale ticket in observe, 3 lets destroy report
 * permission while the worker is still started.
 */
#define _GNU_SOURCE
#include "bridge.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#ifndef P5_MUTANT
#define P5_MUTANT 0
#endif

enum bridge_limit
{
    QUEUE_CAPACITY = 4,
    WAKE_DRAIN_BYTES = 32
};

/* How the worker loop proceeds after one wakeup. */
enum worker_step
{
    WORKER_CONTINUE,
    WORKER_STOP,
    WORKER_ABORT
};

/*
 * Reads whatever is available (mutex held). Returns nonzero when the worker
 * should stop: end of file, a hard error, or a full buffer. EAGAIN and EINTR
 * are not errors for a nonblocking descriptor.
 */
static int read_available(struct p5_bridge *p)
{
    const ssize_t got =
        read(p->fd, p->bytes + p->used, sizeof p->bytes - p->used);
    if (got > 0)
    {
        p->used += (size_t)got;
    }
    else if (got < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
             errno != EINTR)
    {
        p->io_error = errno;
    }
    return got == 0 || p->io_error != 0 || p->used == sizeof p->bytes;
}

/* One poll-and-service iteration. WORKER_ABORT means the mutex failed. */
static enum worker_step worker_iteration(struct p5_bridge *p,
                                         struct pollfd fds[2])
{
    int ready;
    do
    {
        ready = poll(fds, 2, -1);
    } while (ready < 0 && errno == EINTR);
    const int poll_errno = errno; /* Meaningful only when ready < 0. */
    if (pthread_mutex_lock(&p->mutex))
    {
        return WORKER_ABORT;
    }
    if (p->cancelled)
    {
        pthread_mutex_unlock(&p->mutex);
        return WORKER_STOP;
    }
    if (ready < 0)
    {
        p->io_error = poll_errno;
        pthread_mutex_unlock(&p->mutex);
        return WORKER_STOP;
    }
    int done = 0;
    if (fds[0].revents)
    {
        done = read_available(p);
        pthread_cond_broadcast(&p->changed);
    }
    pthread_mutex_unlock(&p->mutex);
    return done ? WORKER_STOP : WORKER_CONTINUE;
}

/*
 * Publishes quiescence. A withheld gate (hold_ack) is a deterministic
 * laboratory hold, not simulated evidence of earlier quiescence.
 */
static void acknowledge_quiescence(struct p5_bridge *p)
{
    if (pthread_mutex_lock(&p->mutex))
    {
        return;
    }
    while (p->hold_ack && !p->release_ack)
    {
        if (pthread_cond_wait(&p->changed, &p->mutex))
        {
            pthread_mutex_unlock(&p->mutex);
            return;
        }
    }
    p->quiescent = 1;
    ++p->callbacks;
    pthread_cond_broadcast(&p->changed);
    pthread_mutex_unlock(&p->mutex);
}

static void *work(void *arg)
{
    struct p5_bridge *p = arg;
    struct pollfd fds[2] = {{p->fd, POLLIN, 0}, {p->wake[0], POLLIN, 0}};
    enum worker_step step;
    do
    {
        step = worker_iteration(p, fds);
    } while (step == WORKER_CONTINUE);
    if (step == WORKER_ABORT)
    {
        return 0;
    }
    /* No more descriptor I/O or payload access after this point. */
    acknowledge_quiescence(p);
    return 0;
}

/* Creates the condition variable on CLOCK_MONOTONIC. Returns 0 or an errno. */
static int init_condition(pthread_cond_t *cond)
{
    pthread_condattr_t attr;
    int error = pthread_condattr_init(&attr);
    if (error)
    {
        return error;
    }
    error = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (!error)
    {
        error = pthread_cond_init(cond, &attr);
    }
    pthread_condattr_destroy(&attr);
    return error;
}

/*
 * Returns -1 without writing *p if fd is invalid or blocking. Any later
 * failure releases everything acquired here and leaves *p with
 * initialized == 0. Forward gotos undo acquisitions in reverse order.
 */
int p5_bridge_init(struct p5_bridge *p, int fd, int hold)
{
    if (!p)
    {
        return -1;
    }
    const int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || !(flags & O_NONBLOCK))
    {
        return -1;
    }
    *p = (struct p5_bridge){.fd = -1, .wake = {-1, -1}, .hold_ack = !!hold};
    p->fd = fcntl(fd, F_DUPFD_CLOEXEC, 0);
    if (p->fd < 0)
    {
        return -1;
    }
    if (pipe2(p->wake, O_NONBLOCK | O_CLOEXEC))
    {
        close(p->fd);
        return -1;
    }
    if (pthread_mutex_init(&p->mutex, 0))
    {
        goto err_close_fds;
    }
    if (init_condition(&p->changed))
    {
        goto err_destroy_mutex;
    }
    if (algo_ring_init(&p->queue, QUEUE_CAPACITY) != ALGO_OK)
    {
        pthread_cond_destroy(&p->changed);
        goto err_destroy_mutex;
    }
    p->initialized = 1;
    return 0;
err_destroy_mutex:
    pthread_mutex_destroy(&p->mutex);
err_close_fds:
    close(p->fd);
    close(p->wake[0]);
    close(p->wake[1]);
    return -1;
}

int p5_bridge_offer(struct p5_bridge *p, int32_t value)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    int result = -1;
    if (!p->started && algo_ring_put(&p->queue, value) == ALGO_OK)
    {
        result = 0;
    }
    pthread_mutex_unlock(&p->mutex);
    return result;
}

/* Discards stale wake-up bytes from a previous cancellation (mutex held). */
static void drain_wake_pipe(struct p5_bridge *p)
{
    unsigned char scratch[WAKE_DRAIN_BYTES];
    while (read(p->wake[0], scratch, sizeof scratch) > 0)
    {
    }
}

int p5_bridge_begin(struct p5_bridge *p, uint32_t *ticket)
{
    if (!p || !ticket || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    if (p->started || f_submit(&p->operation, &p->ticket) != F_OK)
    {
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    p->cancelled = 0;
    p->quiescent = 0;
    p->release_ack = 0;
    p->used = 0;
    p->io_error = 0;
    drain_wake_pipe(p);
    if (pthread_create(&p->worker, 0, work, p))
    {
        /* No worker exists: local construction failure is known quiescent. */
        (void)f_complete(&p->operation, p->ticket);
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    p->started = 1;
    *ticket = p->ticket;
    pthread_mutex_unlock(&p->mutex);
    return 0;
}

/* Wakes the worker through the self-pipe. A full pipe already has a wakeup. */
static int signal_worker(struct p5_bridge *p)
{
    const unsigned char byte = 1;
    ssize_t written;
    do
    {
        written = write(p->wake[1], &byte, 1);
    } while (written < 0 && errno == EINTR);
    if (written < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        return -1;
    }
    return 0;
}

int p5_bridge_cancel(struct p5_bridge *p, uint32_t ticket)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    if (!p->started || ticket != p->ticket)
    {
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    const int result =
        in_shutdown(&p->queue, &p->operation, 0, ticket, &p->discarded);
    p->cancelled = 1;
    if (P5_MUTANT == 1)
    {
        p->quiescent = 1; /* Wrong: a request is not an acknowledgement. */
    }
    pthread_mutex_unlock(&p->mutex);
    if (signal_worker(p) != 0)
    {
        return -1;
    }
    return result;
}

int p5_bridge_observe(struct p5_bridge *p, uint32_t ticket)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    int result = -1;
    if ((P5_MUTANT == 2 || ticket == p->ticket) && p->started && p->cancelled)
    {
        if (p->operation.state == F_DONE)
        {
            result = 0;
        }
        else
        {
            result =
                in_shutdown(&p->queue, &p->operation, (unsigned)p->quiescent,
                            p->ticket, &p->discarded);
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return result;
}

int p5_bridge_wait(struct p5_bridge *p, const struct timespec *until)
{
    if (!p || !until || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    int error = 0;
    while (!p->quiescent && !error)
    {
        error = pthread_cond_timedwait(&p->changed, &p->mutex, until);
    }
    int result = -1;
    if (p->quiescent)
    {
        result = 0;
    }
    else if (error == ETIMEDOUT)
    {
        result = 1;
    }
    pthread_mutex_unlock(&p->mutex);
    return result;
}

int p5_bridge_release_gate(struct p5_bridge *p)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    p->release_ack = 1;
    const int error = pthread_cond_broadcast(&p->changed);
    pthread_mutex_unlock(&p->mutex);
    return error ? -1 : 0;
}

int p5_bridge_reap(struct p5_bridge *p)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    const int joinable =
        p->started && p->quiescent && p->operation.state == F_DONE;
    pthread_mutex_unlock(&p->mutex);
    if (!joinable)
    {
        return 1;
    }
    if (pthread_join(p->worker, 0))
    {
        return -1;
    }
    p->started = 0;
    ++p->joins;
    return 0;
}

/*
 * Closes every owned descriptor exactly once. A failing close is recorded in
 * cleanup_error (first error wins) and is never retried: on Linux the
 * descriptor is already released, and another thread may have reused its
 * number. Returns -1 if any close failed.
 */
static int close_owned_descriptors(struct p5_bridge *p)
{
    int result = 0;
    const int owned[3] = {p->fd, p->wake[0], p->wake[1]};
    for (unsigned i = 0; i < 3; i++)
    {
        if (close(owned[i]))
        {
            if (!p->cleanup_error)
            {
                p->cleanup_error = errno;
            }
            result = -1;
        }
    }
    p->fd = -1;
    p->wake[0] = -1;
    p->wake[1] = -1;
    return result;
}

int p5_bridge_destroy(struct p5_bridge *p)
{
    if (!p || !p->initialized)
    {
        return -1;
    }
    if (p->started)
    {
        /* Mutant 3 wrongly reports permission; it still frees nothing. */
        return P5_MUTANT == 3 ? 0 : 1;
    }
    if (p->operation.state == F_INFLIGHT ||
        p->operation.state == F_CANCEL_REQUESTED)
    {
        return 1;
    }
    /* Explicit destruction abandons never-submitted/construction-failed queued
       work. No worker was created (or all were joined), hence local quiescence. */
    if (in_shutdown(&p->queue, &p->operation, 1, p->ticket, &p->discarded))
    {
        return -1;
    }
    const int close_result = close_owned_descriptors(p);
    if (pthread_cond_destroy(&p->changed) || pthread_mutex_destroy(&p->mutex))
    {
        return -1;
    }
    p->initialized = 0;
    return close_result;
}
