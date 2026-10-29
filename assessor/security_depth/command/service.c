#include "command.h"

static int shape(const struct sc_batch *b)
{
    unsigned i, min = 0;
    if (!b || !b->count || b->count > 4 || !b->work || b->work > 4)
    {
        return 0;
    }
    for (i = 0; i < b->count; ++i)
    {
        const struct sc_command *c = &b->commands[i];
        if (c->count > 4 || (c->opcode != 1 && c->opcode != 2))
        {
            return 0;
        }
        min += c->count ? c->count : 1;
    }
    return min <= b->work;
}

void sc_service_init(struct sc_service *s)
{
    unsigned i;
    for (i = 0; i < 3; ++i)
    {
        s->jobs[i].active = 0;
        s->jobs[i].generation = 1;
        s->jobs[i].retries = 0;
    }
    s->used = s->count = s->terminal = s->released = s->control_pending =
        s->closed = 0;
}

enum sc_result sc_submit(struct sc_service *s, const struct sc_batch *b,
                         struct sc_ticket *out)
{
    unsigned i;
    if (!s || !out || !shape(b))
    {
        return SC_INVALID;
    }
    if (s->closed || s->terminal > UINT_MAX - s->count - 1 ||
        s->released > UINT_MAX - s->count - 1)
    {
        return SC_EXHAUSTED;
    }
#ifndef SC_MUTANT_QUOTA
    if (b->work > 6 - s->used)
    {
        return SC_BUDGET;
    }
#endif
    for (i = 0; i < 3; ++i)
    {
        if (!s->jobs[i].active && s->jobs[i].generation < UINT_MAX)
        {
            s->jobs[i].request = *b;
            s->jobs[i].active = 1;
            s->jobs[i].retries = 0;
            s->used += b->work;
            ++s->count;
            out->index = i;
            out->generation = s->jobs[i].generation;
            return SC_OK;
        }
    }
    return SC_FULL;
}

static struct sc_job *find(struct sc_service *s, struct sc_ticket t)
{
    if (!s || t.index >= 3 || !s->jobs[t.index].active ||
        s->jobs[t.index].generation != t.generation)
    {
        return 0;
    }
    return &s->jobs[t.index];
}

static void release(struct sc_service *s, struct sc_job *j)
{
    s->used -= j->request.work;
    --s->count;
    ++s->terminal;
    ++s->released;
#ifndef SC_MUTANT_TERMINAL
    j->active = 0;
    ++j->generation;
#endif
}

enum sc_result sc_finish(struct sc_service *s, struct sc_ticket t, int cancel,
                         const struct sc_authority *a, uint32_t *p, uint32_t *q)
{
    struct sc_job *j = find(s, t);
    enum sc_result r;
    if (!j)
    {
        return SC_STALE;
    }
    if (s->terminal == UINT_MAX || s->released == UINT_MAX)
    {
        return SC_EXHAUSTED;
    }
    r = cancel ? SC_OK : sc_apply(&j->request, a, p, q);
    release(s, j);
    return r;
}

enum sc_result sc_retry(struct sc_service *s, struct sc_ticket t)
{
    struct sc_job *j = find(s, t);
    if (!j)
    {
        return SC_STALE;
    }
#ifndef SC_MUTANT_RETRY
    if (j->retries == 2)
    {
        return SC_EXHAUSTED;
    }
#endif
    ++j->retries;
    return SC_OK;
}

enum sc_result sc_control(struct sc_service *s)
{
    if (!s)
    {
        return SC_INVALID;
    }
    if (s->closed)
    {
        return SC_EXHAUSTED;
    }
    if (s->control_pending)
    {
        return SC_FULL;
    }
    s->control_pending = 1;
    return SC_OK;
}

enum sc_result sc_control_complete(struct sc_service *s)
{
    if (!s)
    {
        return SC_INVALID;
    }
    if (!s->control_pending)
    {
        return SC_STALE;
    }
    s->control_pending = 0;
    return SC_OK;
}

enum sc_result sc_shutdown(struct sc_service *s)
{
    unsigned i;
    if (!s)
    {
        return SC_INVALID;
    }
    s->closed = 1;
    s->control_pending = 0;
    for (i = 0; i < 3; ++i)
    {
        if (s->jobs[i].active)
        {
            release(s, &s->jobs[i]);
        }
    }
    return SC_OK;
}

enum sc_result sc_recover(struct sc_recovery *s, struct sc_resources r,
                          int logger_ok)
{
    unsigned i;
    if (!s || !r.acquire || !r.release || s->state > 2 || s->attempts > 2 ||
        s->owned > 2)
    {
        return SC_INVALID;
    }
    while (s->owned)
    {
        r.release(r.context, --s->owned);
    }
    if (!logger_ok && s->log_fallbacks < UINT_MAX)
    {
        ++s->log_fallbacks;
    }
    if (s->state == 2
#ifndef SC_MUTANT_RECOVERY
        || s->attempts == 2
#endif
    )
    {
        s->state = 2;
        return SC_EXHAUSTED;
    }
    s->state = 1;
    ++s->attempts;
    for (i = 0; i < 2; ++i)
    {
        if (r.acquire(r.context, i))
        {
            while (s->owned)
            {
                r.release(r.context, --s->owned);
            }
            return SC_FULL;
        }
        ++s->owned;
    }
    s->state = 0;
    return SC_OK;
}
