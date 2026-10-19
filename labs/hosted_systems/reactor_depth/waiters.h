#ifndef CM_REACTOR_WAITERS_H
#define CM_REACTOR_WAITERS_H
#include <pthread.h>
#include <time.h>
enum RdWaitResult {
  RD_WAIT_PENDING,
  RD_WAIT_SUCCESS,
  RD_WAIT_STOP,
  RD_WAIT_TIMEOUT,
  RD_WAIT_PEER,
  RD_WAIT_ERROR
};
typedef struct {
  enum RdWaitResult result;
  unsigned char value;
} RdWait;
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t changed, enrollment;
  unsigned initialized, queued, stopped, peer_gone, waiters, entries;
  unsigned char value;
} RdPort;
/* Original capacity-one port, cooperative stop only. No pthread_cancel is
 * issued. All predicates, enrollment and terminal decisions share one mutex.
 * An already observed stop/peer or available item takes precedence over a
 * simultaneous timeout return. Once returned, the result never changes. This
 * is arbitration at mutex reacquisition, not a physical deadline certificate.
 */
int rd_port_init(RdPort *, unsigned initially_full, unsigned char value);
int rd_port_wait(RdPort *, unsigned producer, RdWait *,
                 const struct timespec *absolute_monotonic);
int rd_port_enrolled(RdPort *, unsigned entries,
                     const struct timespec *absolute_monotonic);
int rd_port_stop(RdPort *);
int rd_port_peer_gone(RdPort *);
int rd_port_notify(RdPort *);
/* After join of every user; refuses nonzero waiters. */
int rd_port_destroy(RdPort *);
#endif
