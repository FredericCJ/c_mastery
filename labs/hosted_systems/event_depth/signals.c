#include "signals.h"
#include <errno.h>

int es_counted_push(struct es_counted *q, unsigned v)
{
    (void)q;
    (void)v;
    return -1;
}

int es_counted_pop(struct es_counted *q, unsigned *v)
{
    (void)q;
    (void)v;
    return -1;
}

int es_open(struct es_signals *p)
{
    (void)p;
    errno = ENOSYS;
    return -1;
}

int es_wait(struct es_signals *p, const struct timespec *t)
{
    (void)p;
    (void)t;
    errno = ENOSYS;
    return -1;
}

unsigned es_take(struct es_signals *p, unsigned selected)
{
    (void)p;
    (void)selected;
    return 0;
}

int es_close(struct es_signals *p)
{
    (void)p;
    errno = ENOSYS;
    return -1;
}

int es_remaining(struct timespec n, struct timespec d, struct timespec *r)
{
    (void)n;
    (void)d;
    (void)r;
    return -1;
}

int es_wait_until(struct es_wait ops, struct timespec d)
{
    (void)ops;
    (void)d;
    errno = ENOSYS;
    return -1;
}

int es_wait_fd(int fd, struct timespec d)
{
    (void)fd;
    (void)d;
    errno = ENOSYS;
    return -1;
}

int es_reap(pid_t *p, int *s, unsigned n)
{
    (void)p;
    (void)s;
    (void)n;
    errno = ENOSYS;
    return -1;
}
