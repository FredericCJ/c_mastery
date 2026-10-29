#include "messaging.h"

/* Unsolved operations compile; public initializers are supplied. */

void msg_nursery_init(struct msg_nursery *n, size_t fail)
{
    atomic_init(&n->cancel, false);
    n->count = 0;
    n->attempts = 0;
    n->fail_at = fail;
    n->closing = false;
    n->closed = false;
}

void msg_cancel(struct msg_nursery *n)
{
    (void)n;
}

enum msg_status msg_spawn(struct msg_nursery *n, msg_task_fn task, void *ctx)
{
    (void)n;
    (void)task;
    (void)ctx;
    return MSG_INVALID;
}

enum msg_status msg_join(struct msg_nursery *n, int *result)
{
    (void)n;
    (void)result;
    return MSG_INVALID;
}

enum msg_status msg_hub_init(struct msg_hub *h, size_t a, size_t b)
{
    if (a > 4 || b > 4)
        return MSG_INVALID;
    if (pthread_mutex_init(&h->mutex, NULL))
        return MSG_FAILED;
    if (pthread_cond_init(&h->changed, NULL))
    {
        pthread_mutex_destroy(&h->mutex);
        return MSG_FAILED;
    }
    h->channel[0] = (struct msg_channel){.capacity = a};
    h->channel[1] = (struct msg_channel){.capacity = b};
    h->stop = false;
    h->next = 0;
    return MSG_OK;
}

void msg_hub_destroy(struct msg_hub *h)
{
    pthread_cond_destroy(&h->changed);
    pthread_mutex_destroy(&h->mutex);
}

void msg_hub_cancel(struct msg_hub *h)
{
    (void)h;
}

enum msg_status msg_close(struct msg_hub *h, unsigned k)
{
    (void)h;
    (void)k;
    return MSG_INVALID;
}

enum msg_status msg_send(struct msg_hub *h, unsigned k, unsigned v, bool wait)
{
    (void)h;
    (void)k;
    (void)v;
    (void)wait;
    return MSG_INVALID;
}

enum msg_status msg_select(struct msg_hub *h, unsigned mask, bool wait,
                           unsigned *which, unsigned *value)
{
    (void)h;
    (void)mask;
    (void)wait;
    (void)which;
    (void)value;
    return MSG_INVALID;
}

void msg_connect(struct msg_operation *o, msg_receiver_fn r, void *ctx)
{
    *o = (struct msg_operation){.state = MSG_NEW, .receiver = r, .ctx = ctx};
}

enum msg_status msg_start(struct msg_operation *o)
{
    (void)o;
    return MSG_INVALID;
}

enum msg_status msg_stop_operation(struct msg_operation *o)
{
    (void)o;
    return MSG_INVALID;
}

enum msg_status msg_complete(struct msg_operation *o, enum msg_status s,
                             unsigned v)
{
    (void)o;
    (void)s;
    (void)v;
    return MSG_INVALID;
}

void msg_actor_init(struct msg_actor *a, msg_handler_fn f, void *ctx)
{
    *a = (struct msg_actor){
        .generation = 1, .alive = true, .handler = f, .ctx = ctx};
}

enum msg_status msg_actor_send(struct msg_actor *a, uint64_t g, unsigned v)
{
    (void)a;
    (void)g;
    (void)v;
    return MSG_INVALID;
}

enum msg_status msg_actor_step(struct msg_actor *a)
{
    (void)a;
    return MSG_INVALID;
}

enum msg_status msg_actor_restart(struct msg_actor *a)
{
    (void)a;
    return MSG_INVALID;
}

enum msg_status msg_supervisor_init(struct msg_supervisor *s, size_t limit,
                                    uint64_t window)
{
    if (!limit || limit > 4 || !window)
        return MSG_INVALID;
    *s = (struct msg_supervisor){.limit = limit, .window = window};
    return MSG_OK;
}

enum msg_status msg_restart_plan(struct msg_supervisor *s, uint64_t now,
                                 enum msg_strategy strategy, unsigned failed,
                                 int *actions, size_t *count)
{
    (void)s;
    (void)now;
    (void)strategy;
    (void)failed;
    (void)actions;
    (void)count;
    return MSG_INVALID;
}

