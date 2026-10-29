/* Eight kernel-owned messages per channel, cross-process attenuated capabilities. */
struct sf_message
{
    unsigned length;
    uint64_t sender;
    unsigned char bytes[SF_PAYLOAD_MAX];
};

struct sf_queue
{
    unsigned generation, state, owner, owner_generation, head, count;
    struct sf_message messages[8];
};

struct sf_capability
{
    unsigned generation, open, owner, owner_generation, queue, incarnation,
        rights;
};

struct sf_pending
{
    unsigned active, queue, incarnation, sending;
    struct sf_message message;
};
static struct sf_queue sf_queues[8];
static struct sf_capability sf_caps[64];
static struct sf_pending sf_waiters[THREADS];

static unsigned sf_cap(uint64_t token, unsigned pid, unsigned rights)
{
    if ((token & ~UINT64_C(0xffffffff)) != SF_CHANNEL_TAG)
    {
        return 64;
    }
    unsigned slot = (unsigned)(token & 65535);
    if (!slot || --slot >= 64)
    {
        return 64;
    }
    struct sf_capability *c = &sf_caps[slot];
    if (!c->open || c->owner != pid ||
        c->owner_generation != tasks[pid].generation ||
        (uint32_t)token != sf_raw_handle(slot, c->generation, 64) ||
        c->queue >= 8)
    {
        return 64;
    }
    struct sf_queue *q = &sf_queues[c->queue];
    if (!q->state || q->generation != c->incarnation ||
        (c->rights & rights) != rights)
    {
        return 64;
    }
    return slot;
}

static int64_t sf_cap_new(unsigned pid, unsigned q, unsigned rights)
{
    for (unsigned i = 0; i < 64; i++)
    {
        if (!sf_caps[i].open && sf_caps[i].generation < 65535)
        {
            struct sf_capability *c = &sf_caps[i];
            c->generation++;
            c->open = 1;
            c->owner = pid;
            c->owner_generation = tasks[pid].generation;
            c->queue = q;
            c->incarnation = sf_queues[q].generation;
            c->rights = rights;
            return (int64_t)(SF_CHANNEL_TAG |
                             sf_raw_handle(i, c->generation, 64));
        }
    }
    return SF_FULL;
}

static void sf_wake(unsigned id)
{
    struct sf_queue *q = &sf_queues[id];
    for (unsigned i = 0; i < thread_count; i++)
    {
        if (sf_waiters[i].active && sf_waiters[i].queue == id)
        {
            struct sf_pending *w = &sf_waiters[i];
            int64_t result = SF_RETRY;
            if (w->incarnation != q->generation)
            {
                result = SF_STALE;
            }
            else if (w->sending)
            {
                if (q->state == 1 && q->count == 8)
                {
                    continue;
                }
                if (q->state == 1)
                {
                    q->messages[(q->head + q->count) % 8] = w->message;
                    q->count++;
                    result = 0;
                }
                else
                {
                    result = SF_CLOSED;
                }
            }
            else if (q->state == 1 && !q->count)
            {
                continue;
            }
            w->active = 0;
            uthreads[i].frame->rax = (uint64_t)result;
            uthreads[i].state = UT_READY;
        }
    }
}

static void sf_channel_cleanup(unsigned pid)
{
    for (unsigned i = 0; i < thread_count; i++)
    {
        if (uthreads[i].process == pid)
        {
            sf_waiters[i].active = 0;
        }
    }
    for (unsigned i = 0; i < 64; i++)
    {
        if (sf_caps[i].open && sf_caps[i].owner == pid)
        {
            sf_caps[i].open = 0;
        }
    }
    for (unsigned q = 0; q < 8; q++)
    {
        if (sf_queues[q].state)
        {
            if (sf_queues[q].owner == pid &&
                sf_queues[q].owner_generation == tasks[pid].generation)
            {
                sf_queues[q].state = 2;
                sf_wake(q);
            }
            unsigned reachable = 0;
            for (unsigned i = 0; i < 64; i++)
            {
                if (sf_caps[i].open && sf_caps[i].queue == q &&
                    sf_caps[i].incarnation == sf_queues[q].generation)
                {
                    reachable = 1;
                }
            }
            if (!reachable)
            {
                sf_queues[q].state = 0;
            }
        }
    }
}

