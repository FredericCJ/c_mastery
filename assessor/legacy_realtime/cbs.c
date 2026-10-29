#include "cbs.h"
#ifndef CB_MUTANT
#define CB_MUTANT 0
#endif
#define HALF UINT32_C(0x80000000)

static int parameters(uint32_t q, uint32_t d, uint32_t p)
{
    return q && q <= d && d <= p && p < HALF;
}

int cb_invariant(const struct cb_budget *s)
{
    return s &&
           parameters(s->core.budget, s->relative_deadline, s->core.period) &&
           (unsigned)s->policy <= CB_HARD_DOCUMENT &&
           (s->policy == CB_HARD_DOCUMENT ||
            s->relative_deadline == s->core.period) &&
           s->core.remaining <= s->core.budget && s->core.active <= 1 &&
           s->queued <= 1 && s->throttled <= 1 &&
           (!s->queued || s->core.active) &&
           (s->throttled ? (s->policy == CB_HARD_DOCUMENT && !s->core.remaining)
                         : s->core.remaining > 0);
}

enum cmx_result cb_init(struct cb_budget *s, uint32_t q, uint32_t d, uint32_t p,
                        enum cb_policy policy, uint32_t now)
{
    if (!s || !parameters(q, d, p) || (unsigned)policy > CB_HARD_DOCUMENT ||
        (policy != CB_HARD_DOCUMENT && d != p))
        return CMX_INVALID;
    struct cb_budget next = {0};
    enum cmx_result r = cmx_cbs_init(&next.core, q, p);
    if (r != CMX_OK)
        return r;
    next.relative_deadline = d;
    next.policy = policy;
    next.last = now;
    *s = next;
    return CMX_OK;
}

enum cmx_result cb_update(struct cb_budget *s, uint32_t now)
{
    if (!cb_invariant(s))
        return CMX_BROKEN;
    if (now - s->last >= HALF)
        return CMX_INVALID;
    struct cb_budget next = *s;
    if (next.throttled && now - next.core.deadline < HALF)
    {
        /* The finite comparison admits observations no later than one period
         * after the replenishment deadline; multiple missed windows require an
         * explicit catch-up policy, not hidden repeated refill. */
        if (now - next.core.deadline >= next.core.period)
            return CMX_FULL;
        next.core.deadline += next.core.period;
        next.core.remaining = next.core.budget;
        next.throttled = 0;
    }
    next.last = now;
    *s = next;
    return CMX_OK;
}

enum cmx_result cb_arrive(struct cb_budget *s, uint32_t now)
{
    if (!cb_invariant(s))
        return CMX_BROKEN;
    struct cb_budget next = *s;
    enum cmx_result r = cb_update(&next, now);
    if (r != CMX_OK)
        return r;
    if (!next.queued && !next.throttled)
    {
        uint32_t distance = next.core.deadline - now;
        uint64_t left = (uint64_t)next.core.remaining * next.core.period;
        uint64_t right = (uint64_t)next.core.budget * distance;
        unsigned reset = !next.core.active || distance >= HALF ||
                         left > right ||
                         (next.policy == CB_SOFT_BOOK && left == right);
        if (next.policy != CB_HARD_DOCUMENT)
        {
            /* Keep the actual producer's active bit after idle: it remembers the
             * current reservation. queued is the distinct pending-work state. */
            r = cmx_cbs_release(&next.core, now);
            if (r != CMX_OK)
                return r;
            if (reset && next.policy == CB_SOFT_BOOK)
            {
                next.core.remaining = next.core.budget;
                next.core.deadline = now + next.core.period;
            }
        }
        else if (reset)
        {
            next.core.remaining = next.core.budget;
            next.core.deadline =
                now +
                (CB_MUTANT == 3 ? next.core.period : next.relative_deadline);
            next.core.active = 1;
        }
        if (CB_MUTANT == 4 && next.policy == CB_SOFT_BOOK && left == right)
        {
            next.core.remaining = s->core.remaining;
            next.core.deadline = s->core.deadline;
        }
    }
    next.queued = 1;
    *s = next;
    return CMX_OK;
}

enum cmx_result cb_idle(struct cb_budget *s)
{
    if (!cb_invariant(s))
        return CMX_BROKEN;
    s->queued = 0;
    return CMX_OK;
}

