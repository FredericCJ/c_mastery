#include "b05.h"
#include <limits.h>

int b5_verify(const struct b5_instruction *p, size_t n, unsigned *maximum)
{
    if (!p || !maximum || !n || n > 16)
    {
        return B5_INVALID;
    }
    int height[16];
    unsigned queue[16], head = 0, tail = 1, peak = 0;
    for (size_t i = 0; i < n; i++)
    {
        height[i] = -1;
        if (p[i].opcode > B5_HALT ||
            ((p[i].opcode == B5_JUMP || p[i].opcode == B5_BRANCH) &&
             p[i].target >= n))
        {
            return B5_INVALID;
        }
    }
    height[0] = 0;
    queue[0] = 0;
    while (head < tail)
    {
        unsigned pc = queue[head++], h = (unsigned)height[pc],
                 op = p[pc].opcode;
        if (op == B5_PUSH)
        {
            h++;
        }
        else if (op == B5_POP || op == B5_BRANCH)
        {
            if (!h)
            {
                return B5_CONFLICT;
            }
            h--;
        }
        else if (op == B5_ADD)
        {
            if (h < 2)
            {
                return B5_CONFLICT;
            }
            h--;
        }
        if (h > 8)
        {
            return B5_RANGE;
        }
        if (h > peak)
        {
            peak = h;
        }
        if (op == B5_HALT)
        {
            if (h != 1)
            {
                return B5_CONFLICT;
            }
            continue;
        }
        unsigned successors[2], count = 0;
        if (op != B5_JUMP)
        {
            if (pc + 1 >= n)
            {
                return B5_CONFLICT;
            }
            successors[count++] = pc + 1;
        }
        if (op == B5_JUMP || op == B5_BRANCH)
        {
            successors[count++] = p[pc].target;
        }
        for (unsigned i = 0; i < count; i++)
        {
            unsigned next = successors[i];
            if (height[next] < 0)
            {
                height[next] = (int)h;
                queue[tail++] = next;
            }
#ifndef B5_MUTATE_VERIFY
            else if (height[next] != (int)h)
            {
                return B5_CONFLICT;
            }
#endif
        }
    }
    *maximum = peak;
    return B5_OK;
}

uint32_t b5_checksum(uint32_t gen, uint32_t value)
{
    return gen ^ value ^ UINT32_C(0x93c72a5d);
}

int b5_recover(uint32_t generation, uint32_t base, const struct b5_redo *r,
               size_t n, uint32_t *out)
{
    if (!out || (!r && n) || n > 8)
    {
        return B5_INVALID;
    }
    uint32_t result = base, current = generation;
    for (size_t i = 0; i < n; i++)
    {
        if (r[i].commit > 1)
        {
            return B5_INVALID;
        }
        if (!r[i].commit)
        {
            break; /* first uncommitted tail terminates the log */
        }
#ifndef B5_MUTATE_RECOVER
        if (r[i].checksum != b5_checksum(r[i].generation, r[i].value))
        {
            return B5_CONFLICT;
        }
#endif
        if (r[i].generation <= generation)
        {
            continue; /* old already-checkpointed record */
        }
        if (current == UINT32_MAX || r[i].generation != current + 1)
        {
            return B5_CONFLICT;
        }
        current = r[i].generation;
        result = r[i].value;
    }
    *out = result;
    return B5_OK;
}

int b5_copy_to_user(struct b5_page pages[4], size_t address,
                    const unsigned char *input, size_t n)
{
    if (!pages || (!input && n))
    {
        return B5_INVALID;
    }
    if (address > 64 || n > 64 - address)
    {
        return B5_RANGE;
    }
#ifndef B5_MUTATE_COPY
    for (size_t i = 0; i < n; i++)
    {
        struct b5_page *p = &pages[(address + i) / 16];
        if (!p->present || !p->user || !p->writable)
        {
            return B5_CONFLICT;
        }
    }
#endif
    for (size_t i = 0; i < n; i++)
    {
        struct b5_page *p = &pages[(address + i) / 16];
#ifdef B5_MUTATE_COPY
        if (!p->present || !p->user || !p->writable)
        {
            return B5_CONFLICT;
        }
#endif
        p->bytes[(address + i) % 16] = input[i];
    }
    return B5_OK;
}

int b5_timer_arm(struct b5_timer *t, uint32_t now, uint32_t delay)
{
    if (!t)
    {
        return B5_INVALID;
    }
    if (delay >= UINT32_C(0x80000000))
    {
        return B5_RANGE;
    }
    t->last = now;
    t->remaining = delay;
    t->armed = 1;
    t->expired = delay == 0;
    return B5_OK;
}

