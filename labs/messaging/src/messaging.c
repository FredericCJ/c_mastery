#include "messaging.h"
/* Unsolved operations compile; public initializers are supplied. */

void msg_nursery_init(MsgNursery *n, size_t fail) {
  atomic_init(&n->cancel, false);
  n->count = 0;
  n->attempts = 0;
  n->fail_at = fail;
  n->closing = false;
  n->closed = false;
}

void msg_cancel(MsgNursery *n) {
 (void)n;
}

MsgStatus msg_spawn(MsgNursery *n, MsgTask task, void *ctx) {
 (void)n;
 (void)task;
 (void)ctx;
 return MSG_INVALID;
}

MsgStatus msg_join(MsgNursery *n, int *result) {
 (void)n;
 (void)result;
 return MSG_INVALID;
}

MsgStatus msg_hub_init(MsgHub *h, size_t a, size_t b) {
  if (a > 4 || b > 4)
    return MSG_INVALID;
  if (pthread_mutex_init(&h->mutex, NULL))
    return MSG_FAILED;
  if (pthread_cond_init(&h->changed, NULL)) {
    pthread_mutex_destroy(&h->mutex);
    return MSG_FAILED;
  }
  h->channel[0] = (MsgChannel){.capacity = a};
  h->channel[1] = (MsgChannel){.capacity = b};
  h->stop = false;
  h->next = 0;
  return MSG_OK;
}

void msg_hub_destroy(MsgHub *h) {
  pthread_cond_destroy(&h->changed);
  pthread_mutex_destroy(&h->mutex);
}

void msg_hub_cancel(MsgHub *h) {
 (void)h;
}

MsgStatus msg_close(MsgHub *h, unsigned k) {
 (void)h;
 (void)k;
 return MSG_INVALID;
}

MsgStatus msg_send(MsgHub *h, unsigned k, unsigned v, bool wait) {
 (void)h;
 (void)k;
 (void)v;
 (void)wait;
 return MSG_INVALID;
}

MsgStatus msg_select(MsgHub *h, unsigned mask, bool wait, unsigned *which,
                     unsigned *value) {
 (void)h;
 (void)mask;
 (void)wait;
 (void)which;
 (void)value;
 return MSG_INVALID;
}

void msg_connect(MsgOperation *o, MsgReceiver r, void *ctx) {
  *o = (MsgOperation){.state = MSG_NEW, .receiver = r, .ctx = ctx};
}

MsgStatus msg_start(MsgOperation *o) {
 (void)o;
 return MSG_INVALID;
}

MsgStatus msg_stop_operation(MsgOperation *o) {
 (void)o;
 return MSG_INVALID;
}

MsgStatus msg_complete(MsgOperation *o, MsgStatus s, unsigned v) {
 (void)o;
 (void)s;
 (void)v;
 return MSG_INVALID;
}

void msg_actor_init(MsgActor *a, MsgHandler f, void *ctx) {
  *a = (MsgActor){.generation = 1, .alive = true, .handler = f, .ctx = ctx};
}

MsgStatus msg_actor_send(MsgActor *a, uint64_t g, unsigned v) {
 (void)a;
 (void)g;
 (void)v;
 return MSG_INVALID;
}

MsgStatus msg_actor_step(MsgActor *a) {
 (void)a;
 return MSG_INVALID;
}

MsgStatus msg_actor_restart(MsgActor *a) {
 (void)a;
 return MSG_INVALID;
}

MsgStatus msg_supervisor_init(MsgSupervisor *s, size_t limit, uint64_t window) {
  if (!limit || limit > 4 || !window)
    return MSG_INVALID;
  *s = (MsgSupervisor){.limit = limit, .window = window};
  return MSG_OK;
}

MsgStatus msg_restart_plan(MsgSupervisor *s, uint64_t now, MsgStrategy strategy,
                           unsigned failed, int *actions, size_t *count) {
 (void)s;
 (void)now;
 (void)strategy;
 (void)failed;
 (void)actions;
 (void)count;
 return MSG_INVALID;
}

