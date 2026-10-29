#include "messaging.h"
#include <limits.h>
#ifndef SC_MUTANT
#define SC_MUTANT 0
#endif
static void *child_main(void *p)
{
    struct msg_child *c = p;
    c->result = c->task(c->ctx, &c->owner->cancel);
    if (c->result != 0 && SC_MUTANT != 1)
    {
        atomic_store(&c->owner->cancel, true);
    }
    return NULL;
}

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
    atomic_store(&n->cancel, true);
}

enum msg_status msg_spawn(struct msg_nursery *n, msg_task_fn task, void *ctx)
{
    if (n->closing || n->closed)
    {
        return MSG_CLOSED;
    }
    if (!task)
    {
        return MSG_INVALID;
    }
    if (n->count == 8)
    {
        return MSG_FULL;
    }
    if (n->attempts == SIZE_MAX)
    {
        return MSG_OVERFLOW;
    }
    ++n->attempts;
    if (n->fail_at == n->attempts)
    {
        msg_cancel(n);
        return MSG_FAILED;
    }
    struct msg_child *c = &n->child[n->count];
    c->task = task;
    c->ctx = ctx;
    c->owner = n;
    c->result = 0;
    c->joined = false;
    if (pthread_create(&c->thread, NULL, child_main, c) != 0)
    {
        msg_cancel(n);
        return MSG_FAILED;
    }
    ++n->count;
    return MSG_OK;
}

enum msg_status msg_join(struct msg_nursery *n, int *result)
{
    n->closing = true;
    int first = 0;
    for (size_t i = 0; i < n->count; i++)
    {
        struct msg_child *c = &n->child[i];
        if (!c->joined)
        {
            if (pthread_join(c->thread, NULL) != 0)
            {
                return MSG_FAILED;
            }
            c->joined = true;
        }
        if (first == 0 && c->result != 0 && SC_MUTANT != 11)
        {
            first = c->result;
        }
    }
    n->closed = true;
    *result = first;
    return MSG_OK;
}

enum msg_status msg_hub_init(struct msg_hub *h, size_t a, size_t b)
{
    if (a > 4 || b > 4)
    {
        return MSG_INVALID;
    }
    if (pthread_mutex_init(&h->mutex, NULL))
    {
        return MSG_FAILED;
    }
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
    pthread_mutex_lock(&h->mutex);
    h->stop = true;
    pthread_cond_broadcast(&h->changed);
    pthread_mutex_unlock(&h->mutex);
}

enum msg_status msg_close(struct msg_hub *h, unsigned k)
{
    if (k > 1)
    {
        return MSG_INVALID;
    }
    pthread_mutex_lock(&h->mutex);
    h->channel[k].closed = true;
    pthread_cond_broadcast(&h->changed);
    pthread_mutex_unlock(&h->mutex);
    return MSG_OK;
}

enum msg_status msg_send(struct msg_hub *h, unsigned k, unsigned v, bool wait)
{
    if (k > 1)
    {
        return MSG_INVALID;
    }
    pthread_mutex_lock(&h->mutex);
    struct msg_channel *c = &h->channel[k];
    enum msg_status status = MSG_OK;
    while (!h->stop && !c->closed &&
           (c->capacity ? c->count == c->capacity : c->offered))
    {
        if (!wait)
        {
            status = MSG_FULL;
            goto out;
        }
        pthread_cond_wait(&h->changed, &h->mutex);
    }
    if (h->stop)
    {
        status = MSG_CANCELLED;
        goto out;
    }
    if (c->closed && SC_MUTANT != 2)
    {
        status = MSG_CLOSED;
        goto out;
    }
    if (c->capacity)
    {
        c->values[(c->head + c->count) % 4] = v;
        ++c->count;
        pthread_cond_broadcast(&h->changed);
    }
    else
    {
        if (!wait)
        {
            status = MSG_FULL;
            goto out;
        }
        if (c->ticket == UINT64_MAX)
        {
            status = MSG_OVERFLOW;
            goto out;
        }
        uint64_t ticket = ++c->ticket;
        c->offer = v;
        c->offered = true;
        pthread_cond_broadcast(&h->changed);
        while (c->accepted < ticket && !c->closed && !h->stop)
        {
            pthread_cond_wait(&h->changed, &h->mutex);
        }
        if (c->accepted < ticket)
        {
            c->offered = false;
            status = h->stop ? MSG_CANCELLED : MSG_CLOSED;
            pthread_cond_broadcast(&h->changed);
        }
    }
out:
    pthread_mutex_unlock(&h->mutex);
    return status;
}

