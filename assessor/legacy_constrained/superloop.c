#include "superloop.h"
#ifndef SC_MUTANT
#define SC_MUTANT 0
#endif
int sc_bound(const uint64_t *cost, size_t count, uint64_t event_cost, int known,
             struct sc_bound *out)
{
    if (!cost || !out || !count || count > 4)
    {
        return SC_INVALID;
    }
    if (!known && SC_MUTANT != 1)
    {
        return SC_UNBOUNDED;
    }
    struct sc_bound staged = {0};
    for (size_t i = 0; i < count; i++)
    {
        if (UINT64_MAX - staged.round < cost[i])
        {
            return SC_RANGE;
        }
        staged.round += cost[i];
        if (cost[i] > staged.longest)
        {
            staged.longest = cost[i];
        }
    }
    if (UINT64_MAX - staged.round < event_cost)
    {
        return SC_RANGE;
    }
    staged.response = staged.round + event_cost;
    *out = staged;
    return SC_OK;
}

int sc_task_init(struct sc_task *task, const unsigned char *input,
                 size_t length)
{
    if (!task || (!input && length) || length > 128)
    {
        return SC_INVALID;
    }
    struct sc_task staged = {0};
    staged.length = length;
    staged.state = length ? SC_READY : SC_DONE;
    staged.emissions = length ? 0 : 1;
    for (size_t i = 0; i < length; i++)
    {
        staged.input[i] = input[i];
    }
    *task = staged;
    return SC_OK;
}

int sc_task_step(struct sc_task *task, size_t quota, size_t *work)
{
    if (!task || !work || !quota)
    {
        return SC_INVALID;
    }
    if (task->state != SC_READY && SC_MUTANT != 2)
    {
        return SC_CLOSED;
    }
    size_t count = task->length - task->cursor;
    if (count > quota)
    {
        count = quota;
    }
    for (size_t i = 0; i < count; i++)
    {
        unsigned char c = task->input[task->cursor];
        task->staged[task->cursor++] =
            (c >= 0x61 && c <= 0x7a) ? (unsigned char)(c - 0x20) : c;
    }
    if (task->cursor == task->length)
    {
        task->state = SC_DONE;
        task->emissions++;
    }
    *work = count;
    return SC_OK;
}

int sc_task_cancel(struct sc_task *task)
{
    if (!task)
    {
        return SC_INVALID;
    }
    if (task->state != SC_READY)
    {
        return SC_CLOSED;
    }
    task->state = SC_CANCELLED;
    if (SC_MUTANT == 3)
    {
        task->state = SC_DONE;
    }
    return SC_OK;
}

int sc_task_result(const struct sc_task *task, unsigned char *out,
                   size_t capacity, size_t *length)
{
    if (!task || !out || !length)
    {
        return SC_INVALID;
    }
    if (task->state != SC_DONE)
    {
        return SC_CLOSED;
    }
    if (capacity < task->length)
    {
        return SC_FULL;
    }
    for (size_t i = 0; i < task->length; i++)
    {
        out[i] = task->staged[i];
    }
    *length = task->length;
    return SC_OK;
}

int sc_events_init(struct sc_events *events)
{
    if (!events)
    {
        return SC_INVALID;
    }
    *events = (struct sc_events){0};
    return SC_OK;
}

int sc_events_push(struct sc_events *events, struct sc_event event)
{
    if (!events)
    {
        return SC_INVALID;
    }
    if (events->count == 3)
    {
        if (events->rejected == UINT64_MAX)
        {
            return SC_RANGE;
        }
        events->rejected++;
        if (SC_MUTANT != 4)
        {
            return SC_FULL;
        }
        events->data[events->head] = event;
        return SC_OK;
    }
    if (events->accepted == UINT64_MAX)
    {
        return SC_RANGE;
    }
    events->data[(events->head + events->count) % 3] = event;
    events->count++;
    events->accepted++;
    return SC_OK;
}

int sc_events_pop(struct sc_events *events, struct sc_event *out)
{
    if (!events || !out)
    {
        return SC_INVALID;
    }
    if (!events->count)
    {
        return SC_EMPTY;
    }
    *out = events->data[events->head];
    events->head = (events->head + 1) % 3;
    events->count--;
    return SC_OK;
}

int sc_latest_init(struct sc_latest *latest)
{
    if (!latest)
    {
        return SC_INVALID;
    }
    *latest = (struct sc_latest){0};
    return SC_OK;
}

int sc_latest_put(struct sc_latest *latest, uint64_t value)
{
    if (!latest)
    {
        return SC_INVALID;
    }
    if (latest->updates == UINT64_MAX ||
        (latest->pending && latest->superseded == UINT64_MAX))
    {
        return SC_RANGE;
    }
    if (latest->pending)
    {
        latest->superseded++;
    }
    latest->value = value;
    latest->updates++;
    latest->pending = 1;
    return SC_OK;
}