void msg_deque_init(struct msg_deque *q)
{
    atomic_init(&q->top, 1);
    atomic_init(&q->bottom, 1);
    for (unsigned i = 0; i < 8; i++)
        atomic_init(&q->slots[i], 0);
}

enum msg_status msg_push(struct msg_deque *q, unsigned v)
{
    (void)q;
    (void)v;
    return MSG_INVALID;
}

enum msg_status msg_pop(struct msg_deque *q, unsigned *out)
{
    (void)q;
    (void)out;
    return MSG_INVALID;
}

enum msg_status msg_steal(struct msg_deque *q, unsigned *out)
{
    (void)q;
    (void)out;
    return MSG_INVALID;
}

void msg_store_init(struct msg_store *s)
{
    atomic_init(&s->clock, 0);
    for (unsigned i = 0; i < 4; i++)
    {
        atomic_init(&s->cells[i].stamp, 0);
        atomic_init(&s->cells[i].value, 0);
    }
}

void msg_begin(struct msg_store *s, struct msg_txn *t)
{
    (void)s;
    (void)t;
}

void msg_abort(struct msg_txn *t)
{
    (void)t;
}

enum msg_status msg_read(struct msg_txn *t, unsigned k, unsigned *out)
{
    (void)t;
    (void)k;
    (void)out;
    return MSG_INVALID;
}

enum msg_status msg_write(struct msg_txn *t, unsigned k, unsigned v)
{
    (void)t;
    (void)k;
    (void)v;
    return MSG_INVALID;
}

enum msg_status msg_commit(struct msg_txn *t)
{
    (void)t;
    return MSG_INVALID;
}

enum msg_status msg_versions_init(struct msg_versions *s)
{
    if (pthread_mutex_init(&s->mutex, NULL))
        return MSG_FAILED;
    s->clock = 0;
    for (unsigned i = 0; i < 8; i++)
        s->versions[i] = (struct msg_version){0};
    s->versions[0].live = true;
    for (unsigned i = 0; i < 4; i++)
        s->pins[i] = (struct msg_pin){0};
    return MSG_OK;
}

void msg_versions_destroy(struct msg_versions *s)
{
    pthread_mutex_destroy(&s->mutex);
}

enum msg_status msg_snapshot(struct msg_versions *s, struct msg_snapshot *out)
{
    (void)s;
    (void)out;
    return MSG_INVALID;
}

enum msg_status msg_snapshot_read(struct msg_versions *s, struct msg_snapshot p,
                                  unsigned *out)
{
    (void)s;
    (void)p;
    (void)out;
    return MSG_INVALID;
}

enum msg_status msg_snapshot_drop(struct msg_versions *s, struct msg_snapshot p)
{
    (void)s;
    (void)p;
    return MSG_INVALID;
}

enum msg_status msg_publish(struct msg_versions *s, unsigned a, unsigned b)
{
    (void)s;
    (void)a;
    (void)b;
    return MSG_INVALID;
}

void msg_sequence_init(struct msg_sequence *s)
{
    atomic_init(&s->sequence, 0);
    atomic_init(&s->first, 0);
    atomic_init(&s->second, 0);
}

enum msg_status msg_sequence_write(struct msg_sequence *s, unsigned a,
                                   unsigned b)
{
    (void)s;
    (void)a;
    (void)b;
    return MSG_INVALID;
}

enum msg_status msg_sequence_read(struct msg_sequence *s, size_t tries,
                                  unsigned *out)
{
    (void)s;
    (void)tries;
    (void)out;
    return MSG_INVALID;
}

void msg_stream_init(struct msg_stream *s, msg_next_fn next,
                     msg_terminal_fn end, void *ctx)
{
    *s = (struct msg_stream){.next = next, .end = end, .ctx = ctx};
}

enum msg_status msg_request(struct msg_stream *s, uint64_t n)
{
    (void)s;
    (void)n;
    return MSG_INVALID;
}

void msg_stream_cancel(struct msg_stream *s)
{
    (void)s;
}

enum msg_status msg_emit(struct msg_stream *s, unsigned v)
{
    (void)s;
    (void)v;
    return MSG_INVALID;
}

enum msg_status msg_stream_end(struct msg_stream *s, enum msg_status status)
{
    (void)s;
    (void)status;
    return MSG_INVALID;
}
