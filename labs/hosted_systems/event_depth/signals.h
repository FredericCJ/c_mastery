#ifndef CM_EVENT_SIGNALS_H
#define CM_EVENT_SIGNALS_H
/* C17 + selected POSIX/Linux profile; compile with -D_GNU_SOURCE -pthread. */
#include <signal.h>
#include <time.h>

enum
{
    ES_STOP = 1,
    ES_OTHER = 2,
    ES_CHILD = 4
};

struct es_signals
{
    int read_fd, write_fd, active;
    sigset_t previous_mask, managed, waiting_mask;
    struct sigaction previous_actions[3];
};

/* Exactly one live port in a single-threaded process. Other signal producers
 * must be quiescent before close. Open leaves the managed signals blocked. */
int es_open(struct es_signals *port);
int es_wait(struct es_signals *port, const struct timespec *relative);
unsigned es_take(struct es_signals *port, unsigned selected);
int es_close(struct es_signals *port);
/* now/deadline have nonnegative seconds, normalized nanoseconds. On timeout
 * write {0,0}; otherwise write the positive remaining interval. */
int es_remaining(struct timespec now, struct timespec deadline,
                 struct timespec *remaining);

struct es_wait
{
    int (*now)(void *, struct timespec *);
    /* Return >0 ready, 0 interval expired, -1 with errno on error. */
    int (*wait)(void *, const struct timespec *);
    void *context;
};

int es_wait_until(struct es_wait operations, struct timespec deadline);
int es_wait_fd(int fd, struct timespec deadline);
/* Reap only the supplied owned children. Zero slots already mean reaped.
 * Return number newly reaped or -1; partial progress remains in pids/statuses.
 */
int es_reap(pid_t *pids, int *statuses, unsigned count);

struct es_counted
{
    unsigned slots[2], head, count;
};

/* Ordinary serialized application queue, never invoked from a handler. */
int es_counted_push(struct es_counted *queue, unsigned value);
int es_counted_pop(struct es_counted *queue, unsigned *value);
#endif
