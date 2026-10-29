#include "resilience.h"
#include <limits.h>

static int add_int(int a, int b, int *out)
{
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b))
        return RZ_RANGE;
    *out = a + b;
    return RZ_OK;
}

int rz_scope_init(struct rz_scope *s, const struct rz_scope *p, uint64_t now,
                  uint64_t budget)
{
    if (!s || s == p || (p && p->depth >= 8))
        return RZ_INVALID;
    uint64_t deadline;
    if (p && p->deadline <= now)
        deadline = p->deadline;
    else if (p && budget >= p->deadline - now)
        deadline = p->deadline;
    else
    {
        if (budget > UINT64_MAX - now)
            return RZ_RANGE;
        deadline = now + budget;
    }
#ifdef RZ_MUTATE_DEADLINE1
    if (p && budget <= UINT64_MAX - now)
        deadline = now + budget;
#endif
    s->parent = p;
    s->deadline = deadline;
    s->depth = p ? p->depth + 1 : 1;
    atomic_init(&s->cancelled, false);
    return RZ_OK;
}

void rz_cancel(struct rz_scope *s)
{
    if (s)
        atomic_store(&s->cancelled, true);
}

int rz_check(const struct rz_scope *s, uint64_t now)
{
    if (!s)
        return RZ_INVALID;
#ifndef RZ_MUTATE_DEADLINE2
    for (const struct rz_scope *p = s; p; p = p->parent)
        if (atomic_load(&p->cancelled))
            return RZ_CANCELLED;
#endif
    return now >= s->deadline ? RZ_EXPIRED : RZ_OK;
}

int rz_remaining(const struct rz_scope *s, uint64_t now, uint64_t *out)
{
    if (!out)
        return RZ_INVALID;
    int status = rz_check(s, now);
    if (status != RZ_OK)
        return status;
    *out = s->deadline - now;
    return RZ_OK;
}

void rz_queue_init(struct rz_queue *q)
{
    if (q)
        *q = (struct rz_queue){0};
}

int rz_offer(struct rz_queue *q, struct rz_offer *o, enum rz_policy policy,
             uint64_t now)
{
    if (!q || !o || !o->pending || policy > RZ_DEGRADE || policy < RZ_BLOCK)
        return RZ_INVALID;
    int status = rz_check(o->scope, now);
    if (status != RZ_OK)
    {
        o->pending = false;
        return status;
    }
    if (q->closed)
    {
        o->pending = false;
        return RZ_CLOSED;
    }
    if (q->count == 4)
    {
        if (policy == RZ_BLOCK)
            return RZ_PENDING;
        o->pending = false;
        if (policy == RZ_REJECT)
            return RZ_FULL;
        if (policy == RZ_DROP)
        {
#ifdef RZ_MUTATE_PRESSURE1
            q->values[q->head] = o->value;
#endif
            return RZ_DROPPED;
        }
        if (policy == RZ_DEGRADE)
        {
            size_t last = (q->head + 3) % 4;
            if (o->value > q->values[last])
                q->values[last] = o->value;
            return RZ_DEGRADED;
        }
#ifndef RZ_MUTATE_PRESSURE2
        q->head = (q->head + 1) % 4;
#endif
        q->count--;
    }
    q->values[(q->head + q->count) % 4] = o->value;
    q->count++;
    o->pending = false;
    return RZ_OK;
}

int rz_pop(struct rz_queue *q, unsigned *out)
{
    if (!q || !out)
        return RZ_INVALID;
    if (!q->count)
        return q->closed ? RZ_CLOSED : RZ_EMPTY;
    *out = q->values[q->head];
    q->head = (q->head + 1) % 4;
    q->count--;
    return RZ_OK;
}

int rz_forward(struct rz_queue *q, struct msg_hub *hub, unsigned channel)
{
    if (!q || !hub || channel > 1)
        return RZ_INVALID;
    if (!q->count)
        return RZ_EMPTY;
    enum msg_status s = msg_send(hub, channel, q->values[q->head], false);
    if (s == MSG_FULL)
        return RZ_FULL;
    if (s == MSG_CLOSED)
        return RZ_CLOSED;
    if (s == MSG_CANCELLED)
        return RZ_CANCELLED;
    if (s != MSG_OK)
        return RZ_INVALID;
    q->head = (q->head + 1) % 4;
    q->count--;
    return RZ_OK;
}

void rz_wheel_init(struct rz_wheel *w, uint64_t now)
{
    if (!w)
        return;
    *w = (struct rz_wheel){.now = now};
    for (unsigned i = 0; i < 4; i++)
    {
        w->head[i] = -1;
        w->slots[i] =
            (struct rz_timer){.next = -1, .previous = -1, .generation = 1};
    }
}

