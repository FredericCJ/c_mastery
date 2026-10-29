#define _GNU_SOURCE
#include "bridge.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#ifndef P5_MUTANT
#define P5_MUTANT 0
#endif
static void *work(void *arg)
{
    struct p5_bridge *p = arg;
    struct pollfd fds[2] = {{p->fd, POLLIN, 0}, {p->wake[0], POLLIN, 0}};
    for (;;)
    {
        int n;
        do
        {
            n = poll(fds, 2, -1);
        } while (n < 0 && errno == EINTR);
        if (pthread_mutex_lock(&p->mutex))
        {
            return 0;
        }
        if (p->cancelled)
        {
            pthread_mutex_unlock(&p->mutex);
            break;
        }
        if (n < 0)
        {
            p->io_error = errno;
            pthread_mutex_unlock(&p->mutex);
            break;
        }
        if (fds[0].revents)
        {
            ssize_t got =
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
            int done = got == 0 || p->io_error || p->used == sizeof p->bytes;
            pthread_cond_broadcast(&p->changed);
            pthread_mutex_unlock(&p->mutex);
            if (done)
            {
                break;
            }
        }
        else
        {
            pthread_mutex_unlock(&p->mutex);
        }
    }
    /* No more descriptor I/O or payload access after this point. A withheld
       completion gate is not simulated evidence of earlier quiescence. */
    if (pthread_mutex_lock(&p->mutex))
    {
        return 0;
    }
    while (p->hold_ack && !p->release_ack)
    {
        if (pthread_cond_wait(&p->changed, &p->mutex))
        {
            pthread_mutex_unlock(&p->mutex);
            return 0;
        }
    }
    p->quiescent = 1;
    ++p->callbacks;
    pthread_cond_broadcast(&p->changed);
    pthread_mutex_unlock(&p->mutex);
    return 0;
}

int p5_bridge_init(struct p5_bridge *p, int fd, int hold)
{
    if (!p)
    {
        return -1;
    }
    int flags = fcntl(fd, F_GETFL);
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
        goto fail_fds;
    }
    pthread_condattr_t a;
    if (pthread_condattr_init(&a))
    {
        goto fail_mutex;
    }
    int e = pthread_condattr_setclock(&a, CLOCK_MONOTONIC);
    if (!e)
    {
        e = pthread_cond_init(&p->changed, &a);
    }
    pthread_condattr_destroy(&a);
    if (e)
    {
        goto fail_mutex;
    }
    if (algo_ring_init(&p->queue, 4) != ALGO_OK)
    {
        pthread_cond_destroy(&p->changed);
        goto fail_mutex;
    }
    p->initialized = 1;
    return 0;
fail_mutex:
    pthread_mutex_destroy(&p->mutex);
fail_fds:
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
    int r = p->started                                   ? -1
            : algo_ring_put(&p->queue, value) == ALGO_OK ? 0
                                                         : -1;
    pthread_mutex_unlock(&p->mutex);
    return r;
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
    p->cancelled = p->quiescent = p->release_ack = 0;
    p->used = 0;
    p->io_error = 0;
    unsigned char discarded[32];
    while (read(p->wake[0], discarded, sizeof discarded) > 0)
    {
    }
    int e = pthread_create(&p->worker, 0, work, p);
    if (e)
    { /* No worker exists: local construction failure is known quiescent. */
        (void)f_complete(&p->operation, p->ticket);
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    p->started = 1;
    *ticket = p->ticket;
    pthread_mutex_unlock(&p->mutex);
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
    int r = in_shutdown(&p->queue, &p->operation, 0, ticket, &p->discarded);
    p->cancelled = 1;
    if (P5_MUTANT == 1)
    {
        p->quiescent = 1; /* Wrong: a request is not an acknowledgement. */
    }
    pthread_mutex_unlock(&p->mutex);
    unsigned char b = 1;
    ssize_t n;
    do
    {
        n = write(p->wake[1], &b, 1);
    } while (n < 0 && errno == EINTR);
    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        return -1;
    }
    return r;
}

int p5_bridge_observe(struct p5_bridge *p, uint32_t ticket)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    int r = -1;
    if ((P5_MUTANT == 2 || ticket == p->ticket) && p->started && p->cancelled)
    {
        if (p->operation.state == F_DONE)
        {
            r = 0;
        }
        else
        {
            r = in_shutdown(&p->queue, &p->operation, (unsigned)p->quiescent,
                            p->ticket, &p->discarded);
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return r;
}

int p5_bridge_wait(struct p5_bridge *p, const struct timespec *until)
{
    if (!p || !until || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    int e = 0;
    while (!p->quiescent && !e)
    {
        e = pthread_cond_timedwait(&p->changed, &p->mutex, until);
    }
    int r = p->quiescent ? 0 : e == ETIMEDOUT ? 1 : -1;
    pthread_mutex_unlock(&p->mutex);
    return r;
}

int p5_bridge_release_gate(struct p5_bridge *p)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    p->release_ack = 1;
    int e = pthread_cond_broadcast(&p->changed);
    pthread_mutex_unlock(&p->mutex);
    return e ? -1 : 0;
}

int p5_bridge_reap(struct p5_bridge *p)
{
    if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    {
        return -1;
    }
    int ready = p->started && p->quiescent && p->operation.state == F_DONE;
    pthread_mutex_unlock(&p->mutex);
    if (!ready)
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

int p5_bridge_destroy(struct p5_bridge *p)
{
    if (!p || !p->initialized)
    {
        return -1;
    }
    if (p->started)
    {
        return P5_MUTANT == 3
                   ? 0
                   : 1; /* Wrong mutant merely reports permission; no unsafe free. */
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
    int result = 0;
    int fds[3] = {p->fd, p->wake[0], p->wake[1]};
    for (unsigned i = 0; i < 3; i++)
    {
        if (close(fds[i]))
        {
            if (!p->cleanup_error)
            {
                p->cleanup_error = errno;
            }
            result = -1;
        }
    }
    p->fd = p->wake[0] = p->wake[1] = -1;
    if (pthread_cond_destroy(&p->changed) || pthread_mutex_destroy(&p->mutex))
    {
        return -1;
    }
    p->initialized = 0;
    return result; /* Linux close errors recorded; never retry fd. */
}
