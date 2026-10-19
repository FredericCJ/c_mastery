#define _POSIX_C_SOURCE 200809L
#include "cl_posix.h"
#include <errno.h>
#ifndef CLP_MUTANT
#define CLP_MUTANT 0
#endif
static int cleanup_init(CLPQueue *q, int error) {
  if (q->initialized & 8u)
    (void)pthread_cond_destroy(&q->enrolled);
  if (q->initialized & 4u)
    (void)pthread_cond_destroy(&q->writable);
  if (q->initialized & 2u)
    (void)pthread_cond_destroy(&q->readable);
  if (q->initialized & 1u)
    (void)pthread_mutex_destroy(&q->mutex);
  q->initialized = 0;
  return error;
}
int clp_init(CLPQueue *q, unsigned fail) {
  int r;
  pthread_condattr_t attr;
  q->initialized = 0;
  q->active = 0;
  q->send_waiters = 0;
  q->recv_waiters = 0;
  cl_queue_init(&q->core, q->slots);
  if (fail == 1)
    return EAGAIN;
  r = pthread_mutex_init(&q->mutex, 0);
  if (r)
    return r;
  q->initialized = 1;
  r = pthread_condattr_init(&attr);
  if (r)
    return cleanup_init(q, r);
  r = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  if (!r)
    r = fail == 2 ? EAGAIN : pthread_cond_init(&q->readable, &attr);
  if (!r) {
    q->initialized |= 2u;
    r = fail == 3 ? EAGAIN : pthread_cond_init(&q->writable, &attr);
  }
  if (!r) {
    q->initialized |= 4u;
    r = fail == 4 ? EAGAIN : pthread_cond_init(&q->enrolled, &attr);
  }
  if (!r)
    q->initialized |= 8u;
  int a = pthread_condattr_destroy(&attr);
  if (!r)
    r = a;
  return r ? cleanup_init(q, r) : 0;
}
typedef struct {
  CLPQueue *q;
  unsigned *waiters;
  unsigned waiting;
} Cleanup;
static void unlock(void *arg) {
  volatile Cleanup *c = arg;
  if (c->waiting)
    (*c->waiters)--;
  if (CLP_MUTANT != 2)
    c->q->active--;
  (void)pthread_cond_broadcast(&c->q->enrolled);
  (void)pthread_mutex_unlock(&c->q->mutex);
}
static int wait_for(volatile Cleanup *c, pthread_cond_t *condition,
                    const struct timespec *deadline) {
  c->waiting = 1;
  (*c->waiters)++;
  (void)pthread_cond_broadcast(&c->q->enrolled);
  int r = deadline ? pthread_cond_timedwait(condition, &c->q->mutex, deadline)
                   : pthread_cond_wait(condition, &c->q->mutex);
  (*c->waiters)--;
  c->waiting = 0;
  return r;
}
int clp_send(CLPQueue *q, uint32_t p, uint32_t seq, const unsigned char *in,
             size_t n, const struct timespec *deadline) {
  int r = pthread_mutex_lock(&q->mutex);
  if (r)
    return r;
  /* The C cleanup macro may use setjmp-based unwinding. Preserve changes to
   * this automatic record across cancellation, including optimized builds. */
  volatile Cleanup c = {q, &q->send_waiters, 0};
  q->active++;
  pthread_cleanup_push(unlock, (void *)&c);
  for (;;) {
    enum cl_result x = cl_send(&q->core, p, seq, in, n);
    if (x == CL_OK) {
      r = 0;
      (void)pthread_cond_signal(&q->readable);
      break;
    }
    if (x != CL_FULL) {
      r = x == CL_CLOSED ? EPIPE : EINVAL;
      break;
    }
    r = wait_for(&c, &q->writable, deadline);
    /* This contract chooses timeout if a timed wait returns ETIMEDOUT;
     * it does not enqueue a coincident newly available item afterward. */
    if (r)
      break;
    if (CLP_MUTANT == 1) {
      r = 0;
      break;
    }
  }
  pthread_cleanup_pop(1);
  return r;
}
int clp_recv(CLPQueue *q, unsigned char *out, size_t cap, size_t *n,
             uint32_t *p, uint32_t *seq, uint64_t *ticket,
             const struct timespec *deadline) {
  if (!ticket)
    return EINVAL;
  int r = pthread_mutex_lock(&q->mutex);
  if (r)
    return r;
  volatile Cleanup c = {q, &q->recv_waiters, 0};
  q->active++;
  pthread_cleanup_push(unlock, (void *)&c);
  for (;;) {
    enum cl_result x = cl_recv(&q->core, out, cap, n, p, seq);
    if (x == CL_OK) {
      r = 0;
      *ticket = q->core.delivered;
      (void)pthread_cond_signal(&q->writable);
      break;
    }
    if (x != CL_EMPTY) {
      r = x == CL_CLOSED ? EPIPE : EMSGSIZE;
      break;
    }
    r = wait_for(&c, &q->readable, deadline);
    if (r)
      break;
  }
  pthread_cleanup_pop(1);
  return r;
}
int clp_close(CLPQueue *q, unsigned abort_queued) {
  int r = pthread_mutex_lock(&q->mutex);
  if (r)
    return r;
  if (abort_queued)
    (void)cl_abort(&q->core);
  else
    cl_close(&q->core);
  (void)pthread_cond_broadcast(&q->writable);
  (void)pthread_cond_broadcast(&q->readable);
  return pthread_mutex_unlock(&q->mutex);
}
int clp_destroy(CLPQueue *q) {
  if (!q->initialized)
    return EINVAL;
  int r = pthread_mutex_lock(&q->mutex);
  if (r)
    return r;
  unsigned busy = !q->core.closed || q->core.count || q->active ||
                  q->send_waiters || q->recv_waiters;
  (void)pthread_mutex_unlock(&q->mutex);
  if (busy && CLP_MUTANT != 3)
    return EBUSY;
  return cleanup_init(q, 0);
}
int clp_record_recover(pthread_mutex_t *m, CLPRecord *r, unsigned repair) {
  int error = pthread_mutex_lock(m);
  if (error == ENOTRECOVERABLE)
    return error;
  if (error != 0 && error != EOWNERDEAD)
    return error;
  if (error == EOWNERDEAD) {
    if (repair) {
      if (CLP_MUTANT != 4)
        r->value = r->committed;
      r->checksum = ~r->value;
      if (CLP_MUTANT != 5)
        error = pthread_mutex_consistent(m);
      else
        error = 0;
    } else {
      (void)pthread_mutex_unlock(m);
      return ENOTRECOVERABLE;
    }
  }
  if (!error && r->checksum != ~r->value)
    error = EPROTO;
  int unlock_error = pthread_mutex_unlock(m);
  return error ? error : unlock_error;
}