static void unlink_timer(struct rz_wheel *w, unsigned i)
{
    struct rz_timer *t = &w->slots[i];
    unsigned bucket = (unsigned)(t->due % 4);
    if (t->previous < 0)
        w->head[bucket] = t->next;
    else
        w->slots[t->previous].next = t->next;
    if (t->next >= 0)
        w->slots[t->next].previous = t->previous;
    t->active = false;
    t->next = t->previous = -1;
    t->generation++;
}

int rz_schedule(struct rz_wheel *w, uint64_t delay, struct rz_timer_id *out)
{
    if (!w || !out || delay < 1 || delay > 16)
        return RZ_INVALID;
    if (delay > UINT64_MAX - w->now)
        return RZ_RANGE;
    unsigned i;
    for (i = 0; i < 4; i++)
        if (!w->slots[i].active && w->slots[i].generation <= 3)
            break;
    if (i == 4)
        return RZ_FULL;
    struct rz_timer *t = &w->slots[i];
    t->due = w->now + delay;
    unsigned bucket = (unsigned)(t->due % 4);
    t->next = w->head[bucket];
    t->previous = -1;
    if (t->next >= 0)
        w->slots[t->next].previous = (int)i;
    w->head[bucket] = (int)i;
    t->active = true;
    *out = (struct rz_timer_id){w, i, t->generation};
    return RZ_OK;
}

int rz_unschedule(struct rz_wheel *w, struct rz_timer_id id)
{
    if (!w)
        return RZ_INVALID;
    if (id.owner != w || id.slot >= 4 || !w->slots[id.slot].active)
        return RZ_STALE;
#ifndef RZ_MUTATE_WHEEL2
    if (w->slots[id.slot].generation != id.generation)
        return RZ_STALE;
#endif
    unlink_timer(w, id.slot);
    return RZ_OK;
}

int rz_tick(struct rz_wheel *w, unsigned *mask)
{
    if (!w || !mask)
        return RZ_INVALID;
    if (w->now == UINT64_MAX)
        return RZ_RANGE;
    w->now++;
    unsigned fired = 0;
    int i = w->head[w->now % 4];
    while (i >= 0)
    {
        struct rz_timer *t = &w->slots[i];
        int next = t->next;
#ifdef RZ_MUTATE_WHEEL1
        if (true)
#else
        if (t->due == w->now)
#endif
        {
            fired |= 1u << (unsigned)i;
            unlink_timer(w, (unsigned)i);
        }
        i = next;
    }
    *mask = fired;
    return RZ_OK;
}

int rz_bucket_init(struct rz_bucket *b, uint64_t capacity, uint64_t rate,
                   uint64_t now)
{
    if (!b || capacity < 1 || capacity > 16 || rate < 1 || rate > 16)
        return RZ_INVALID;
    *b = (struct rz_bucket){capacity, rate, capacity, now};
    return RZ_OK;
}

int rz_consume(struct rz_bucket *b, uint64_t now, uint64_t cost)
{
    if (!b || !cost || cost > b->capacity)
        return RZ_INVALID;
    if (now < b->last)
        return RZ_INVALID;
    uint64_t missing = b->capacity - b->tokens, elapsed = now - b->last;
#ifdef RZ_MUTATE_BUCKET1
    (void)missing;
    (void)elapsed;
    b->tokens = b->capacity;
#else
    if (elapsed >= missing / b->rate + (missing % b->rate != 0))
        b->tokens = b->capacity;
    else
        b->tokens += elapsed * b->rate;
#endif
    b->last = now;
#ifdef RZ_MUTATE_BUCKET2
    if (b->tokens <= cost)
        return RZ_FULL;
#else
    if (b->tokens < cost)
        return RZ_FULL;
#endif
    b->tokens -= cost;
    return RZ_OK;
}

int rz_retry_init(struct rz_retry *r, const struct rz_scope *s, unsigned max,
                  uint64_t base, uint64_t cap, bool safe)
{
    if (!r || !s || !max || max > 4 || !base || base > 8 || cap < base ||
        cap > 32)
        return RZ_INVALID;
    *r = (struct rz_retry){s, 0, max, base, cap, safe};
    return RZ_OK;
}

int rz_retry_next(struct rz_retry *r, uint64_t now, uint64_t draw,
                  uint64_t *out)
{
    if (!r || !out)
        return RZ_INVALID;
#ifndef RZ_MUTATE_RETRY1
    if (!r->idempotent)
        return RZ_DENIED;
#endif
    int s = rz_check(r->scope, now);
    if (s != RZ_OK)
        return s;
    if (r->attempts == r->maximum)
        return RZ_FULL;
    uint64_t window = r->base;
    for (unsigned i = 0; i < r->attempts; i++)
        window = window > r->cap / 2 ? r->cap : window * 2;
    if (draw > window)
        return RZ_INVALID;
#ifndef RZ_MUTATE_RETRY2
    if (draw >= r->scope->deadline - now)
        return RZ_EXPIRED;
#endif
    if (draw > UINT64_MAX - now)
        return RZ_RANGE;
    r->attempts++;
    *out = now + draw;
    return RZ_OK;
}