static int64_t sf_channel_call(struct frame *f)
{
    unsigned pid = uthreads[current].process;
    struct task *p = &tasks[pid];
    uint64_t a = f->rdi, b = f->rsi, c = f->rdx;
    switch (f->rax)
    {
        case SF_CHANNEL:
            for (unsigned q = 0; q < 8; q++)
            {
                if (!sf_queues[q].state && sf_queues[q].generation < 65535)
                {
                    struct sf_queue *v = &sf_queues[q];
                    v->generation++;
                    v->state = 1;
                    v->owner = pid;
                    v->owner_generation = p->generation;
                    v->head = v->count = 0;
                    int64_t h = sf_cap_new(pid, q, 7);
                    if (h < 0)
                    {
                        v->state = 0;
                    }
                    return h;
                }
            }
            return SF_FULL;
        case SF_GRANT:
        {
            unsigned cap = sf_cap(a, pid, 0);
            if (cap == 64)
            {
                return SF_DENIED;
            }
            struct sf_grant g;
            if (!user_ok(p, b, sizeof g, 0))
            {
                return SF_COPY;
            }
            memcpy(&g, (void *)(uintptr_t)b, sizeof g);
            unsigned recipient = sf_process(g.recipient);
            if (recipient == TASKS || !tasks[recipient].alive || g.reserved ||
                !g.rights || (g.rights & ~3u) ||
                (g.rights & sf_caps[cap].rights) != g.rights)
            {
                return SF_DENIED;
            }
            return sf_cap_new(recipient, sf_caps[cap].queue, g.rights);
        }
        case SF_SEND:
        case SF_RECEIVE:
        {
            unsigned sending = f->rax == SF_SEND,
                     cap = sf_cap(a, pid, sending ? 1 : 2);
            if (cap == 64)
            {
                return SF_DENIED;
            }
            if (c > 1)
            {
                return SF_INVALID;
            }
            unsigned id = sf_caps[cap].queue;
            struct sf_queue *q = &sf_queues[id];
            if (sending)
            {
                if (q->state != 1)
                {
                    return SF_CLOSED;
                }
                struct sf_span span;
                if (!user_ok(p, b, sizeof span, 0))
                {
                    return SF_COPY;
                }
                memcpy(&span, (void *)(uintptr_t)b, sizeof span);
                if (span.length > SF_PAYLOAD_MAX ||
                    (span.length && !user_ok(p, span.pointer, span.length, 0)))
                {
                    return SF_COPY;
                }
                struct sf_message m;
                m.length = (unsigned)span.length;
                m.sender = sf_process_token(pid);
                if (m.length)
                {
                    memcpy(m.bytes, (void *)(uintptr_t)span.pointer, m.length);
                }
                if (q->count == 8)
                {
                    if (c)
                    {
                        return SF_FULL;
                    }
                    sf_waiters[current].active = 1;
                    sf_waiters[current].queue = id;
                    sf_waiters[current].incarnation = q->generation;
                    sf_waiters[current].sending = 1;
                    sf_waiters[current].message = m;
                    uthreads[current].state = UT_SF_WAIT;
                    return 0;
                }
                q->messages[(q->head + q->count) % 8] = m;
                q->count++;
                sf_wake(id);
                return 0;
            }
            struct sf_receive span;
            if (!user_ok(p, b, sizeof span, 1))
            {
                return SF_COPY;
            }
            memcpy(&span, (void *)(uintptr_t)b, sizeof span);
            if (span.capacity > SF_PAYLOAD_MAX)
            {
                return SF_COPY;
            }
            if (!q->count)
            {
                if (q->state != 1)
                {
                    return SF_CLOSED;
                }
                if (c)
                {
                    return SF_FULL;
                }
                sf_waiters[current].active = 1;
                sf_waiters[current].queue = id;
                sf_waiters[current].incarnation = q->generation;
                sf_waiters[current].sending = 0;
                uthreads[current].state = UT_SF_WAIT;
                return 0;
            }
            struct sf_message *m = &q->messages[q->head];
            if (span.capacity < m->length)
            {
                return SF_FULL;
            }
            if (m->length && !user_ok(p, span.pointer, m->length, 1))
            {
#ifdef SF_MUTANT_CONSUME_COPY
                q->head = (q->head + 1) % 8;
                q->count--; /* Deliberately wrong failure publication. */
#endif
                return SF_COPY;
            }
            if (m->length)
            {
                memcpy((void *)(uintptr_t)span.pointer, m->bytes, m->length);
            }
            ((struct sf_receive *)(uintptr_t)b)->sender = m->sender;
            unsigned n = m->length;
            q->head = (q->head + 1) % 8;
            q->count--;
            sf_wake(id);
            return n;
        }
        case SF_CHANNEL_CLOSE:
        {
            unsigned cap = sf_cap(a, pid, 4);
            if (cap == 64)
            {
                return SF_DENIED;
            }
            unsigned q = sf_caps[cap].queue;
            if (sf_queues[q].owner != pid ||
                sf_queues[q].owner_generation != p->generation)
            {
                return SF_DENIED;
            }
            sf_queues[q].state = 2;
            sf_wake(q);
            return 0;
        }
        case SF_CHANNEL_DESTROY:
        {
            unsigned cap = sf_cap(a, pid, 0);
            if (cap == 64)
            {
                return SF_DENIED;
            }
            unsigned q = sf_caps[cap].queue;
            struct sf_queue *v = &sf_queues[q];
            unsigned owner_alive =
                v->owner < TASKS && tasks[v->owner].alive &&
                tasks[v->owner].generation == v->owner_generation;
            if (v->state != 2 || v->count ||
                (owner_alive &&
                 (v->owner != pid || !(sf_caps[cap].rights & 4))))
            {
                return SF_DENIED;
            }
            for (unsigned i = 0; i < THREADS; i++)
            {
                if (sf_waiters[i].active && sf_waiters[i].queue == q)
                {
                    return SF_INVALID;
                }
            }
            v->state = 0;
            for (unsigned i = 0; i < 64; i++)
            {
                if (sf_caps[i].queue == q)
                {
                    sf_caps[i].open = 0;
                }
            }
            return 0;
        }
        case SF_BLOCKED:
        {
            unsigned tid = sf_thread(a, pid);
            return tid == THREADS ? SF_STALE : (int64_t)sf_waiters[tid].active;
        }
        default:
            return SF_INVALID;
    }
}