enum cmx_result cb_charge(struct cb_budget *s, uint32_t now, uint32_t amount)
{
    if (!cb_invariant(s))
        return CMX_BROKEN;
    struct cb_budget next = *s;
    enum cmx_result r = cb_update(&next, now);
    if (r != CMX_OK)
        return r;
    if (!amount || !next.queued || next.throttled)
        return CMX_INVALID;
    if (amount > next.core.remaining)
        return CMX_FULL;
    if (amount == next.core.remaining && next.policy != CB_HARD_DOCUMENT)
    {
        uint32_t distance = next.core.deadline - now;
        if (distance < HALF && next.core.period >= HALF - distance)
            return CMX_FULL;
    }
    if (CB_MUTANT != 2)
    {
        if (next.policy == CB_HARD_DOCUMENT || CB_MUTANT == 1)
        {
            next.core.remaining -= amount;
            if (!next.core.remaining)
            {
                next.throttled = 1;
                if (CB_MUTANT == 1)
                    next.policy = CB_HARD_DOCUMENT;
            }
        }
        else
        {
            r = cmx_cbs_charge(&next.core, amount);
            if (r != CMX_OK)
                return r;
        }
    }
    *s = next;
    return CMX_OK;
}

enum cmx_result cb_admit(uint32_t q0, uint32_t d0, uint32_t p0, uint32_t q1,
                         uint32_t d1, uint32_t p1, unsigned *out)
{
    if (!out || !parameters(q0, d0, p0) || !parameters(q1, d1, p1))
        return CMX_INVALID;
    *out = (uint64_t)q0 * d1 + (uint64_t)q1 * d0 <= (uint64_t)d0 * d1;
    return CMX_OK;
}

enum cmx_result cb_workload(const struct cb_workload *w, struct cb_trace *out)
{
    if (!w || !out || w->count > CB_REQUESTS || !w->horizon ||
        w->horizon > CB_HORIZON || !w->capacity || w->capacity > 8)
        return CMX_INVALID;
    struct cb_budget s[2];
    for (unsigned c = 0; c < 2; c++)
    {
        enum cmx_result r = cb_init(&s[c], w->quota[c], w->deadline[c],
                                    w->period[c], w->policy[c], 0);
        if (r != CMX_OK)
            return r;
    }
    for (unsigned i = 0; i < w->count; i++)
        if (w->request[i].client > 1 || w->request[i].release >= w->horizon ||
            w->request[i].cost > 1024 ||
            w->request[i].application_deadline < w->request[i].release ||
            w->request[i].application_deadline > CB_HORIZON)
            return CMX_INVALID;
    struct cb_trace trace = {0};
    for (unsigned i = 0; i < CB_REQUESTS; i++)
        trace.finish[i] = UINT32_MAX;
    for (unsigned t = 0; t < w->horizon; t++)
    {
        unsigned pending[2] = {0, 0};
        int head[2] = {-1, -1};
        for (unsigned i = 0; i < w->count; i++)
            if (trace.admitted[i] && trace.remaining[i])
                pending[w->request[i].client]++;
        for (unsigned i = 0; i < w->count; i++)
            if (w->request[i].release == t)
            {
                unsigned c = w->request[i].client;
                if (w->request[i].cost && pending[c] == w->capacity)
                    trace.rejected[i] = 1;
                else
                {
                    trace.admitted[i] = 1;
                    trace.remaining[i] = w->request[i].cost;
                    if (!trace.remaining[i])
                        trace.finish[i] = t;
                    else
                    {
                        pending[c]++;
                        enum cmx_result r = cb_arrive(&s[c], t);
                        if (r != CMX_OK)
                            return r;
                    }
                }
            }
        for (unsigned i = 0; i < w->count; i++)
            if (trace.admitted[i] && trace.remaining[i])
            {
                unsigned c = w->request[i].client;
                if (head[c] < 0 || w->request[i].release <
                                       w->request[(unsigned)head[c]].release)
                    head[c] = (int)i;
                if (w->request[i].application_deadline <= t)
                    trace.missed[i] = 1;
            }
        int chosen = -1;
        for (unsigned c = 0; c < 2; c++)
        {
            enum cmx_result r = cb_update(&s[c], t);
            if (r != CMX_OK)
                return r;
            if (head[c] >= 0 && !s[c].throttled &&
                (chosen < 0 ||
                 (s[c].core.deadline != s[chosen].core.deadline &&
                  s[chosen].core.deadline - s[c].core.deadline < HALF)))
                chosen = (int)c;
        }
        trace.selected[t] = chosen;
        if (chosen >= 0)
        {
            unsigned i = (unsigned)head[chosen];
            enum cmx_result r = cb_charge(&s[chosen], t, 1);
            if (r != CMX_OK)
                return r;
            trace.remaining[i]--;
            trace.executed[i]++;
            if (!trace.remaining[i])
            {
                trace.finish[i] = t + 1;
                pending[chosen]--;
                if (!pending[chosen])
                {
                    r = cb_idle(&s[chosen]);
                    if (r != CMX_OK)
                        return r;
                }
            }
        }
        for (unsigned c = 0; c < 2; c++)
        {
            trace.budget[t][c] = s[c].core.remaining;
            trace.deadline[t][c] = s[c].core.deadline;
            trace.pending[t][c] = pending[c];
        }
    }
    for (unsigned i = 0; i < w->count; i++)
        if (trace.admitted[i] && trace.remaining[i] &&
            w->request[i].application_deadline <= w->horizon)
            trace.missed[i] = 1;
    *out = trace;
    return CMX_OK;
}