enum msg_status msg_select(struct msg_hub *h, unsigned mask, bool wait,
                           unsigned *which, unsigned *value)
{
    if (!mask || (mask & ~3u))
    {
        return MSG_INVALID;
    }
    pthread_mutex_lock(&h->mutex);
    enum msg_status status;
    for (;;)
    {
        if (h->stop)
        {
            status = MSG_CANCELLED;
            break;
        }
        unsigned live = 0;
        for (unsigned j = 0; j < 2; j++)
        {
            unsigned k = (h->next + j) % 2;
            if (!(mask & (1u << k)))
            {
                continue;
            }
            struct msg_channel *c = &h->channel[k];
            if (!c->closed)
            {
                ++live;
            }
            if ((c->count && SC_MUTANT != 12) || (c->offered && !c->closed))
            {
                *which = k;
                if (c->count)
                {
                    *value = c->values[c->head];
                    c->head = (c->head + 1) % 4;
                    --c->count;
                }
                else
                {
                    *value = c->offer;
                    c->offered = false;
                    c->accepted = c->ticket;
                }
                h->next = 1 - k;
                pthread_cond_broadcast(&h->changed);
                status = MSG_OK;
                goto out;
            }
        }
        if (!live)
        {
            status = MSG_CLOSED;
            break;
        }
        if (!wait)
        {
            status = MSG_EMPTY;
            break;
        }
        pthread_cond_wait(&h->changed, &h->mutex);
    }
out:
    pthread_mutex_unlock(&h->mutex);
    return status;
}

void msg_connect(struct msg_operation *o, msg_receiver_fn r, void *ctx)
{
    *o = (struct msg_operation){.state = MSG_NEW, .receiver = r, .ctx = ctx};
}

enum msg_status msg_start(struct msg_operation *o)
{
    if (o->state != MSG_NEW)
    {
        return MSG_INVALID;
    }
    if (!o->receiver)
    {
        return MSG_INVALID;
    }
    o->state = MSG_STARTED;
    return MSG_OK;
}

enum msg_status msg_stop_operation(struct msg_operation *o)
{
    if (o->state != MSG_STARTED)
    {
        return MSG_INVALID;
    }
    o->cancel = true;
    if (SC_MUTANT == 13)
    {
        o->state = MSG_DONE;
    }
    return MSG_OK;
}

enum msg_status msg_complete(struct msg_operation *o, enum msg_status s,
                             unsigned v)
{
    if (o->state != MSG_STARTED && SC_MUTANT != 3)
    {
        return MSG_INVALID;
    }
    if (s != MSG_OK && s != MSG_FAILED && s != MSG_CANCELLED)
    {
        return MSG_INVALID;
    }
    msg_receiver_fn r = o->receiver;
    void *ctx = o->ctx;
    o->state = MSG_DONE;
    r(ctx, s, v);
    return MSG_OK;
}

void msg_actor_init(struct msg_actor *a, msg_handler_fn f, void *ctx)
{
    *a = (struct msg_actor){
        .generation = 1, .alive = true, .handler = f, .ctx = ctx};
}

enum msg_status msg_actor_send(struct msg_actor *a, uint64_t g, unsigned v)
{
    if (g != a->generation && SC_MUTANT != 4)
    {
        return MSG_STALE;
    }
    if (!a->alive)
    {
        return MSG_CLOSED;
    }
    if (a->count == 4)
    {
        return MSG_FULL;
    }
    a->queue[(a->head + a->count) % 4] = v;
    ++a->count;
    return MSG_OK;
}

enum msg_status msg_actor_step(struct msg_actor *a)
{
    if (!a->alive)
    {
        return MSG_CLOSED;
    }
    if (a->stepping)
    {
        return MSG_BUSY;
    }
    if (!a->count)
    {
        return MSG_EMPTY;
    }
    unsigned v = a->queue[a->head];
    a->head = (a->head + 1) % 4;
    --a->count;
    a->stepping = true;
    bool ok = a->handler(a->ctx, v);
    a->stepping = false;
    if (!ok)
    {
        a->alive = false;
        if (SC_MUTANT != 14)
        {
            a->count = 0;
        }
        return MSG_FAILED;
    }
    return MSG_OK;
}

enum msg_status msg_actor_restart(struct msg_actor *a)
{
    if (a->alive || a->stepping)
    {
        return MSG_BUSY;
    }
    if (a->generation == UINT64_MAX)
    {
        return MSG_OVERFLOW;
    }
    ++a->generation;
    a->count = 0;
    a->head = 0;
    a->alive = true;
    return MSG_OK;
}

