#include "waiting.h"
#include <errno.h>
#include <stdlib.h>
static void ok(int e) {
  if (e)
    abort();
}
static void dispose_prefix(struct tw_queue *q, unsigned count, struct tw_init *r) {
  if (count >= 5) {
    ok(pthread_cond_destroy(&q->observer));
    r->destroyed++;
  }
  if (count >= 4) {
    ok(pthread_cond_destroy(&q->nonfull));
    r->destroyed++;
  }
  if (count >= 3) {
    ok(pthread_cond_destroy(&q->nonempty));
    r->destroyed++;
  }
  if (count >= 2) {
    ok(pthread_mutex_destroy(&q->mutex));
    r->destroyed++;
  }
  if (count >= 1) {
    ok(pthread_mutex_destroy(&q->root));
    r->destroyed++;
  }
}
int tw_queue_init(struct tw_queue *q, unsigned fail, struct tw_init *r) {
  if (fail > 5)
    return EINVAL;
  *r = (struct tw_init){0};
  pthread_condattr_t attr;
  int e = pthread_condattr_init(&attr);
  if (e)
    return e;
  e = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  if (e) {
    ok(pthread_condattr_destroy(&attr));
    return e;
  }
  for (unsigned stage = 1; stage <= 5; stage++) {
    if (stage == fail)
      e = EAGAIN;
    else if (stage == 1)
      e = pthread_mutex_init(&q->root, NULL);
    else if (stage == 2)
      e = pthread_mutex_init(&q->mutex, NULL);
    else
      e = pthread_cond_init(stage == 3   ? &q->nonempty
                            : stage == 4 ? &q->nonfull
                                         : &q->observer,
                            &attr);
    if (e) {
      r->failed_stage = stage;
      dispose_prefix(q, r->initialized, r);
      ok(pthread_condattr_destroy(&attr));
      return e;
    }
    r->initialized++;
  }
  ok(pthread_condattr_destroy(&attr));
  q->state = 0;
  q->references = 1;
  q->active = 0;
  q->initialized = 5;
  q->wait_entries = 0;
  q->head = q->tail = q->count = 0;
  q->timed_waits = q->deadline_changes = 0;
  q->last_deadline = (struct timespec){0};
  return 0;
}
int tw_ref(struct tw_queue *q) {
  int e = 0;
  ok(pthread_mutex_lock(&q->root));
  if (q->state || q->references == 0)
    e = EPIPE;
  else if (q->references == 16)
    e = EOVERFLOW;
  else
    q->references++;
  ok(pthread_mutex_unlock(&q->root));
  return e;
}
static void close_locked(struct tw_queue *q) {
  if (q->state == 0) {
    ok(pthread_mutex_lock(&q->mutex));
    q->state = 1;
    ok(pthread_cond_broadcast(&q->nonempty));
    ok(pthread_cond_broadcast(&q->nonfull));
    ok(pthread_mutex_unlock(&q->mutex));
  }
}
int tw_unref(struct tw_queue *q) {
  int e = 0;
  ok(pthread_mutex_lock(&q->root));
  if (!q->references)
    e = EINVAL;
  else {
    if (q->references == 1) {
      ok(pthread_mutex_lock(&q->mutex));
      if (q->count)
        e = EBUSY;
      ok(pthread_mutex_unlock(&q->mutex));
    }
    if (!e && --q->references == 0)
      close_locked(q);
  }
  ok(pthread_mutex_unlock(&q->root));
  return e;
}
int tw_close(struct tw_queue *q) {
  ok(pthread_mutex_lock(&q->root));
  close_locked(q);
  ok(pthread_mutex_unlock(&q->root));
  return 0;
}
int tw_retire(struct tw_queue *q) {
  int e = 0;
  ok(pthread_mutex_lock(&q->root));
  if (q->state != 1 || q->active || q->references)
    e = EBUSY;
  else {
    ok(pthread_mutex_lock(&q->mutex));
    int empty = q->count == 0;
    ok(pthread_mutex_unlock(&q->mutex));
    if (!empty)
      e = EBUSY;
    else {
      ok(pthread_cond_destroy(&q->observer));
      ok(pthread_cond_destroy(&q->nonfull));
      ok(pthread_cond_destroy(&q->nonempty));
      ok(pthread_mutex_destroy(&q->mutex));
      q->state = 2;
      q->initialized = 1;
    }
  }
  ok(pthread_mutex_unlock(&q->root));
  return e;
}
void tw_root_destroy(struct tw_queue *q) {
  if (q->state != 2 || q->references || q->active)
    abort();
  ok(pthread_mutex_destroy(&q->root));
  q->initialized = 0;
}
static int enter(struct tw_queue *q, int writing) {
  int e = 0;
  ok(pthread_mutex_lock(&q->root));
  if (q->state == 2 || !q->references || (writing && q->state))
    e = EPIPE;
  else
    q->active++;
  ok(pthread_mutex_unlock(&q->root));
  if (!e)
    ok(pthread_mutex_lock(&q->mutex));
  return e;
}
/* State is read with core held and changed with root+core held. */
static void leave(void *arg) {
  struct tw_queue *q = arg;
  int old;
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
  ok(pthread_mutex_unlock(&q->mutex));
  ok(pthread_mutex_lock(&q->root));
  if (!q->active)
    abort();
  q->active--;
  ok(pthread_mutex_unlock(&q->root));
}
static int wait_cond(struct tw_queue *q, pthread_cond_t *cond,
                     const struct timespec *deadline) {
  q->wait_entries++;
  ok(pthread_cond_broadcast(&q->observer));
  if (deadline) {
    if (q->timed_waits && (deadline->tv_sec != q->last_deadline.tv_sec ||
                           deadline->tv_nsec != q->last_deadline.tv_nsec))
      q->deadline_changes++;
    q->last_deadline = *deadline;
    q->timed_waits++;
    return pthread_cond_timedwait(cond, &q->mutex, deadline);
  }
  return pthread_cond_wait(cond, &q->mutex);
}
int tw_push(struct tw_queue *q, int value, const struct timespec *deadline) {
  if (deadline && (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000L))
    return EINVAL;
  int old, e;
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
  e = enter(q, 1);
  if (e) {
    ok(pthread_setcancelstate(old, NULL));
    return e;
  }
  pthread_cleanup_push(leave, q);
  ok(pthread_setcancelstate(old, NULL));
  while (q->count == 3 && !q->state) {
    e = wait_cond(q, &q->nonfull, deadline);
    if (e == ETIMEDOUT)
      break;
    ok(e);
  }
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
  if (q->state)
    e = EPIPE;
  else if (q->count == 3)
    e = ETIMEDOUT;
  else {
    q->values[q->tail] = value;
    q->tail = (q->tail + 1) % 3;
    q->count++;
    e = 0;
    ok(pthread_cond_broadcast(&q->nonempty));
  }
  pthread_cleanup_pop(1);
  ok(pthread_setcancelstate(old, NULL));
  return e;
}
int tw_pop(struct tw_queue *q, int *out, const struct timespec *deadline, tw_hook_fn hook,
           void *arg) {
  if (deadline && (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000L))
    return EINVAL;
  tw_hook_fn volatile pending = hook;
  int old, e;
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
  e = enter(q, 0);
  if (e) {
    ok(pthread_setcancelstate(old, NULL));
    return e;
  }
  pthread_cleanup_push(leave, q);
  ok(pthread_setcancelstate(old, NULL));
  while (!q->count && !q->state) {
    e = wait_cond(q, &q->nonempty, deadline);
    if (e == ETIMEDOUT)
      break;
    ok(e);
    if (pending) {
      int previous;
      ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &previous));
      ok(pthread_mutex_unlock(&q->mutex));
      pending(arg);
      ok(pthread_mutex_lock(&q->mutex));
      pending = NULL;
      ok(pthread_setcancelstate(previous, NULL));
    }