enum cmx_result cb_mutex(unsigned length, struct cb_mutex_trace *out)
{
    if (!out || length < 2 || length > 12)
        return CMX_INVALID;
    cmx_kernel kernel;
    cmx_mutex mutex;
    struct cb_budget budget[2];
    struct cb_mutex_trace trace = {0};
    cmx_init(&kernel, CMX_EDF);
    if (cmx_task_add(&kernel, 2, 10, 0x1000, 1024) != 0 ||
        cmx_task_add(&kernel, 1, 6, 0x2000, 1024) != 1)
        return CMX_BROKEN;
    if (cmx_mutex_init(&kernel, &mutex, 0, 0) != CMX_OK)
        return CMX_BROKEN;
    enum cmx_result r = cb_init(&budget[0], 1, 10, 10, CB_SOFT_BOOK, 0);
    if (r != CMX_OK)
        return r;
    r = cb_init(&budget[1], 2, 5, 5, CB_SOFT_BOOK, 0);
    if (r != CMX_OK)
        return r;
    r = cb_arrive(&budget[0], 0);
    if (r != CMX_OK)
        return r;
    /* The waiter is not yet released. Its sleep ends at t=1. */
    if (cmx_sleep(&kernel, 1, cmx_at(1)) != CMX_BLOCKED ||
        cmx_schedule(&kernel) != 0 ||
        cmx_mutex_lock(&kernel, &mutex, 0, cmx_forever()) != CMX_OK)
        return CMX_BROKEN;
    for (unsigned t = 0; t <= length; t++)
    {
        cmx_tick(&kernel, t);
        if (t == 1)
        {
            r = cb_arrive(&budget[1], t);
            if (r != CMX_OK)
                return r;
            kernel.task[1].job_deadline = budget[1].core.deadline;
            if (cmx_schedule(&kernel) != 1 ||
                cmx_mutex_lock(&kernel, &mutex, 1, cmx_forever()) !=
                    CMX_BLOCKED)
                return CMX_BROKEN;
        }
        int running = cmx_schedule(&kernel);
        if (running < 0)
            return CMX_BROKEN;
        unsigned charged = (unsigned)running;
        if (CB_MUTANT == 5 && kernel.task[1].state == CMX_WAITING)
            charged = 1;
        r = cb_charge(&budget[charged], t, 1);
        if (r != CMX_OK)
            return r;
        kernel.task[charged].job_deadline = budget[charged].core.deadline;
        trace.owner[t] = (unsigned)mutex.owner;
        trace.charged[t] = charged;
        trace.waiter_blocked[t] = kernel.task[1].state == CMX_WAITING;
        for (unsigned c = 0; c < 2; c++)
        {
            trace.budget[t][c] = budget[c].core.remaining;
            trace.deadline[t][c] = budget[c].core.deadline;
        }
        if ((unsigned)running == 0 && t + 1 == length)
        {
            if (cmx_mutex_unlock(&kernel, &mutex, 0) != CMX_OK ||
                cmx_exit(&kernel, 0, 0) != CMX_OK)
                return CMX_BROKEN;
        }
        else if (running == 1)
        {
            if (mutex.owner != 1 ||
                cmx_mutex_unlock(&kernel, &mutex, 1) != CMX_OK ||
                cmx_exit(&kernel, 1, 0) != CMX_OK)
                return CMX_BROKEN;
            trace.waiter_finish = t + 1;
            trace.waiter_missed = trace.waiter_finish > 4;
        }
        trace.ticks++;
    }
    *out = trace;
    return CMX_OK;
}