enum msg_status msg_supervisor_init(struct msg_supervisor *s, size_t limit,
                                    uint64_t window)
{
    if (!limit || limit > 4 || !window)
    {
        return MSG_INVALID;
    }
    *s = (struct msg_supervisor){.limit = limit, .window = window};
    return MSG_OK;
}

enum msg_status msg_restart_plan(struct msg_supervisor *s, uint64_t now,
                                 enum msg_strategy strategy, unsigned failed,
                                 int *actions, size_t *count)
{
    if (failed > 3 || strategy > MSG_REST || now < s->last)
    {
        return MSG_INVALID;
    }
    if (s->escalated)
    {
        return MSG_CLOSED;
    }
    s->last = now;
    size_t kept = 0;
    for (size_t i = 0; i < s->count; i++)
    {
        if (now - s->times[i] < s->window)
        {
            s->times[kept++] = s->times[i];
        }
    }
    s->count = kept;
    if (s->count == s->limit)
    {
        s->escalated = true;
        for (unsigned i = 0; i < 4; i++)
        {
            actions[i] = -(int)(4 - i);
        }
        *count = 4;
        return MSG_FAILED;
    }
    s->times[s->count++] = now;
    unsigned first = strategy == MSG_ALL ? 0 : failed,
             last = strategy == MSG_ONE ? failed : 3;
    size_t n = 0;
    for (unsigned i = last + 1; i > first; i--)
    {
        actions[n++] = -(int)(SC_MUTANT == 5 ? first + last + 2 - i : i);
    }
    if (SC_MUTANT != 15)
    {
        for (unsigned i = first; i <= last; i++)
        {
            actions[n++] = (int)i + 1;
        }
    }
    *count = n;
    return MSG_OK;
}

void msg_deque_init(struct msg_deque *q)
{
    atomic_init(&q->top, 1);
    atomic_init(&q->bottom, 1);
    for (unsigned i = 0; i < 8; i++)
    {
        atomic_init(&q->slots[i], 0);
    }
}

enum msg_status msg_push(struct msg_deque *q, unsigned v)
{
    size_t b = atomic_load(&q->bottom), t = atomic_load(&q->top);
    if (b == SIZE_MAX - 1)
    {
        return MSG_OVERFLOW;
    }
    if (b - t >= 8)
    {
        return MSG_FULL;
    }
    atomic_store(&q->slots[b % 8], v);
    atomic_store(&q->bottom, b + 1);
    return MSG_OK;
}

enum msg_status msg_pop(struct msg_deque *q, unsigned *out)
{
    size_t b = atomic_load(&q->bottom);
    if (!b)
    {
        return MSG_EMPTY;
    }
    --b;
    atomic_store(&q->bottom, b);
    size_t t = atomic_load(&q->top);
    if (t > b)
    {
        atomic_store(&q->bottom, b + 1);
        return MSG_EMPTY;
    }
    unsigned v = atomic_load(&q->slots[b % 8]);
    if (t == b)
    {
        bool won = atomic_compare_exchange_strong(&q->top, &t, t + 1);
        atomic_store(&q->bottom, b + 1);
        if (!won)
        {
            return MSG_CONFLICT;
        }
    }
    *out = SC_MUTANT == 6 ? v + 1 : v;
    return MSG_OK;
}

enum msg_status msg_steal(struct msg_deque *q, unsigned *out)
{
    size_t t = atomic_load(&q->top), b = atomic_load(&q->bottom);
    if (t >= b)
    {
        return MSG_EMPTY;
    }
    unsigned v = atomic_load(&q->slots[t % 8]);
    if (SC_MUTANT != 16 && !atomic_compare_exchange_strong(&q->top, &t, t + 1))
    {
        return MSG_CONFLICT;
    }
    *out = v;
    return MSG_OK;
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
    *t = (struct msg_txn){
        .store = s, .snapshot = atomic_load(&s->clock), .active = true};
}

void msg_abort(struct msg_txn *t)
{
    t->active = false;
}

enum msg_status msg_read(struct msg_txn *t, unsigned k, unsigned *out)
{
    if (!t->active || k > 3)
    {
        return MSG_INVALID;
    }
    if (t->writes & (1u << k))
    {
        *out = t->values[k];
        return MSG_OK;
    }
    struct msg_cell *c = &t->store->cells[k];
    unsigned before = atomic_load(&c->stamp), v = atomic_load(&c->value),
             after = atomic_load(&c->stamp);
    if (SC_MUTANT != 7 &&
        ((before & 1u) || before != after || (before >> 1) > t->snapshot))
    {
        msg_abort(t);
        return MSG_CONFLICT;
    }
    t->reads |= 1u << k;
    *out = v;
    return MSG_OK;
}