int sc_latest_take(struct sc_latest *latest, uint64_t *out)
{
    if (!latest || !out)
    {
        return SC_INVALID;
    }
    if (!latest->pending)
    {
        return SC_EMPTY;
    }
    *out = latest->value;
    latest->pending = 0;
    return SC_OK;
}

int sc_releases_init(struct sc_releases *r, uint64_t period, uint64_t first,
                     unsigned policy)
{
    if (!r || !period || policy > SC_LOSSLESS)
    {
        return SC_INVALID;
    }
    *r =
        (struct sc_releases){.period = period, .next = first, .policy = policy};
    return SC_OK;
}

int sc_releases_poll(struct sc_releases *r, uint64_t now)
{
    if (!r || (r->observed && now < r->last))
    {
        return SC_INVALID;
    }
    struct sc_releases s = *r;
    uint64_t due = 0, latest = 0;
    if (!s.exhausted && now >= s.next)
    {
        uint64_t quotient = (now - s.next) / s.period;
        if (quotient == UINT64_MAX)
        {
            return SC_RANGE;
        }
        due = quotient + 1;
        latest = s.next + quotient * s.period;
    }
    if (UINT64_MAX - s.seen < due)
    {
        return SC_RANGE;
    }
    if (s.policy == SC_LOSSLESS)
    {
        if (due > 2 - s.count && SC_MUTANT != 5)
        {
            return SC_FULL;
        }
        if (due > 2 - s.count)
        {
            return SC_RANGE; /* mutant refusal still wrong status, no overwrite */
        }
        for (uint64_t i = 0; i < due; i++)
        {
            s.items[s.count++] = s.next + i * s.period;
        }
    }
    else if (due || s.policy == SC_REJECT_STALE)
    {
        unsigned keep = due && (s.policy == SC_COALESCE || latest == now);
        if (!due && s.count && s.items[s.count - 1] == now)
        {
            keep = 1;
        }
        uint64_t removed = due - (due && keep ? 1u : 0u);
        if (UINT64_MAX - removed < s.count)
        {
            return SC_RANGE;
        }
        removed += s.count;
        if (!due && keep)
        {
            removed--;
        }
        if (UINT64_MAX - s.lost < removed)
        {
            return SC_RANGE;
        }
        s.lost += removed;
        s.count = keep;
        if (keep && due)
        {
            s.items[0] = latest;
        }
    }
    if (due)
    {
        s.seen += due;
        if (UINT64_MAX - latest < s.period)
        {
            s.exhausted = 1;
        }
        else
        {
            s.next = latest + s.period;
        }
    }
    s.last = now;
    s.observed = 1;
    *r = s;
    return SC_OK;
}

int sc_releases_pop(struct sc_releases *r, uint64_t *out)
{
    if (!r || !out)
    {
        return SC_INVALID;
    }
    if (!r->count)
    {
        return SC_EMPTY;
    }
    *out = r->items[0];
    if (r->count == 2)
    {
        r->items[0] = r->items[1];
    }
    r->count--;
    return SC_OK;
}

int sc_schedule_init(struct sc_schedule *s, unsigned mode)
{
    if (!s || mode > SC_BUDGETED)
    {
        return SC_INVALID;
    }
    *s = (struct sc_schedule){.mode = mode};
    return SC_OK;
}

int sc_schedule_next(struct sc_schedule *s, uint64_t now, unsigned ready,
                     unsigned *out)
{
    if (!s || !out || ready > 15 || (s->observed && now <= s->last))
    {
        return SC_INVALID;
    }
    struct sc_schedule staged = *s;
    staged.last = now;
    staged.observed = 1;
    uint64_t epoch = now / 4;
    if (!s->observed || epoch != s->epoch)
    {
        staged.epoch = epoch;
        staged.high_used = 0;
    }
    unsigned selected = 4;
    if (staged.mode != SC_ROUND_ROBIN && (ready & 8) &&
        (staged.mode == SC_PRIORITY || !staged.high_used || SC_MUTANT == 7))
    {
        selected = 3;
        staged.high_used = 1;
    }
    else
    {
        unsigned width = staged.mode == SC_ROUND_ROBIN ? 4 : 3;
        for (unsigned i = 0; i < width; i++)
        {
            unsigned n = (staged.cursor + i) % width;
            if (ready & (1u << n))
            {
                selected = n;
                if (SC_MUTANT != 6)
                {
                    staged.cursor = (n + 1) % width;
                }
                break;
            }
        }
    }
    *s = staged;
    if (selected == 4)
    {
        return SC_EMPTY;
    }
    *out = selected;
    return SC_OK;
}
