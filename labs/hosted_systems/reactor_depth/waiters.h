#ifndef CM_REACTOR_WAITERS_H
#define CM_REACTOR_WAITERS_H
#include <pthread.h>
#include <time.h>
enum rd_wait_result {
  RD_WAIT_PENDING,
  RD_WAIT_SUCCESS,
  RD_WAIT_STOP,
  RD_WAIT_TIMEOUT,
  RD_WAIT_PEER,
  RD_WAIT_ERROR
};
struct rd_wait {
  enum rd_wait_result result;
  unsigned char value;
};
struct rd_port {
  pthread_mutex_t mutex;
  pthread_cond_t changed, enrollment;
  unsigned initialized, queued, stopped, peer_gone, waiters, entries;
  unsigned char value;
};
/* Original capacity-one port, cooperative stop only. No pthread_cancel is
 * issued. All predicates, enrollment and terminal decisions share one mutex.
 * An already observed stop/peer or available item takes precedence over a
 * simultaneous timeout return. Once returned, the result never changes. This
 * is arbitration at mutex reacquisition, not a physical deadline certificate.
 */
int rd_port_init(struct rd_port *, unsigned initially_full, unsigned char value);
int rd_port_wait(struct rd_port *, unsigned producer, struct rd_wait *,
                 const struct timespec *absolute_monotonic);
int rd_port_enrolled(struct rd_port *, unsigned entries,
                     const struct timespec *absolute_monotonic);
int rd_port_stop(struct rd_port *);
int rd_port_peer_gone(struct rd_port *);
int rd_port_notify(struct rd_port *);
/* After join of every user; refuses nonzero waiters. */
int rd_port_destroy(struct rd_port *);
#endif