enum msg_status msg_write(struct msg_txn *t, unsigned k, unsigned v)
{
    if (!t->active || k > 3)
    {
        return MSG_INVALID;
    }
    t->writes |= 1u << k;
    t->values[k] = v;
    return MSG_OK;
}

enum msg_status msg_commit(struct msg_txn *t)
{
    if (!t->active)
    {
        return MSG_INVALID;
    }
    struct msg_store *s = t->store;
    unsigned locked = 0, old[4] = {0}, stamp = 0;
    enum msg_status status = MSG_CONFLICT;
    for (unsigned i = 0; i < 4; i++)
    {
        if (t->writes & (1u << i))
        {
            unsigned v = atomic_load(&s->cells[i].stamp);
            old[i] = v;
            if ((v & 1u) ||
                !atomic_compare_exchange_strong(&s->cells[i].stamp, &v, v | 1u))
            {
                goto abort;
            }
            locked |= 1u << i;
        }
    }
    if (t->writes)
    {
        unsigned clock = atomic_load(&s->clock);
        do
        {
            if (clock == UINT_MAX / 2)
            {
                status = MSG_OVERFLOW;
                goto abort;
            }
        } while (!atomic_compare_exchange_weak(&s->clock, &clock, clock + 1));
        stamp = clock + 1;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (t->reads & (1u << i))
        {
            unsigned v =
                (locked & (1u << i)) ? old[i] : atomic_load(&s->cells[i].stamp);
            if (SC_MUTANT != 17 && ((v & 1u) || (v >> 1) > t->snapshot))
            {
                goto abort;
            }
        }
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (locked & (1u << i))
        {
            atomic_store(&s->cells[i].value, t->values[i]);
        }
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (locked & (1u << i))
        {
            atomic_store(&s->cells[i].stamp, stamp << 1);
        }
    }
    t->active = false;
    return MSG_OK;
abort:
    for (unsigned i = 0; i < 4; i++)
    {
        if (locked & (1u << i))
        {
            atomic_store(&s->cells[i].stamp, old[i]);
        }
    }
    t->active = false;
    return status;
}

static size_t visible(struct msg_versions *s, uint64_t stamp)
{
    size_t best = 8;
    for (size_t i = 0; i < 8; i++)
    {
        if (s->versions[i].live && s->versions[i].stamp <= stamp &&
            (best == 8 || s->versions[i].stamp > s->versions[best].stamp))
        {
            best = i;
        }
    }
    return best;
}

static void collect(struct msg_versions *s)
{
    bool keep[8] = {false};
    size_t current = visible(s, s->clock);
    keep[current] = true;
    if (SC_MUTANT != 8)
    {
        for (unsigned j = 0; j < 4; j++)
        {
            if (s->pins[j].active)
            {
                keep[visible(s, s->pins[j].stamp)] = true;
            }
        }
    }
    for (unsigned i = 0; i < 8; i++)
    {
        if (!keep[i])
        {
            s->versions[i].live = false;
        }
    }
}

enum msg_status msg_versions_init(struct msg_versions *s)
{
    if (pthread_mutex_init(&s->mutex, NULL))
    {
        return MSG_FAILED;
    }
    s->clock = 0;
    for (unsigned i = 0; i < 8; i++)
    {
        s->versions[i] = (struct msg_version){0};
    }
    s->versions[0].live = true;
    for (unsigned i = 0; i < 4; i++)
    {
        s->pins[i] = (struct msg_pin){0};
    }
    return MSG_OK;
}

void msg_versions_destroy(struct msg_versions *s)
{
    pthread_mutex_destroy(&s->mutex);
}

enum msg_status msg_snapshot(struct msg_versions *s, struct msg_snapshot *out)
{
    pthread_mutex_lock(&s->mutex);
    enum msg_status status = MSG_FULL;
    for (unsigned i = 0; i < 4; i++)
    {
        if (!s->pins[i].active && s->pins[i].generation < UINT64_MAX)
        {
            s->pins[i].active = true;
            s->pins[i].stamp = s->clock;
            ++s->pins[i].generation;
            *out = (struct msg_snapshot){i, s->pins[i].generation};
            status = MSG_OK;
            break;
        }
    }
    pthread_mutex_unlock(&s->mutex);
    return status;
}

static bool pin_valid(struct msg_versions *s, struct msg_snapshot p)
{
    return p.slot < 4 && s->pins[p.slot].active &&
           (SC_MUTANT == 18 || s->pins[p.slot].generation == p.generation);
}

