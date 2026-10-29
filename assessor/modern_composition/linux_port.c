#define _POSIX_C_SOURCE 200809L
#include "linux_port.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#ifndef MC_MUTANT
#define MC_MUTANT 0
#endif
static void *worker(void *context)
{
    struct mc_linux_port *p = context;
    struct pollfd fds[2] = {{p->fd, POLLIN, 0}, {p->wake[0], POLLIN, 0}};
    for (;;)
    {
        int ready;
        do
        {
            ready = poll(fds, 2, -1);
        } while (ready < 0 && errno == EINTR);
        pthread_mutex_lock(&p->mutex);
        if (p->stopping)
        {
            pthread_mutex_unlock(&p->mutex);
            break;
        }
        /* A poll error also wakes the owner; read supplies the terminal result.
           Only the owner touches COAsync or runs user callbacks. */
        if (ready < 0 || fds[0].revents)
        {
            p->notification = 1;
            ++p->references;
            ++p->queued;
            pthread_cond_broadcast(&p->changed);
            while (p->notification && !p->stopping)
                pthread_cond_wait(&p->changed, &p->mutex);
        }
        unsigned stopping = p->stopping;
        pthread_mutex_unlock(&p->mutex);
        if (stopping)
            break;
    }
    pthread_mutex_lock(&p->mutex);
    --p->references;
    ++p->worker_exits;
    pthread_cond_broadcast(&p->changed);
    pthread_mutex_unlock(&p->mutex);
    return NULL;
}

int mc_linux_init(struct mc_linux_port *p, struct mc_read *read, int fd)
{
    if (!p || !read || fd < 0)
        return 0;
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || !(flags & O_NONBLOCK))
        return 0;
    *p = (struct mc_linux_port){0};
    p->fd = fd;
    p->read = read;
    if (pipe(p->wake))
        return 0;
    for (unsigned i = 0; i < 2; ++i)
        if (fcntl(p->wake[i], F_SETFL, O_NONBLOCK) < 0)
        {
            close(p->wake[0]);
            close(p->wake[1]);
            return 0;
        }
    if (pthread_mutex_init(&p->mutex, NULL))
    {
        close(p->wake[0]);
        close(p->wake[1]);
        return 0;
    }
    if (pthread_cond_init(&p->changed, NULL))
    {
        pthread_mutex_destroy(&p->mutex);
        close(p->wake[0]);
        close(p->wake[1]);
        return 0;
    }
    p->initialized = 1;
    return 1;
}

static struct co_read_result read_bytes(void *context, unsigned char *out,
                                        size_t size)
{
    struct mc_linux_port *p = context;
    ssize_t n = read(p->fd, out, size);
    if (n > 0)
        return (struct co_read_result){CO_READ_DATA, (size_t)n};
    if (n == 0)
        return (struct co_read_result){CO_READ_EOF, 0};
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        return (struct co_read_result){CO_READ_AGAIN, 0};
    return (struct co_read_result){CO_READ_ERROR, 0};
}

static int arm(void *context, uint32_t generation)
{
    struct mc_linux_port *p = context;
    if (p->started || p->stopping)
        return -1;
    p->generation = generation;
    pthread_mutex_lock(&p->mutex);
    ++p->references;
    pthread_mutex_unlock(&p->mutex);
    if (pthread_create(&p->worker, NULL, worker, p))
    {
        pthread_mutex_lock(&p->mutex);
        --p->references;
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    p->started = 1;
    return 0;
}

int mc_linux_request_cancel(struct mc_linux_port *p)
{
    if (!p || !p->initialized)
        return 0;
    pthread_mutex_lock(&p->mutex);
    unsigned first = !p->stopping;
    p->stopping = 1;
    pthread_cond_broadcast(&p->changed);
    pthread_mutex_unlock(&p->mutex);
    if (first)
    {
        unsigned char byte = 1;
        ssize_t n;
        do
        {
            n = write(p->wake[1], &byte, 1);
        } while (n < 0 && errno == EINTR);
        if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
            return 0;
    }
    return 1;
}

static int quiesce(struct mc_linux_port *p)
{
    if (!mc_linux_request_cancel(p))
        return 0;
    if (MC_MUTANT == 7)
        return 1; /* cancellation request is not quiescence */
    if (p->started)
    {
        if (pthread_join(p->worker, NULL))
            return 0;
        p->started = 0;
        ++p->joined;
    }
    pthread_mutex_lock(&p->mutex);
    if (p->notification && MC_MUTANT != 8)
    {
        p->notification = 0;
        --p->references;
        ++p->discarded;
    }
    pthread_mutex_unlock(&p->mutex);
    return 1;
}

static void disarm(void *context, uint32_t generation)
{
    struct mc_linux_port *p = context;
    /* The void port contract cannot report failed quiescence. An unexpected
       broken pthread/descriptor precondition must never authorize reclamation. */
    if (generation != p->generation || !quiesce(p))
        abort();
}

struct co_async_port mc_linux_interface(struct mc_linux_port *p)
{
    return (struct co_async_port){p, read_bytes, arm, disarm};
}

int mc_linux_wait(struct mc_linux_port *p, unsigned milliseconds)
{
    if (!p || !p->initialized)
        return 0;
    struct timespec deadline;
    if (clock_gettime(CLOCK_REALTIME, &deadline))
        return 0;
    deadline.tv_sec += (time_t)(milliseconds / 1000);
    deadline.tv_nsec += (long)(milliseconds % 1000) * 1000000L;
    if (deadline.tv_nsec >= 1000000000L)
    {
        ++deadline.tv_sec;
        deadline.tv_nsec -= 1000000000L;
    }
    pthread_mutex_lock(&p->mutex);
    int r = 0;
    while (!p->notification && !p->stopping && !r)
        r = pthread_cond_timedwait(&p->changed, &p->mutex, &deadline);
    int ready = p->notification != 0;
    pthread_mutex_unlock(&p->mutex);
    return ready;
}

int mc_linux_dispatch(struct mc_linux_port *p)
{
    if (!p || !p->initialized)
        return 0;
    pthread_mutex_lock(&p->mutex);
    unsigned ready = p->notification && !p->stopping;
    if (ready)
    {
        p->notification = 0;
        --p->references;
        ++p->consumed;
        pthread_cond_broadcast(&p->changed);
    }
    pthread_mutex_unlock(&p->mutex);
    return ready && co_async_notify(&p->read->frame, p->generation) == CO_OK;
}

enum msg_status mc_linux_finish_cancel(struct mc_linux_port *p)
{
    if (!p || !p->initialized || !quiesce(p))
        return MSG_FAILED;
    struct mc_read *read = p->read;
    return mc_read_cancel(read); /* receiver can destroy read and p */
}

int mc_linux_quiescent(struct mc_linux_port *p)
{
    if (!p || !p->initialized)
        return 0;
    pthread_mutex_lock(&p->mutex);
    int result = !p->started && !p->references && !p->notification;
    pthread_mutex_unlock(&p->mutex);
    return result;
}

int mc_linux_destroy(struct mc_linux_port *p)
{
    if (!p || !p->initialized || !mc_linux_quiescent(p))
        return 0;
    close(p->wake[0]);
    close(p->wake[1]);
    pthread_cond_destroy(&p->changed);
    pthread_mutex_destroy(&p->mutex);
    p->initialized = 0;
    return 1;
}
