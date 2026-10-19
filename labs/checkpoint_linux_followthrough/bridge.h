#ifndef P5_BRIDGE_H
#define P5_BRIDGE_H
#include "integration.h"
#include <pthread.h>
#include <time.h>
/* Original local teaching adapter. Owner serializes API calls, except the
   worker callback. All FAsync/ring and callback state is protected by mutex.
   Never copy an initialized bridge; external lifetime outlasts join/destroy. */
typedef struct {
 pthread_mutex_t mutex;pthread_cond_t changed;pthread_t worker;
 FAsync operation;algo_ring queue;uint32_t ticket;
 int fd,wake[2],initialized,started,cancelled,quiescent,hold_ack,release_ack;
 int io_error,cleanup_error;unsigned callbacks,discarded,joins;
 unsigned char bytes[32];size_t used;
} P5Bridge;
/* fd is borrowed only during init; duplicate ownership belongs to bridge.
   It must already be nonblocking. hold_ack is a deterministic laboratory gate. */
int p5_bridge_init(P5Bridge *,int fd,int hold_ack);
int p5_bridge_offer(P5Bridge *,int32_t);
int p5_bridge_begin(P5Bridge *,uint32_t *);
int p5_bridge_cancel(P5Bridge *,uint32_t);
/* 0 observed matching quiescence/discard,1 pending,-1 stale/invalid/error.
   A repeated or stale callback observation never increments release count. */
int p5_bridge_observe(P5Bridge *,uint32_t);
int p5_bridge_wait(P5Bridge *,const struct timespec *monotonic_deadline);
int p5_bridge_release_gate(P5Bridge *);
/* Join is separate from quiescence; deadline expiration never authorizes free. */
int p5_bridge_reap(P5Bridge *);
/* Once joined, discard any never-submitted items and close owned descriptors.
   A Linux late close error is reported, but its descriptor is never retried. */
int p5_bridge_destroy(P5Bridge *);
#endif