int b5_timer_poll(struct b5_timer *t, uint32_t now, int *out)
{
    if (!t || !out || !t->armed)
    {
        return B5_INVALID;
    }
    uint32_t elapsed = now - t->last;
    if (elapsed >= UINT32_C(0x80000000))
    {
        return B5_RANGE;
    }
#ifdef B5_MUTATE_TIMER
    if (now < t->last)
    {
        return B5_RANGE;
    }
#endif
    if (elapsed >= t->remaining)
    {
        t->remaining = 0;
        t->expired = 1;
    }
    else
    {
        t->remaining -= elapsed;
    }
    t->last = now;
    *out = (int)t->expired;
    return B5_OK;
}

int b5_response(uint32_t own, uint32_t blocking, uint32_t deadline,
                const struct b5_task *h, size_t n, uint32_t *out)
{
    if (!out || (!h && n) || n > 4 || !own || !deadline || deadline > 1000000)
    {
        return B5_INVALID;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (!h[i].period || h[i].jitter > 1000000 || h[i].cost > 1000000)
        {
            return B5_INVALID;
        }
    }
    uint64_t base = (uint64_t)own + blocking, r = base;
    if (r > deadline)
    {
        return B5_RANGE;
    }
    for (uint32_t iteration = 0; iteration <= deadline; iteration++)
    {
        uint64_t next = base;
        for (size_t i = 0; i < n; i++)
        {
#ifdef B5_MUTATE_RESPONSE
            uint64_t jobs = (r + h[i].jitter) / h[i].period;
#else
            uint64_t jobs = (r + h[i].jitter + h[i].period - 1) / h[i].period;
#endif
            next += jobs * h[i].cost;
            if (next > deadline)
            {
                return B5_RANGE;
            }
        }
        if (next == r)
        {
            *out = (uint32_t)r;
            return B5_OK;
        }
        if (next < r)
        {
            return B5_CONFLICT;
        }
        r = next;
    }
    return B5_RANGE;
}

int b5_read_enter(struct b5_grace *g, unsigned id, uint32_t *out)
{
    if (!g || !out || id >= 4)
    {
        return B5_INVALID;
    }
    if (g->active[id] || g->next_ticket == UINT32_MAX)
    {
        return B5_RANGE;
    }
    g->next_ticket++;
    g->active[id] = g->next_ticket;
    *out = g->next_ticket;
    return B5_OK;
}

int b5_read_exit(struct b5_grace *g, unsigned id, uint32_t ticket)
{
    if (!g || id >= 4 || !ticket)
    {
        return B5_INVALID;
    }
    if (g->active[id] != ticket)
    {
        return B5_CONFLICT;
    }
    g->active[id] = 0;
    return B5_OK;
}

int b5_grace_begin(struct b5_grace *g)
{
    if (!g)
    {
        return B5_INVALID;
    }
    if (g->pending)
    {
        return B5_CONFLICT;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        g->waiting[i] = g->active[i];
    }
    g->pending = 1;
    return B5_OK;
}

int b5_grace_done(struct b5_grace *g, int *out)
{
    if (!g || !out || !g->pending)
    {
        return B5_INVALID;
    }
    unsigned blocked = 0;
    for (unsigned i = 0; i < 4; i++)
    {
#ifdef B5_MUTATE_GRACE
        blocked |= g->active[i] != 0;
#else
        blocked |= g->waiting[i] != 0 && g->waiting[i] == g->active[i];
#endif
    }
    *out = !blocked;
    if (!blocked)
    {
        g->pending = 0;
    }
    return B5_OK;
}

int b5_request(struct b5_request *r, unsigned op, uint32_t gen, unsigned result)
{
    if (!r || op > B5_REAP || result > 2)
    {
        return B5_INVALID;
    }
    if (gen != r->generation)
    {
        return B5_CONFLICT;
    }
    if (op == B5_SUBMIT)
    {
        if (r->state != B5_IDLE)
        {
            return B5_CONFLICT;
        }
        r->state = B5_INFLIGHT;
        r->terminal_count = 0;
        r->result = 0;
    }
    else if (op == B5_CANCEL)
    {
        if (r->state != B5_INFLIGHT && r->state != B5_CANCEL_REQUESTED)
        {
            return B5_CONFLICT;
        }
        r->state = B5_CANCEL_REQUESTED;
#ifdef B5_MUTATE_REQUEST
        r->state = B5_QUIESCENT;
        r->result = 2;
        r->terminal_count = 1;
#endif
    }
    else if (op == B5_COMPLETE)
    {
        if (r->state != B5_INFLIGHT && r->state != B5_CANCEL_REQUESTED)
        {
            return B5_CONFLICT;
        }
        r->state = B5_QUIESCENT;
        r->result = result;
        r->terminal_count = 1;
    }
    else
    {
        if (r->state != B5_QUIESCENT)
        {
            return B5_CONFLICT;
        }
        if (r->generation == UINT32_MAX)
        {
            r->state = B5_RETIRED;
        }
        else
        {
            r->generation++;
            r->state = B5_IDLE;
        }
    }
    return B5_OK;
}
