#ifndef CM_EVENT_SIGNALS_H
#define CM_EVENT_SIGNALS_H
/* C17 + selected POSIX/Linux profile; compile with -D_GNU_SOURCE -pthread. */
#include <signal.h>
#include <time.h>
enum { ES_STOP = 1, ES_OTHER = 2, ES_CHILD = 4 };
typedef struct {
  int read_fd, write_fd, active;
  sigset_t previous_mask, managed, waiting_mask;
  struct sigaction previous_actions[3];
} EsSignals;
/* Exactly one live port in a single-threaded process. Other signal producers
 * must be quiescent before close. Open leaves the managed signals blocked. */
int es_open(EsSignals *port);
int es_wait(EsSignals *port, const struct timespec *relative);
unsigned es_take(EsSignals *port, unsigned selected);
int es_close(EsSignals *port);
/* now/deadline have nonnegative seconds, normalized nanoseconds. On timeout
 * write {0,0}; otherwise write the positive remaining interval. */
int es_remaining(struct timespec now, struct timespec deadline,
                 struct timespec *remaining);
typedef struct {
  int (*now)(void *, struct timespec *);
  /* Return >0 ready, 0 interval expired, -1 with errno on error. */
  int (*wait)(void *, const struct timespec *);
  void *context;
} EsWait;
int es_wait_until(EsWait operations, struct timespec deadline);
int es_wait_fd(int fd, struct timespec deadline);
/* Reap only the supplied owned children. Zero slots already mean reaped.
 * Return number newly reaped or -1; partial progress remains in pids/statuses.
 */
int es_reap(pid_t *pids, int *statuses, unsigned count);
typedef struct {
  unsigned slots[2], head, count;
} EsCounted;
/* Ordinary serialized application queue, never invoked from a handler. */
int es_counted_push(EsCounted *queue, unsigned value);
int es_counted_pop(EsCounted *queue, unsigned *value);
#endif
