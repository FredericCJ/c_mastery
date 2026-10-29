#include "waiters.h"
#include <errno.h>
int rd_port_init(struct rd_port *p, unsigned full, unsigned char value) {
  if (!p || full > 1)
    return -1;
  *p = (struct rd_port){0};
  pthread_condattr_t attr;
  if (pthread_mutex_init(&p->mutex, 0))
    return -1;
  if (pthread_condattr_init(&attr)) {
    pthread_mutex_destroy(&p->mutex);
    return -1;
  }
  if (pthread_condattr_setclock(&attr, CLOCK_MONOTONIC)) {
    pthread_condattr_destroy(&attr);
    pthread_mutex_destroy(&p->mutex);
    return -1;
  }
  if (pthread_cond_init(&p->changed, &attr)) {
    pthread_condattr_destroy(&attr);
    pthread_mutex_destroy(&p->mutex);
    return -1;
  }
  if (pthread_cond_init(&p->enrollment, &attr)) {
    pthread_cond_destroy(&p->changed);
    pthread_condattr_destroy(&attr);
    pthread_mutex_destroy(&p->mutex);
    return -1;
  }
  if (pthread_condattr_destroy(&attr)) {
    pthread_cond_destroy(&p->enrollment);
    pthread_cond_destroy(&p->changed);
    pthread_mutex_destroy(&p->mutex);
    return -1;
  }
  p->queued = full;
  p->value = value;
  p->initialized = 1;
  return 0;
}
int rd_port_wait(struct rd_port *p, unsigned producer, struct rd_wait *w,
                 const struct timespec *end) {
  if (!p || !p->initialized || producer > 1 || !w || !end ||
      w->result != RD_WAIT_PENDING || end->tv_nsec < 0 ||
      end->tv_nsec >= 1000000000L)
    return -1;
  if (pthread_mutex_lock(&p->mutex))
    return -1;
  int last = 0;
  ++p->waiters;
  for (;;) {
    if (p->stopped) {
#ifdef RD_WAIT_MUTANT_STOP
      w->result = RD_WAIT_TIMEOUT;
#else
      w->result = RD_WAIT_STOP;
#endif
      break;
    }
    if (p->peer_gone) {
      w->result = RD_WAIT_PEER;
      break;
    }
    if (p->queued != (producer != 0)) {
      if (producer) {
        p->value = w->value;
        p->queued = 1;
      } else {
        w->value = p->value;
        p->queued = 0;
      }
      w->result = RD_WAIT_SUCCESS;
      pthread_cond_broadcast(&p->changed);
      break;
    }
    if (last == ETIMEDOUT) {
      w->result = RD_WAIT_TIMEOUT;
      break;
    }
    if (last) {
      w->result = RD_WAIT_ERROR;
      break;
    }
    ++p->entries;
    pthread_cond_broadcast(&p->enrollment);
    last = pthread_cond_timedwait(&p->changed, &p->mutex, end);
#ifdef RD_WAIT_MUTANT_WAKE
    if (!last) {
      w->result = RD_WAIT_SUCCESS;
      break;
    }
#endif
  }
  --p->waiters;
  int result = pthread_mutex_unlock(&p->mutex);
  return result ? -1 : 0;
}
int rd_port_enrolled(struct rd_port *p, unsigned entries, const struct timespec *end) {
  if (!p || !p->initialized || !end || pthread_mutex_lock(&p->mutex))
    return -1;
  int result = 0;
  while (p->entries < entries && !result)
    result = pthread_cond_timedwait(&p->enrollment, &p->mutex, end);
  int unlocked = pthread_mutex_unlock(&p->mutex);
  return result || unlocked ? -1 : 0;
}
static int notify(struct rd_port *p, unsigned reason) {
  if (!p || !p->initialized || pthread_mutex_lock(&p->mutex))
    return -1;
  if (reason == 1)
    p->stopped = 1;
  if (reason == 2)
    p->peer_gone = 1;
  int r = pthread_cond_broadcast(&p->changed),
      u = pthread_mutex_unlock(&p->mutex);
  return r || u ? -1 : 0;
}
int rd_port_stop(struct rd_port *p) { return notify(p, 1); }
int rd_port_peer_gone(struct rd_port *p) { return notify(p, 2); }
int rd_port_notify(struct rd_port *p) { return notify(p, 0); }
int rd_port_destroy(struct rd_port *p) {
  if (!p || !p->initialized || p->waiters)
    return -1;
  int a = pthread_cond_destroy(&p->changed),
      b = pthread_cond_destroy(&p->enrollment),
      c = pthread_mutex_destroy(&p->mutex);
  if (a || b || c)
    return -1;
  p->initialized = 0;
  return 0;
}
