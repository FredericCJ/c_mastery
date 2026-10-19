#include "cmx.h"
#define BIT(t) (UINT32_C(1) << (t))
static int valid(const cmx_kernel *k, unsigned t) {
  return t < CMX_TASKS && k->task[t].state != CMX_UNUSED &&
         k->task[t].state != CMX_EXITED;
}
static int callable(const cmx_kernel *k, unsigned t) {
  return valid(k, t) &&
         (k->task[t].state == CMX_READY || k->task[t].state == CMX_RUNNING);
}
static int due(uint32_t now, cmx_deadline d) {
  return !d.infinite && (uint32_t)(now - d.at) < UINT32_C(0x80000000);
}
static int before(uint32_t a, uint32_t b) {
  return a != b && (uint32_t)(b - a) < UINT32_C(0x80000000);
}
static void ready(cmx_kernel *k, unsigned t, enum cmx_result r) {
  cmx_task *p = &k->task[t];
  p->state = CMX_READY;
  p->wait = CMX_W_NONE;
  p->object = 0;
  p->aux = 0;
  p->flags = 0;
  p->result = r;
}
static enum cmx_result wait_on(cmx_kernel *k, unsigned t, enum cmx_wait w,
                               void *o, cmx_deadline d) {
  if (!valid(k, t) || k->task[t].state == CMX_WAITING)
    return CMX_INVALID;
  if (k->sequence == UINT32_MAX)
    return CMX_FULL;
  if (due(k->now, d))
    return CMX_TIMEOUT;
  cmx_task *p = &k->task[t];
  p->state = CMX_WAITING;
  p->wait = w;
  p->object = o;
  p->deadline = d;
  p->wait_sequence = k->sequence++;
  p->result = CMX_BLOCKED;
  p->pending = CMX_OK;
  p->flags = 0;
  return CMX_BLOCKED;
}
static int first(const cmx_kernel *k, void *o, enum cmx_wait w) {
  int found = CMX_NONE;
  for (unsigned i = 0; i < CMX_TASKS; i++)
    if (k->task[i].state == CMX_WAITING && k->task[i].object == o &&
        k->task[i].wait == w)
      if (found < 0 || k->task[i].wait_sequence < k->task[found].wait_sequence)
        found = (int)i;
  return found;
}
static void wake_all(cmx_kernel *k, void *o, enum cmx_wait w,
                     enum cmx_result r) {
  int t;
  while ((t = first(k, o, w)) >= 0)
    ready(k, (unsigned)t, r);
}
static void priority(cmx_kernel *k) {
  for (unsigned i = 0; i < CMX_TASKS; i++)
    k->task[i].priority = k->task[i].base_priority;
  for (unsigned m = 0; m < k->mutex_count; m++) {
    cmx_mutex *p = k->mutex[m];
    if (p->owner >= 0 && p->use_ceiling &&
        p->ceiling < k->task[p->owner].priority)
      k->task[p->owner].priority = p->ceiling;
  }
  for (unsigned pass = 0; pass < CMX_TASKS; pass++) {
    unsigned changed = 0;
    for (unsigned i = 0; i < CMX_TASKS; i++)
      if (k->task[i].state == CMX_WAITING && k->task[i].wait == CMX_W_MUTEX) {
        cmx_mutex *m = k->task[i].object;
        if (m->owner >= 0 && k->task[m->owner].priority > k->task[i].priority) {
          k->task[m->owner].priority = k->task[i].priority;
          changed = 1;
        }
      }
    if (!changed)
      break;
  }
}
void cmx_init(cmx_kernel *k, enum cmx_policy p) {
  *k = (cmx_kernel){0};
  k->policy = p;
  k->current = CMX_NONE;
  for (unsigned i = 0; i < CMX_TASKS; i++)
    k->task[i].joiner = CMX_NONE;
}
int cmx_task_add(cmx_kernel *k, unsigned p, uint32_t d, uintptr_t low,
                 size_t size) {
  if (p > 255 || size < 128 || (low & 7u) || (size & 7u) ||
      size > UINTPTR_MAX - low)
    return CMX_NONE;
  for (unsigned i = 0; i < CMX_TASKS; i++)
    if (k->task[i].state != CMX_UNUSED && low < k->task[i].stack_high &&
        low + size > k->task[i].stack_low)
      return CMX_NONE;
  for (unsigned i = 0; i < CMX_TASKS; i++)
    if (k->task[i].state == CMX_UNUSED) {
      unsigned g = k->task[i].generation;
      k->task[i] = (cmx_task){0};
      k->task[i].generation = g;
      k->task[i].state = CMX_READY;
      k->task[i].base_priority = p;
      k->task[i].priority = p;
      k->task[i].job_deadline = d;
      k->task[i].stack_low = low;
      k->task[i].stack_high = low + size;
      k->task[i].joiner = CMX_NONE;
      return (int)i;
    }
  return CMX_NONE;
}
int cmx_schedule(cmx_kernel *k) {
  if (k->current >= 0 && k->task[k->current].state == CMX_RUNNING)
    k->task[k->current].state = CMX_READY;
  unsigned start = k->current < 0 ? 0 : ((unsigned)k->current + 1) % CMX_TASKS;
  int best = CMX_NONE;
  for (unsigned n = 0; n < CMX_TASKS; n++) {
    unsigned i = (start + n) % CMX_TASKS;
    if (k->task[i].state != CMX_READY)
      continue;
    if (best < 0 ||
        (k->policy == CMX_FIXED &&
         k->task[i].priority < k->task[best].priority) ||
        (k->policy == CMX_EDF &&
         before(k->task[i].job_deadline, k->task[best].job_deadline)))
      best = (int)i;
  }
  k->current = best;
  if (best >= 0)
    k->task[best].state = CMX_RUNNING;
  return best;
}
enum cmx_result cmx_sleep(cmx_kernel *k, unsigned t, cmx_deadline d) {
  return wait_on(k, t, CMX_W_SLEEP, 0, d);
}
static void relock(cmx_kernel *k, unsigned t, enum cmx_result result) {
  cmx_mutex *m = k->task[t].aux;
  k->task[t].wait = CMX_W_MUTEX;
  k->task[t].object = m;
  k->task[t].aux = 0;
  k->task[t].deadline = cmx_forever();
  k->task[t].pending = result;
  k->task[t].flags = UINT32_C(0x80000000);
  if (m->owner < 0) {
    m->owner = (int)t;
    ready(k, t, result);
  }
  priority(k);
}
static void rw_admit(cmx_kernel *k, cmx_rwlock *r) {
  if (r->writer >= 0 || r->closed)
    return;
  int n = first(k, r, CMX_W_WRITE);
  if (n >= 0) {
    if (r->readers)
      return;
    r->writer = n;
    k->task[n].owned_sync++;
    ready(k, (unsigned)n, CMX_OK);
  } else {
    while ((n = first(k, r, CMX_W_READ)) >= 0) {
      r->readers |= BIT((unsigned)n);
      k->task[n].owned_sync++;
      ready(k, (unsigned)n, CMX_OK);
    }
  }
}
static void abandon(cmx_kernel *k, unsigned t, enum cmx_result result) {
  cmx_task *p = &k->task[t];
  void *o = p->object;
  enum cmx_wait w = p->wait;
  if (w == CMX_W_MUTEX && (p->flags & UINT32_C(0x80000000))) {
    p->pending = result;
    return;
  }
  if (w == CMX_W_COND) {
    relock(k, t, result);
    return;
  }
  if (w == CMX_W_BARRIER) {
    cmx_barrier *b = o;
    b->broken = 1;
    b->arrived = 0;
    b->generation++;
    wake_all(k, b, CMX_W_BARRIER, CMX_BROKEN);
    p->result = result;
    return;
  }
  if (w == CMX_W_JOIN && p->arg < CMX_TASKS && k->task[p->arg].joiner == (int)t)
    k->task[p->arg].joiner = CMX_NONE;
  ready(k, t, result);
  if (w == CMX_W_WRITE)
    rw_admit(k, o);
  priority(k);
}
void cmx_tick(cmx_kernel *k, uint32_t now) {
  k->now = now;
  for (unsigned i = 0; i < CMX_TASKS; i++)
    if (k->task[i].state == CMX_WAITING && due(now, k->task[i].deadline))
      abandon(k, i, k->task[i].wait == CMX_W_SLEEP ? CMX_OK : CMX_TIMEOUT);
}
enum cmx_result cmx_cancel(cmx_kernel *k, unsigned t) {
  if (!valid(k, t) || k->task[t].state != CMX_WAITING)
    return CMX_INVALID;
  abandon(k, t, CMX_CLOSED);
  return CMX_OK;
}
enum cmx_result cmx_exit(cmx_kernel *k, unsigned t, int status) {
  if (!valid(k, t))
    return CMX_INVALID;
  /* Owner death is refused: caller must release owned mutex/once/rw resources.
   * A forced-kill/recovery protocol is intentionally a separate mechanism. */
  for (unsigned m = 0; m < k->mutex_count; m++)
    if (k->mutex[m]->owner == (int)t)
      return CMX_NOT_OWNER;
  if (k->task[t].owned_sync)
    return CMX_NOT_OWNER;
  if (k->task[t].state == CMX_WAITING)
    return CMX_INVALID;
  k->task[t].state = CMX_EXITED;
  k->task[t].exit_code = status;
  if (k->task[t].joiner >= 0) {
    unsigned j = (unsigned)k->task[t].joiner;
    k->task[j].value = (uint32_t)status;
    ready(k, j, CMX_OK);
  }
  return CMX_OK;
}
enum cmx_result cmx_join(cmx_kernel *k, unsigned t, unsigned target,
                         cmx_deadline d) {
  if (!callable(k, t) || target >= CMX_TASKS || t == target ||
      k->task[target].state == CMX_UNUSED)
    return CMX_INVALID;
  if (k->task[target].joiner >= 0 && k->task[target].joiner != (int)t)
    return CMX_INVALID;
  if (k->task[target].state == CMX_EXITED) {
    k->task[t].value = (uint32_t)k->task[target].exit_code;
    k->task[target].joiner = (int)t;
    return CMX_OK;
  }
  enum cmx_result r = wait_on(k, t, CMX_W_JOIN, &k->task[target], d);
  if (r == CMX_BLOCKED) {
    k->task[t].arg = target;
    k->task[target].joiner = (int)t;
  }
  return r;
}
enum cmx_result cmx_reap(cmx_kernel *k, unsigned t) {
  if (t >= CMX_TASKS || k->task[t].state != CMX_EXITED || k->current == (int)t)
    return CMX_INVALID;
  unsigned g = k->task[t].generation;
  if (g == UINT32_MAX)
    return CMX_FULL;
  k->task[t] = (cmx_task){0};
  k->task[t].generation = g + 1;
  k->task[t].joiner = CMX_NONE;
  return CMX_OK;
}
void cmx_sem_init(cmx_sem *s, unsigned n, unsigned max) {
  *s = (cmx_sem){n, max, 0};
  if (!max || n > max)
    s->closed = 1;
}
enum cmx_result cmx_latch_wait(cmx_kernel *k, cmx_latch *l, unsigned t,
                               cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (l->signaled) {
    l->signaled = 0;
    return CMX_OK;
  }
  return wait_on(k, t, CMX_W_LATCH, l, d);
}
void cmx_latch_signal(cmx_kernel *k, cmx_latch *l) {
  int t = first(k, l, CMX_W_LATCH);
  if (t >= 0)
    ready(k, (unsigned)t, CMX_OK);
  else
    l->signaled = 1;
}
enum cmx_result cmx_sem_take(cmx_kernel *k, cmx_sem *s, unsigned t,
                             cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (s->closed)
    return CMX_CLOSED;
  if (s->count) {
    s->count--;
    return CMX_OK;
  }
  return wait_on(k, t, CMX_W_SEM, s, d);
}
enum cmx_result cmx_sem_give(cmx_kernel *k, cmx_sem *s) {
  if (s->closed)
    return CMX_CLOSED;
  int t = first(k, s, CMX_W_SEM);
  if (t >= 0) {
    ready(k, (unsigned)t, CMX_OK);
    return CMX_OK;
  }
  if (s->count == s->maximum)
    return CMX_FULL;
  s->count++;
  return CMX_OK;
}
enum cmx_result cmx_sem_close(cmx_kernel *k, cmx_sem *s) {
  s->closed = 1;
  wake_all(k, s, CMX_W_SEM, CMX_CLOSED);
  return CMX_OK;
}
enum cmx_result cmx_mutex_init(cmx_kernel *k, cmx_mutex *m, unsigned ceiling,
                               unsigned use) {
  if (k->mutex_count == CMX_MUTEXES || ceiling > 255)
    return CMX_FULL;
  for (unsigned i = 0; i < k->mutex_count; i++)
    if (k->mutex[i] == m)
      return CMX_INVALID;
  *m = (cmx_mutex){CMX_NONE, ceiling, use != 0, 0};
  k->mutex[k->mutex_count++] = m;
  return CMX_OK;
}
enum cmx_result cmx_mutex_lock(cmx_kernel *k, cmx_mutex *m, unsigned t,
                               cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (m->closed)
    return CMX_CLOSED;
  if (m->owner == (int)t)
    return CMX_DEADLOCK;
  if (m->use_ceiling && k->task[t].base_priority < m->ceiling)
    return CMX_INVALID;
  /* Immediate ceiling: ownership raises priority; this is not original PCP. */
  if (m->owner < 0) {
    m->owner = (int)t;
    priority(k);
    return CMX_OK;
  }
  int owner = m->owner;
  for (unsigned n = 0; n < CMX_TASKS; n++) {
    if (owner == (int)t)
      return CMX_DEADLOCK;
    cmx_task *p = &k->task[owner];
    if (p->state != CMX_WAITING || p->wait != CMX_W_MUTEX)
      break;
    owner = ((cmx_mutex *)p->object)->owner;
    if (owner < 0)
      break;
  }
  enum cmx_result r = wait_on(k, t, CMX_W_MUTEX, m, d);
  priority(k);
  return r;
}
enum cmx_result cmx_mutex_unlock(cmx_kernel *k, cmx_mutex *m, unsigned t) {
  if (m->owner != (int)t)
    return CMX_NOT_OWNER;
  int n = first(k, m, CMX_W_MUTEX);
  m->owner = n;
  if (n >= 0)
    ready(k, (unsigned)n, k->task[n].pending);
  priority(k);
  return CMX_OK;
}
enum cmx_result cmx_mutex_close(cmx_kernel *k, cmx_mutex *m) {
  if (m->owner >= 0)
    return CMX_NOT_OWNER;
  for (unsigned t = 0; t < CMX_TASKS; t++)
    if (k->task[t].state == CMX_WAITING &&
        (k->task[t].object == m || k->task[t].aux == m))
      return CMX_BLOCKED;
  for (unsigned i = 0; i < k->mutex_count; i++)
    if (k->mutex[i] == m) {
      k->mutex[i] = k->mutex[--k->mutex_count];
      m->closed = 1;
      return CMX_OK;
    }
  return CMX_INVALID;
}
enum cmx_result cmx_cond_wait(cmx_kernel *k, cmx_cond *c, cmx_mutex *m,
                              unsigned t, cmx_deadline d) {
  if (c->closed)
    return CMX_CLOSED;
  if (m->owner != (int)t)
    return CMX_NOT_OWNER;
  enum cmx_result r = wait_on(k, t, CMX_W_COND, c, d);
  if (r != CMX_BLOCKED)
    return r;
  k->task[t].aux = m;
  cmx_mutex_unlock(k, m, t);
  return r;
}
enum cmx_result cmx_cond_signal(cmx_kernel *k, cmx_cond *c,
                                unsigned broadcast) {
  if (c->closed)
    return CMX_CLOSED;
  int t;
  while ((t = first(k, c, CMX_W_COND)) >= 0) {
    relock(k, (unsigned)t, CMX_OK);
    if (!broadcast)
      break;
  }
  return CMX_OK;
}
enum cmx_result cmx_cond_close(cmx_kernel *k, cmx_cond *c) {
  c->closed = 1;
  int t;
  while ((t = first(k, c, CMX_W_COND)) >= 0)
    relock(k, (unsigned)t, CMX_CLOSED);
  return CMX_OK;
}
void cmx_queue_init(cmx_queue *q, unsigned capacity) {
  *q = (cmx_queue){0};
  q->capacity = capacity;
  q->closed = capacity > CMX_QUEUE;
}
enum cmx_result cmx_queue_send(cmx_kernel *k, cmx_queue *q, unsigned t,
                               uint32_t value, cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (q->closed)
    return CMX_CLOSED;
  int r = first(k, q, CMX_W_RECV);
  if (r >= 0) {
    k->task[r].value = value;
    ready(k, (unsigned)r, CMX_OK);
    return CMX_OK;
  }
  if (q->count < q->capacity) {
    q->data[(q->head + q->count) % CMX_QUEUE] = value;
    q->count++;
    return CMX_OK;
  }
  enum cmx_result x = wait_on(k, t, CMX_W_SEND, q, d);
  if (x == CMX_BLOCKED)
    k->task[t].value = value;
  return x;
}
enum cmx_result cmx_queue_recv(cmx_kernel *k, cmx_queue *q, unsigned t,
                               cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  int s = first(k, q, CMX_W_SEND);
  if (q->count) {
    k->task[t].value = q->data[q->head];
    q->head = (q->head + 1) % CMX_QUEUE;
    q->count--;
    if (s >= 0) {
      q->data[(q->head + q->count) % CMX_QUEUE] = k->task[s].value;
      q->count++;
      ready(k, (unsigned)s, CMX_OK);
    }
    return CMX_OK;
  }
  if (s >= 0) {
    k->task[t].value = k->task[s].value;
    ready(k, (unsigned)s, CMX_OK);
    return CMX_OK;
  }
  if (q->closed)
    return CMX_CLOSED;
  return wait_on(k, t, CMX_W_RECV, q, d);
}
enum cmx_result cmx_queue_close(cmx_kernel *k, cmx_queue *q) {
  q->closed = 1;
  wake_all(k, q, CMX_W_SEND, CMX_CLOSED);
  wake_all(k, q, CMX_W_RECV, CMX_CLOSED);
  return CMX_OK;
}
enum cmx_result cmx_queue_abort(cmx_kernel *k, cmx_queue *q) {
  q->head = 0;
  q->count = 0;
  return cmx_queue_close(k, q);
}
static int event_match(cmx_event *e, uint32_t mask, unsigned all) {
  return all ? (e->bits & mask) == mask : (e->bits & mask) != 0;
}
enum cmx_result cmx_event_wait(cmx_kernel *k, cmx_event *e, unsigned t,
                               uint32_t mask, unsigned all, unsigned clear,
                               cmx_deadline d) {
  if (!callable(k, t) || !mask)
    return CMX_INVALID;
  if (e->closed)
    return CMX_CLOSED;
  if (event_match(e, mask, all)) {
    k->task[t].value = e->bits & mask;
    if (clear)
      e->bits &= ~k->task[t].value;
    return CMX_OK;
  }
  enum cmx_result r = wait_on(k, t, CMX_W_EVENT, e, d);
  if (r == CMX_BLOCKED) {
    k->task[t].arg = mask;
    k->task[t].flags = (all ? 1u : 0u) | (clear ? 2u : 0u);
  }
  return r;
}
void cmx_event_set(cmx_kernel *k, cmx_event *e, uint32_t bits) {
  if (e->closed)
    return;
  e->bits |= bits; /* FIFO consumption: clear-on-consume is not broadcast. */
  for (unsigned n = 0; n < CMX_TASKS; n++) {
    int best = CMX_NONE;
    for (unsigned i = 0; i < CMX_TASKS; i++) {
      cmx_task *p = &k->task[i];
      if (p->state == CMX_WAITING && p->wait == CMX_W_EVENT && p->object == e &&
          event_match(e, p->arg, p->flags & 1u))
        if (best < 0 || p->wait_sequence < k->task[best].wait_sequence)
          best = (int)i;
    }
    if (best < 0)
      break;
    cmx_task *p = &k->task[best];
    p->value = e->bits & p->arg;
    if (p->flags & 2u)
      e->bits &= ~p->value;
    ready(k, (unsigned)best, CMX_OK);
  }
}
void cmx_event_close(cmx_kernel *k, cmx_event *e) {
  e->closed = 1;
  wake_all(k, e, CMX_W_EVENT, CMX_CLOSED);
}
enum cmx_result cmx_notify(cmx_kernel *k, unsigned t, uint32_t count) {
  if (!valid(k, t) || !count)
    return CMX_INVALID;
  cmx_task *p = &k->task[t];
  if (count > UINT32_MAX - p->notifications)
    return CMX_FULL;
  p->notifications += count;
  if (p->state == CMX_WAITING && p->wait == CMX_W_NOTIFY) {
    p->notifications--;
    p->value = 1;
    ready(k, t, CMX_OK);
  }
  return CMX_OK;
}
enum cmx_result cmx_notify_take(cmx_kernel *k, unsigned t, cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (k->task[t].notifications) {
    k->task[t].notifications--;
    k->task[t].value = 1;
    return CMX_OK;
  }
  return wait_on(k, t, CMX_W_NOTIFY, &k->task[t], d);
}
void cmx_barrier_init(cmx_barrier *b, unsigned parties) {
  *b = (cmx_barrier){parties, 0, 0, parties == 0 || parties > CMX_TASKS};
}
enum cmx_result cmx_barrier_wait(cmx_kernel *k, cmx_barrier *b, unsigned t,
                                 cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (b->broken)
    return CMX_BROKEN;
  if (b->arrived + 1 == b->parties) {
    b->arrived = 0;
    b->generation++;
    wake_all(k, b, CMX_W_BARRIER, CMX_OK);
    return CMX_OK;
  }
  enum cmx_result r = wait_on(k, t, CMX_W_BARRIER, b, d);
  if (r == CMX_BLOCKED)
    b->arrived++;
  return r;
}
void cmx_once_init(cmx_once *o) { *o = (cmx_once){0, CMX_NONE}; }
enum cmx_result cmx_once_enter(cmx_kernel *k, cmx_once *o, unsigned t,
                               cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (o->state == 2)
    return CMX_OK;
  if (o->owner == (int)t)
    return CMX_DEADLOCK;
  if (o->state == 0) {
    o->owner = (int)t;
    o->state = 1;
    k->task[t].owned_sync++;
    return CMX_RETRY;
  }
  return wait_on(k, t, CMX_W_ONCE, o, d);
}
enum cmx_result cmx_once_finish(cmx_kernel *k, cmx_once *o, unsigned t,
                                unsigned success) {
  if (o->owner != (int)t || o->state != 1)
    return CMX_NOT_OWNER;
  o->owner = CMX_NONE;
  k->task[t].owned_sync--;
  o->state = success ? 2 : 0;
  wake_all(k, o, CMX_W_ONCE, success ? CMX_OK : CMX_RETRY);
  return CMX_OK;
}
void cmx_rw_init(cmx_rwlock *r) { *r = (cmx_rwlock){0, CMX_NONE, 0}; }
enum cmx_result cmx_rw_read(cmx_kernel *k, cmx_rwlock *r, unsigned t,
                            cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (r->closed)
    return CMX_CLOSED;
  if ((r->readers & BIT(t)) || r->writer == (int)t)
    return CMX_DEADLOCK;
  if (r->writer < 0 && first(k, r, CMX_W_WRITE) < 0) {
    r->readers |= BIT(t);
    k->task[t].owned_sync++;
    return CMX_OK;
  }
  return wait_on(k, t, CMX_W_READ, r, d);
}
enum cmx_result cmx_rw_write(cmx_kernel *k, cmx_rwlock *r, unsigned t,
                             cmx_deadline d) {
  if (!callable(k, t))
    return CMX_INVALID;
  if (r->closed)
    return CMX_CLOSED;
  if ((r->readers & BIT(t)) || r->writer == (int)t)
    return CMX_DEADLOCK;
  if (r->writer < 0 && !r->readers) {
    r->writer = (int)t;
    k->task[t].owned_sync++;
    return CMX_OK;
  }
  return wait_on(k, t, CMX_W_WRITE, r, d);
}
enum cmx_result cmx_rw_unlock(cmx_kernel *k, cmx_rwlock *r, unsigned t) {
  if (t >= CMX_TASKS)
    return CMX_INVALID;
  if (r->writer == (int)t)
    r->writer = CMX_NONE;
  else if (r->readers & BIT(t))
    r->readers &= ~BIT(t);
  else
    return CMX_NOT_OWNER;
  k->task[t].owned_sync--;
  rw_admit(k, r);
  return CMX_OK;
}
enum cmx_result cmx_rw_close(cmx_kernel *k, cmx_rwlock *r) {
  if (r->writer >= 0 || r->readers)
    return CMX_NOT_OWNER;
  r->closed = 1;
  wake_all(k, r, CMX_W_READ, CMX_CLOSED);
  wake_all(k, r, CMX_W_WRITE, CMX_CLOSED);
  return CMX_OK;
}
int cmx_invariant(const cmx_kernel *k) {
  unsigned running = 0;
  for (unsigned i = 0; i < CMX_TASKS; i++) {
    const cmx_task *t = &k->task[i];
    if (t->state == CMX_RUNNING) {
      running++;
      if (k->current != (int)i)
        return 0;
    }
    if ((t->state == CMX_WAITING) != (t->wait != CMX_W_NONE))
      return 0;
    if (t->priority > t->base_priority)
      return 0;
  }
  return running <= 1;
}
enum cmx_result cmx_response(const cmx_job *j, unsigned count, unsigned index,
                             uint32_t *out) {
  if (!j || !out || index >= count || count > CMX_TASKS)
    return CMX_INVALID;
  for (unsigned i = 0; i < count; i++)
    if (!j[i].period || j[i].period > 1000000u || j[i].cost > 1000000u ||
        j[i].jitter > 1000000u || j[i].blocking > 1000000u || !j[i].deadline ||
        j[i].deadline > 1000000u)
      return CMX_INVALID;
  uint64_t r = (uint64_t)j[index].cost + j[index].blocking;
  for (unsigned n = 0; n < 128; n++) {
    uint64_t next = (uint64_t)j[index].cost + j[index].blocking;
    for (unsigned h = 0; h < index; h++) {
      if (!j[h].period)
        return CMX_INVALID;
      /* Validated bounds make this numerator < 4,000,000. Native 32-bit
       * division avoids importing an ARM 64-bit compiler runtime helper. */
      uint64_t arrivals =
          (uint32_t)(r + j[h].jitter + j[h].period - 1) / j[h].period;
      next += arrivals * j[h].cost;
      if (next > UINT32_MAX)
        return CMX_FULL;
    }
    if (next == r) {
      *out = (uint32_t)r;
      return r <= j[index].deadline ? CMX_OK : CMX_TIMEOUT;
    }
    if (next > j[index].deadline) {
      *out = (uint32_t)next;
      return CMX_TIMEOUT;
    }
    r = next;
  }
  return CMX_FULL;
}
enum cmx_result cmx_edf_admit(const cmx_job *j, unsigned count,
                              uint32_t horizon) {
  if (!j || !count || count > CMX_TASKS || !horizon || horizon > 1000000)
    return CMX_INVALID;
  for (unsigned i = 0; i < count; i++)
    if (!j[i].period || !j[i].deadline || j[i].deadline > j[i].period ||
        j[i].jitter || j[i].blocking)
      return CMX_INVALID;
  /* Exact integer demand-bound check only over the explicitly finite horizon.
   */
  for (unsigned i = 0; i < count; i++)
    for (uint32_t t = j[i].deadline; t <= horizon;) {
      uint64_t demand = 0;
      for (unsigned h = 0; h < count; h++)
        if (t >= j[h].deadline)
          demand +=
              (uint64_t)(1 + (t - j[h].deadline) / j[h].period) * j[h].cost;
      if (demand > t)
        return CMX_TIMEOUT;
      if (j[i].period > horizon - t)
        break;
      t += j[i].period;
    }
  return CMX_OK;
}
enum cmx_result cmx_cbs_init(cmx_cbs *s, uint32_t q, uint32_t p) {
  if (!q || q > p || p >= UINT32_C(0x80000000))
    return CMX_INVALID;
  *s = (cmx_cbs){q, p, q, 0, 0};
  return CMX_OK;
}
enum cmx_result cmx_cbs_release(cmx_cbs *s, uint32_t now) {
  uint32_t distance = s->deadline - now;
  if (!s->active || distance >= UINT32_C(0x80000000) ||
      (uint64_t)s->remaining * s->period > (uint64_t)s->budget * distance) {
    s->remaining = s->budget;
    s->deadline = now + s->period;
  }
  s->active = 1;
  return CMX_OK;
}
enum cmx_result cmx_cbs_charge(cmx_cbs *s, uint32_t n) {
  if (!s->active || !n)
    return CMX_INVALID; /* One dispatch cannot spend beyond current remaining
                           budget. */
  if (n > s->remaining)
    return CMX_FULL;
  s->remaining -= n;
  if (!s->remaining) {
    s->remaining = s->budget;
    s->deadline += s->period;
  }
  return CMX_OK;
}
