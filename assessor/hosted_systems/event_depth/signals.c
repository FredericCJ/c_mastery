#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t stop_pending, other_pending, child_pending;
static _Atomic int pipe_writer = -1;
static int live;
static const int signals[3] = {SIGUSR1, SIGUSR2, SIGCHLD};

static void handler(int signal)
{
    int saved = errno;
#ifndef ES_MUTANT_FULL
    if (signal == SIGUSR1)
    {
        stop_pending = 1;
    }
    if (signal == SIGUSR2)
    {
        other_pending = 1;
    }
    if (signal == SIGCHLD)
    {
        child_pending = 1;
    }
#endif
    unsigned char byte = 0;
    ssize_t result = write(pipe_writer, &byte, 1);
#ifdef ES_MUTANT_FULL
    if (result == 1)
    {
        if (signal == SIGUSR1)
        {
            stop_pending = 1;
        }
        if (signal == SIGUSR2)
        {
            other_pending = 1;
        }
        if (signal == SIGCHLD)
        {
            child_pending = 1;
        }
    }
#else
    (void)result; /* A failed notification cannot erase durable predicates. */
#endif
#ifndef ES_MUTANT_ERRNO
    errno = saved;
#else
    (void)saved;
#endif
}

int es_open(struct es_signals *p)
{
    if (!atomic_is_lock_free(&pipe_writer))
    {
        errno = ENOTSUP;
        return -1;
    }
    if (!p || live || p->active)
    {
        errno = EBUSY;
        return -1;
    }
    sigemptyset(&p->managed);
    for (unsigned i = 0; i < 3; ++i)
    {
        sigaddset(&p->managed, signals[i]);
    }
    int e = pthread_sigmask(SIG_BLOCK, &p->managed, &p->previous_mask);
    if (e)
    {
        errno = e;
        return -1;
    }
    sigset_t pending;
    if (sigpending(&pending) < 0)
    {
        goto mask_fail;
    }
    for (unsigned i = 0; i < 3; ++i)
    {
        if (sigismember(&pending, signals[i]) == 1)
        {
            errno = EBUSY;
            goto mask_fail;
        }
    }
    int fds[2];
    if (pipe2(fds, O_NONBLOCK | O_CLOEXEC) < 0)
    {
        goto mask_fail;
    }
    p->read_fd = fds[0];
    p->write_fd = fds[1];
    p->waiting_mask = p->previous_mask;
    for (unsigned i = 0; i < 3; ++i)
    {
        sigdelset(&p->waiting_mask, signals[i]);
    }
    struct sigaction a = {0};
    a.sa_handler = handler;
    a.sa_mask = p->managed;
    unsigned installed = 0;
    for (; installed < 3; ++installed)
    {
        if (sigaction(signals[installed], &a, &p->previous_actions[installed]) <
            0)
        {
            break;
        }
    }
    if (installed != 3)
    {
        int saved = errno;
        while (installed)
        {
            --installed;
            sigaction(signals[installed], &p->previous_actions[installed], 0);
        }
        close(fds[0]);
        close(fds[1]);
        errno = saved;
        goto mask_fail;
    }
    stop_pending = other_pending = child_pending = 0;
    pipe_writer = fds[1];
    live = p->active = 1;
    return 0;
mask_fail:
{
    int saved = errno;
    pthread_sigmask(SIG_SETMASK, &p->previous_mask, 0);
    errno = saved;
    return -1;
}
}

int es_wait(struct es_signals *p, const struct timespec *relative)
{
    if (!p || !p->active)
    {
        errno = EINVAL;
        return -1;
    }
    if (stop_pending || other_pending || child_pending)
    {
        return 1;
    }
    struct pollfd fd = {p->read_fd, POLLIN, 0};
    int result = ppoll(&fd, 1, relative, &p->waiting_mask);
    if (result < 0 && errno != EINTR)
    {
        return -1;
    }
    return stop_pending || other_pending || child_pending || result > 0;
}

unsigned es_take(struct es_signals *p, unsigned selected)
{
    if (!p || !p->active)
    {
        return 0;
    }
    unsigned found = 0;
    if ((selected & ES_STOP) && stop_pending)
    {
        found |= ES_STOP;
        stop_pending = 0;
    }
    if ((selected & ES_OTHER) && other_pending)
    {
        found |= ES_OTHER;
        other_pending = 0;
    }
    if ((selected & ES_CHILD) && child_pending)
    {
        found |= ES_CHILD;
        child_pending = 0;
    }
#ifdef ES_MUTANT_CLEAR_ALL
    stop_pending = other_pending = child_pending = 0;
#endif
    unsigned char bytes[128];
    while (read(p->read_fd, bytes, sizeof bytes) > 0)
    {
    }
    return found;
}