int rz_gate_init(struct rz_gate *g, unsigned limit, uint64_t now)
{
    if (!g || limit < 1 || limit > 2)
        return RZ_INVALID;
    *g = (struct rz_gate){
        .limit = limit, .epoch = 1, .last = now, .generations = {1, 1}};
    return RZ_OK;
}

int rz_admit(struct rz_gate *g, uint64_t now, struct rz_permit *out)
{
    if (!g || !out || now < g->last)
        return RZ_INVALID;
    g->last = now;
    if (g->retired)
        return RZ_OPEN;
    if (g->phase == 1)
    {
        if (now < g->until)
            return RZ_OPEN;
        g->phase = 2;
    }
    unsigned count = 0;
    for (unsigned i = 0; i < 2; i++)
        if (g->active[i])
            count++;
#ifdef RZ_MUTATE_GATE1
    (void)count;
#else
    if (count >= g->limit)
        return RZ_BUSY;
#endif
    if (g->phase == 2)
        for (unsigned i = 0; i < 2; i++)
            if (g->active[i] && g->epochs[i] == g->epoch)
                return RZ_BUSY;
    unsigned i;
    for (i = 0; i < 2; i++)
        if (!g->active[i] && g->generations[i] <= 3)
            break;
    if (i == 2)
        return RZ_FULL;
    g->active[i] = true;
    g->epochs[i] = g->epoch;
    *out = (struct rz_permit){g, i, g->generations[i]};
    return RZ_OK;
}

static void open_gate(struct rz_gate *g, uint64_t now)
{
    g->phase = 1;
    if (g->epoch == 7)
    {
        g->retired = true;
        return;
    }
    g->epoch++;
    if (now > UINT64_MAX - 4)
    {
        g->retired = true;
        return;
    }
    g->until = now + 4;
}

int rz_finish(struct rz_gate *g, struct rz_permit p, uint64_t now, int outcome)
{
    if (!g || now < g->last || outcome < -1 || outcome > 1)
        return RZ_INVALID;
    if (p.owner != g || p.slot >= 2 || !g->active[p.slot] ||
        p.generation != g->generations[p.slot])
        return RZ_STALE;
    unsigned epoch = g->epochs[p.slot];
    g->active[p.slot] = false;
    g->generations[p.slot]++;
    g->last = now;
#ifndef RZ_MUTATE_GATE2
    if (epoch != g->epoch)
        return RZ_OK;
#else
    (void)epoch;
#endif
    if (outcome < 0)
        return RZ_OK;
    if (g->phase == 2)
    {
        if (outcome)
        {
            g->phase = 0;
            g->failures = 0;
        }
        else
            open_gate(g, now);
    }
    else if (outcome)
    {
        g->failures = 0;
#ifdef RZ_MUTATE_GATE2
        g->phase = 0;
#endif
    }
    else if (++g->failures >= 2)
        open_gate(g, now);
    return RZ_OK;
}

int rz_hedge_init(struct rz_hedge *h, const struct rz_scope *s, uint64_t now,
                  uint64_t delay)
{
    if (!h || !s || delay > UINT64_MAX - now)
        return RZ_INVALID;
    int status = rz_check(s, now);
    if (status != RZ_OK)
        return status;
    *h = (struct rz_hedge){
        .scope = s, .hedge_at = now + delay, .started = 1, .winner = -1};
    return RZ_OK;
}

int rz_hedge_start(struct rz_hedge *h, uint64_t now)
{
    if (!h)
        return RZ_INVALID;
    int status = rz_check(h->scope, now);
    if (status != RZ_OK)
        return status;
    if (h->winner >= 0 || h->started == 3)
        return RZ_DONE;
    if (now < h->hedge_at)
        return RZ_PENDING;
    h->started |= 2;
    return RZ_OK;
}

int rz_hedge_complete(struct rz_hedge *h, unsigned attempt, uint64_t now,
                      bool acceptable, unsigned value)
{
    if (!h || attempt > 1)
        return RZ_INVALID;
    unsigned bit = 1u << attempt;
    if (!(h->started & bit) || (h->completed & bit))
        return RZ_STALE;
    h->completed |= bit;
    int status = rz_check(h->scope, now);
    if (status != RZ_OK)
    {
        h->cancel_requested |= h->started & ~h->completed;
        return status;
    }
#ifdef RZ_MUTATE_HEDGE1
    acceptable = true;
#endif
    if (acceptable && h->winner < 0)
    {
        h->winner = (int)attempt;
        h->value = value;
        h->cancel_requested |= h->started & ~h->completed;
#ifdef RZ_MUTATE_HEDGE2
        h->completed = h->started;
#endif
        return RZ_OK;
    }
    return RZ_DONE;
}