void msg_deque_init(MsgDeque *q) {
  atomic_init(&q->top, 1);
  atomic_init(&q->bottom, 1);
  for (unsigned i = 0; i < 8; i++)
    atomic_init(&q->slots[i], 0);
}

MsgStatus msg_push(MsgDeque *q, unsigned v) {
 (void)q;
 (void)v;
 return MSG_INVALID;
}

MsgStatus msg_pop(MsgDeque *q, unsigned *out) {
 (void)q;
 (void)out;
 return MSG_INVALID;
}

MsgStatus msg_steal(MsgDeque *q, unsigned *out) {
 (void)q;
 (void)out;
 return MSG_INVALID;
}

void msg_store_init(MsgStore *s) {
  atomic_init(&s->clock, 0);
  for (unsigned i = 0; i < 4; i++) {
    atomic_init(&s->cells[i].stamp, 0);
    atomic_init(&s->cells[i].value, 0);
  }
}

void msg_begin(MsgStore *s, MsgTxn *t) {
 (void)s;
 (void)t;
}

void msg_abort(MsgTxn *t) {
 (void)t;
}

MsgStatus msg_read(MsgTxn *t, unsigned k, unsigned *out) {
 (void)t;
 (void)k;
 (void)out;
 return MSG_INVALID;
}

MsgStatus msg_write(MsgTxn *t, unsigned k, unsigned v) {
 (void)t;
 (void)k;
 (void)v;
 return MSG_INVALID;
}

MsgStatus msg_commit(MsgTxn *t) {
 (void)t;
 return MSG_INVALID;
}

MsgStatus msg_versions_init(MsgVersions *s) {
  if (pthread_mutex_init(&s->mutex, NULL))
    return MSG_FAILED;
  s->clock = 0;
  for (unsigned i = 0; i < 8; i++)
    s->versions[i] = (MsgVersion){0};
  s->versions[0].live = true;
  for (unsigned i = 0; i < 4; i++)
    s->pins[i] = (MsgPin){0};
  return MSG_OK;
}

void msg_versions_destroy(MsgVersions *s) { pthread_mutex_destroy(&s->mutex); }

MsgStatus msg_snapshot(MsgVersions *s, MsgSnapshot *out) {
 (void)s;
 (void)out;
 return MSG_INVALID;
}

MsgStatus msg_snapshot_read(MsgVersions *s, MsgSnapshot p, unsigned *out) {
 (void)s;
 (void)p;
 (void)out;
 return MSG_INVALID;
}

MsgStatus msg_snapshot_drop(MsgVersions *s, MsgSnapshot p) {
 (void)s;
 (void)p;
 return MSG_INVALID;
}

MsgStatus msg_publish(MsgVersions *s, unsigned a, unsigned b) {
 (void)s;
 (void)a;
 (void)b;
 return MSG_INVALID;
}

void msg_sequence_init(MsgSequence *s) {
  atomic_init(&s->sequence, 0);
  atomic_init(&s->first, 0);
  atomic_init(&s->second, 0);
}

MsgStatus msg_sequence_write(MsgSequence *s, unsigned a, unsigned b) {
 (void)s;
 (void)a;
 (void)b;
 return MSG_INVALID;
}

MsgStatus msg_sequence_read(MsgSequence *s, size_t tries, unsigned *out) {
 (void)s;
 (void)tries;
 (void)out;
 return MSG_INVALID;
}

void msg_stream_init(MsgStream *s, MsgNext next, MsgTerminal end, void *ctx) {
  *s = (MsgStream){.next = next, .end = end, .ctx = ctx};
}

MsgStatus msg_request(MsgStream *s, uint64_t n) {
 (void)s;
 (void)n;
 return MSG_INVALID;
}

void msg_stream_cancel(MsgStream *s) {
 (void)s;
}

MsgStatus msg_emit(MsgStream *s, unsigned v) {
 (void)s;
 (void)v;
 return MSG_INVALID;
}

MsgStatus msg_stream_end(MsgStream *s, MsgStatus status) {
 (void)s;
 (void)status;
 return MSG_INVALID;
}