int es_close(struct es_signals *p)
{
    if (!p || !p->active)
    {
        errno = EINVAL;
        return -1;
    }
    /* Closing explicitly abandons these owned notifications after producers
     * quiesce. No callback can run with the managed mask still blocked. */
    struct timespec zero = {0, 0};
    for (;;)
    {
        int n = sigtimedwait(&p->managed, 0, &zero);
        if (n > 0 || (n < 0 && errno == EINTR))
        {
            continue;
        }
        if (n < 0 && errno == EAGAIN)
        {
            break;
        }
        return -1; /* Keep ownership/mask intact on an unexpected drain error. */
    }
    int error = 0;
    for (unsigned i = 0; i < 3; ++i)
    {
        if (sigaction(signals[i], &p->previous_actions[i], 0) < 0)
        {
            error = errno;
        }
    }
    pipe_writer = -1;
    if (close(p->read_fd) < 0)
    {
        error = errno;
    }
    if (close(p->write_fd) < 0)
    {
        error = errno;
    }
    stop_pending = other_pending = child_pending = 0;
    live = p->active = 0;
    int e = pthread_sigmask(SIG_SETMASK, &p->previous_mask, 0);
    if (e)
    {
        error = e;
    }
    if (error)
    {
        errno = error;
        return -1;
    }
    return 0;
}

int es_remaining(struct timespec now, struct timespec deadline,
                 struct timespec *r)
{
    if (!r || now.tv_sec < 0 || deadline.tv_sec < 0 || now.tv_nsec < 0 ||
        now.tv_nsec >= 1000000000L || deadline.tv_nsec < 0 ||
        deadline.tv_nsec >= 1000000000L)
    {
        errno = EINVAL;
        return -1;
    }
    if (now.tv_sec > deadline.tv_sec ||
        (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec))
    {
        *r = (struct timespec){0, 0};
        return 0;
    }
    r->tv_sec = deadline.tv_sec - now.tv_sec;
    r->tv_nsec = deadline.tv_nsec - now.tv_nsec;
    if (r->tv_nsec < 0)
    {
        --r->tv_sec;
        r->tv_nsec += 1000000000L;
    }
    return 1;
}

int es_wait_until(struct es_wait ops, struct timespec deadline)
{
    if (!ops.now || !ops.wait)
    {
        errno = EINVAL;
        return -1;
    }
#ifdef ES_MUTANT_RESTART_BUDGET
    struct timespec original = {0, 0};
    int first = 1;
#endif
    for (;;)
    {
        struct timespec now, remaining;
        if (ops.now(ops.context, &now) < 0)
        {
            return -1;
        }
        int left = es_remaining(now, deadline, &remaining);
        if (left <= 0)
        {
            return left;
        }
#ifdef ES_MUTANT_RESTART_BUDGET
        if (first)
        {
            original = remaining;
            first = 0;
        }
        remaining = original;
#endif
        int result = ops.wait(ops.context, &remaining);
        if (result > 0)
        {
            return result;
        }
        if (result < 0 && errno != EINTR)
        {
            return -1;
        }
        /* A timeout is a hint to check the absolute clock again. */
    }
}

static int real_now(void *context, struct timespec *out)
{
    (void)context;
    return clock_gettime(CLOCK_MONOTONIC, out);
}

static int real_wait(void *context, const struct timespec *remaining)
{
    struct pollfd fd = {*(int *)context, POLLIN, 0};
    return ppoll(&fd, 1, remaining, 0);
}

int es_wait_fd(int fd, struct timespec deadline)
{
    if (fd < 0)
    {
        errno = EINVAL;
        return -1;
    }
    return es_wait_until((struct es_wait){real_now, real_wait, &fd}, deadline);
}

int es_reap(pid_t *pids, int *statuses, unsigned count)
{
    if (count > 64 || ((!pids || !statuses) && count))
    {
        errno = EINVAL;
        return -1;
    }
    int reaped = 0;
    for (unsigned i = 0; i < count; ++i)
    {
        if (!pids[i])
        {
            continue;
        }
        if (pids[i] < 0)
        {
            errno = EINVAL;
            return -1;
        }
        pid_t r;
        do
        {
            r = waitpid(pids[i], &statuses[i], WNOHANG);
        } while (r < 0 && errno == EINTR);
        if (r < 0)
        {
            return -1;
        }
        if (r)
        {
            pids[i] = 0;
            ++reaped;
        }
#ifdef ES_MUTANT_ONE_CHILD
        if (r)
        {
            break;
        }
#endif
    }
    return reaped;
}

int es_counted_push(struct es_counted *q, unsigned value)
{
    if (!q || q->count > 2 || q->head > 1)
    {
        errno = EINVAL;
        return -1;
    }
    if (q->count == 2)
    {
        return 0;
    }
    q->slots[(q->head + q->count) % 2] = value;
    ++q->count;
    return 1;
}

int es_counted_pop(struct es_counted *q, unsigned *value)
{
    if (!q || !value || q->count > 2 || q->head > 1)
    {
        errno = EINVAL;
        return -1;
    }
    if (!q->count)
    {
        return 0;
    }
    *value = q->slots[q->head];
    q->head = (q->head + 1) % 2;
    --q->count;
    return 1;
}