int rz_hedge_result(const struct rz_hedge *h, unsigned *out)
{
    if (!h || !out)
        return RZ_INVALID;
    if (h->winner < 0)
        return RZ_PENDING;
    *out = h->value;
    return RZ_OK;
}

bool rz_hedge_quiescent(const struct rz_hedge *h)
{
    return h && h->started == h->completed;
}

int rz_receiver_init(struct rz_receiver *r, uint64_t now, uint64_t window)
{
    if (!r || !window || window > 16)
        return RZ_INVALID;
    *r = (struct rz_receiver){.window = window, .last = now};
    return RZ_OK;
}

int rz_receive(struct rz_receiver *r, uint64_t now, unsigned client,
               unsigned id, uint64_t fence, int delta, int *out,
               bool *duplicate)
{
    if (!r || !out || !duplicate || client > 1 || id < 1 || id > 15 || !fence ||
        delta < -8 || delta > 8 || now < r->last)
        return RZ_INVALID;
#ifndef RZ_MUTATE_RECEIVER1
    if (fence < r->fence)
        return RZ_STALE;
#endif
    unsigned free_slot = 4;
    for (unsigned i = 0; i < 4; i++)
    {
        struct rz_entry *e = &r->entries[i];
        if (!e->live || now - e->born >= r->window)
        {
            if (free_slot == 4)
                free_slot = i;
            continue;
        }
        if (e->client == client && e->id == id)
        {
#ifndef RZ_MUTATE_RECEIVER2
            if (e->delta != delta)
                return RZ_CONFLICT;
#endif
            *out = e->result;
            *duplicate = true;
            r->last = now;
            if (fence > r->fence)
                r->fence = fence;
            return RZ_OK;
        }
    }
    if (free_slot == 4)
        return RZ_FULL;
    int total;
    int status = add_int(r->total, delta, &total);
    if (status != RZ_OK)
        return status;
    r->entries[free_slot] =
        (struct rz_entry){true, client, id, delta, total, now};
    r->total = total;
    r->last = now;
    if (fence > r->fence)
        r->fence = fence;
    *out = total;
    *duplicate = false;
    return RZ_OK;
}

int rz_service_init(struct rz_service *s, const struct rz_scope *scope,
                    struct msg_hub *hub, uint64_t now)
{
    if (!s || !scope || !hub)
        return RZ_INVALID;
    int status = rz_check(scope, now);
    if (status != RZ_OK)
        return status;
    s->scope = scope;
    s->hub = hub;
    rz_queue_init(&s->pending);
    (void)rz_bucket_init(&s->limiter, 4, 1, now);
    (void)rz_gate_init(&s->breaker, 2, now);
    (void)rz_receiver_init(&s->receiver, now, 8);
    return RZ_OK;
}

int rz_service_submit(struct rz_service *s, uint64_t now, unsigned client,
                      unsigned id, uint64_t fence, int delta, int *out)
{
    if (!s || !out)
        return RZ_INVALID;
    int status = rz_check(s->scope, now);
    if (status != RZ_OK)
        return status;
#ifndef RZ_MUTATE_SERVICE1
    if (s->pending.count == 4)
        return RZ_FULL;
#endif
    status = rz_consume(&s->limiter, now, 1);
    if (status != RZ_OK)
        return status;
    struct rz_permit p;
    status = rz_admit(&s->breaker, now, &p);
    if (status != RZ_OK)
        return status;
    int result;
    bool duplicate;
    status = rz_receive(&s->receiver, now, client, id, fence, delta, &result,
                        &duplicate);
    (void)rz_finish(&s->breaker, p, now, status == RZ_OK ? 1 : -1);
    if (status != RZ_OK)
        return status;
    if (!duplicate)
    {
        /* Admission reserves this serialized queue slot before the effect. A later
           cancellation requests cleanup; it cannot roll back the committed effect.
         */
        if (s->pending.count == 4)
            return RZ_FULL;
        s->pending.values[(s->pending.head + s->pending.count) % 4] = id;
        s->pending.count++;
    }
    *out = result;
    return RZ_OK;
}

int rz_service_flush(struct rz_service *s)
{
    if (!s)
        return RZ_INVALID;
#ifdef RZ_MUTATE_SERVICE2
    if (s->pending.count)
    {
        s->pending.head = (s->pending.head + 1) % 4;
        s->pending.count--;
        return RZ_OK;
    }
#endif
    return rz_forward(&s->pending, s->hub, 0);
}