#ifdef TW_MUTANT5
    break;
#endif
  }
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
  if (q->count) {
    *out = q->values[q->head];
    q->head = (q->head + 1) % 3;
    q->count--;
    e = 0;
    ok(pthread_cond_broadcast(&q->nonfull));
  } else if (q->state)
    e = EPIPE;
  else
    e = ETIMEDOUT;
#ifdef TW_MUTANT4
  if (!e)
    *out += 1;
#endif
#ifdef TW_MUTANT6
  if (e == ETIMEDOUT)
    e = EPIPE;
#endif
  pthread_cleanup_pop(1);
  ok(pthread_setcancelstate(old, NULL));
  return e;
}
int tw_noise(struct tw_queue *q) {
  int old, e;
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
  e = enter(q, 0);
  if (!e) {
    ok(pthread_cond_broadcast(&q->nonempty));
    leave(q);
  }
  ok(pthread_setcancelstate(old, NULL));
  return e;
}
int tw_wait_entries(struct tw_queue *q, unsigned n) {
  int old, e;
  ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old));
  e = enter(q, 0);
  if (!e) {
    while (q->wait_entries < n)
      ok(pthread_cond_wait(&q->observer, &q->mutex));
    leave(q);
  }
  ok(pthread_setcancelstate(old, NULL));
  return e;
}
int tw_counts(struct tw_queue *q, unsigned *refs, unsigned *active, size_t *count) {
  ok(pthread_mutex_lock(&q->root));
  *refs = q->references;
  *active = q->active;
#ifdef TW_MUTANT3
  *refs = 0;
#endif
  if (q->state == 2)
    *count = 0;
  else {
    ok(pthread_mutex_lock(&q->mutex));
    *count = q->count;
    ok(pthread_mutex_unlock(&q->mutex));
  }
  ok(pthread_mutex_unlock(&q->root));
  return 0;
}