enum msg_status msg_snapshot_read(struct msg_versions *s, struct msg_snapshot p,
                                  unsigned *out)
{
    pthread_mutex_lock(&s->mutex);
    enum msg_status status = MSG_STALE;
    if (pin_valid(s, p))
    {
        size_t v = visible(s, s->pins[p.slot].stamp);
        if (v < 8)
        {
            out[0] = s->versions[v].values[0];
            out[1] = s->versions[v].values[1];
            status = MSG_OK;
        }
        else
        {
            status = MSG_FAILED;
        }
    }
    pthread_mutex_unlock(&s->mutex);
    return status;
}

enum msg_status msg_snapshot_drop(struct msg_versions *s, struct msg_snapshot p)
{
    pthread_mutex_lock(&s->mutex);
    enum msg_status status = MSG_STALE;
    if (pin_valid(s, p))
    {
        s->pins[p.slot].active = false;
        collect(s);
        status = MSG_OK;
    }
    pthread_mutex_unlock(&s->mutex);
    return status;
}

enum msg_status msg_publish(struct msg_versions *s, unsigned a, unsigned b)
{
    pthread_mutex_lock(&s->mutex);
    enum msg_status status = MSG_FULL;
    if (s->clock == UINT64_MAX)
    {
        status = MSG_OVERFLOW;
        goto out;
    }
    collect(s);
    for (unsigned i = 0; i < 8; i++)
    {
        if (!s->versions[i].live)
        {
            s->versions[i] = (struct msg_version){
                .values = {a, b}, .stamp = ++s->clock, .live = true};
            status = MSG_OK;
            break;
        }
    }
out:
    pthread_mutex_unlock(&s->mutex);
    return status;
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
    unsigned v = atomic_load(&s->sequence);
    if (v & 1u)
    {
        return MSG_BUSY;
    }
    if (v > UINT_MAX - 2)
    {
        return MSG_OVERFLOW;
    }
    atomic_store(&s->sequence, v + 1);
    atomic_store(&s->first, a);
    if (SC_MUTANT != 19)
    {
        atomic_store(&s->second, b);
    }
    atomic_store(&s->sequence, v + 2);
    return MSG_OK;
}

enum msg_status msg_sequence_read(struct msg_sequence *s, size_t tries,
                                  unsigned *out)
{
    for (size_t i = 0; i < tries; i++)
    {
        unsigned a = atomic_load(&s->sequence);
        if ((a & 1u) && SC_MUTANT != 9)
        {
            continue;
        }
        unsigned x = atomic_load(&s->first), y = atomic_load(&s->second),
                 b = atomic_load(&s->sequence);
        if (a == b)
        {
            out[0] = x;
            out[1] = y;
            return MSG_OK;
        }
    }
    return MSG_BUSY;
}

void msg_stream_init(struct msg_stream *s, msg_next_fn next,
                     msg_terminal_fn end, void *ctx)
{
    *s = (struct msg_stream){.next = next, .end = end, .ctx = ctx};
}

enum msg_status msg_request(struct msg_stream *s, uint64_t n)
{
    if (s->terminal || s->cancelled || s->pending_error)
    {
        return MSG_CLOSED;
    }
    if (!n)
    {
        if (s->delivering)
        {
            s->pending_error = true;
            return MSG_OK;
        }
        return msg_stream_end(s, MSG_INVALID);
    }
    s->demand = UINT64_MAX - s->demand < n ? UINT64_MAX : s->demand + n;
    return MSG_OK;
}

void msg_stream_cancel(struct msg_stream *s)
{
    s->cancelled = true;
}

enum msg_status msg_emit(struct msg_stream *s, unsigned v)
{
    if (s->terminal || s->cancelled)
    {
        return MSG_CLOSED;
    }
    if (s->delivering)
    {
        return MSG_BUSY;
    }
    if (!s->demand && SC_MUTANT != 10)
    {
        return MSG_EMPTY;
    }
    if (s->demand && s->demand != UINT64_MAX)
    {
        --s->demand;
    }
    s->delivering = true;
    s->next(s->ctx, v);
    s->delivering = false;
    if (s->pending_error && !s->cancelled)
    {
        s->pending_error = false;
        return msg_stream_end(s, MSG_INVALID);
    }
    return MSG_OK;
}

enum msg_status msg_stream_end(struct msg_stream *s, enum msg_status status)
{
    if ((s->terminal || s->cancelled) && SC_MUTANT != 20)
    {
        return MSG_CLOSED;
    }
    if (s->delivering)
    {
        return MSG_BUSY;
    }
    if (status != MSG_OK && status != MSG_FAILED && status != MSG_INVALID)
    {
        return MSG_INVALID;
    }
    s->terminal = true;
    s->end(s->ctx, status);
    return MSG_OK;
}
