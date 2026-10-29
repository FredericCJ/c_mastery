#include "systems.h"
#include <limits.h>

int s_pipeline(const struct s_instruction *p, size_t n, unsigned forwarding,
               unsigned out[16], unsigned *cycles)
{
    if ((!p && n) || !out || !cycles || n > 16 || forwarding > 1)
        return S_INVALID;
    unsigned ready[8] = {0}, trace[16], next = 1, last = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (p[i].opcode > S_BRANCH || p[i].destination > 7 || p[i].left > 7 ||
            p[i].right > 7 || p[i].taken > 1 ||
            (p[i].opcode != S_BRANCH && p[i].taken) ||
            (p[i].opcode == S_BRANCH && p[i].destination))
            return S_INVALID;
        unsigned at = next;
        if (at < ready[p[i].left])
            at = ready[p[i].left];
        if (at < ready[p[i].right])
            at = ready[p[i].right];
        trace[i] = at;
        last = at + 4;
        next = at + 1;
        if (p[i].opcode == S_BRANCH)
        {
            if (p[i].taken)
                next += 2;
        }
        else if (p[i].destination)
        {
            unsigned delay = forwarding ? (p[i].opcode == S_LOAD ? 2 : 1) : 3;
#ifdef S_MUTATE_PIPELINE
            if (forwarding)
                delay = 1;
#endif
            ready[p[i].destination] = at + delay;
        }
    }
    for (size_t i = 0; i < n; i++)
        out[i] = trace[i];
    *cycles = last;
    return S_OK;
}

int s_i2c_drive(const struct s_si2c *s, unsigned *release)
{
    if (!s || !release || s->byte > 255 || s->bit > 8 || s->state > S_I2C_LOST)
        return S_INVALID;
    if (s->state == S_I2C_DATA)
    {
        if (s->bit >= 8)
            return S_INVALID;
        *release = (s->byte >> (7 - s->bit)) & 1;
    }
    else
        *release = 1;
    return S_OK;
}

int s_i2c_sample(struct s_si2c *s, unsigned scl, unsigned sda)
{
    unsigned release;
    if (!s || scl > 1 || sda > 1 || s_i2c_drive(s, &release))
        return S_INVALID;
    if (s->state != S_I2C_DATA && s->state != S_I2C_ACK)
        return S_CONFLICT;
    if (!scl)
        return S_PENDING;
    if (!release && sda)
        return S_CONFLICT; /* electrically inconsistent observation */
    if (s->state == S_I2C_ACK)
    {
        s->state = sda ? S_I2C_NACK : S_I2C_DONE;
        return S_OK;
    }
#ifndef S_MUTATE_I2C
    if (release && !sda)
    {
        s->state = S_I2C_LOST;
        return S_OK;
    }
#endif
    s->bit++;
    if (s->bit == 8)
        s->state = S_I2C_ACK;
    return S_OK;
}

int s_checksum_add(struct s_checksum *s, const unsigned char *p, size_t n)
{
    if (!s || (!p && n) || s->sum > 65535 || s->pending > 1 || s->octet > 255)
        return S_INVALID;
    for (size_t i = 0; i < n; i++)
    {
        if (!s->pending)
        {
            s->octet = p[i];
            s->pending = 1;
        }
        else
        {
            s->sum += (s->octet << 8) | p[i];
            s->sum = (s->sum & 65535) + (s->sum >> 16);
            s->pending = 0;
        }
    }
#ifdef S_MUTATE_CHECKSUM
    if (s->pending)
    {
        s->sum += s->octet << 8;
        s->sum = (s->sum & 65535) + (s->sum >> 16);
        s->pending = 0;
    }
#endif
    return S_OK;
}

int s_checksum_finish(const struct s_checksum *s, uint16_t *out)
{
    if (!s || !out || s->sum > 65535 || s->pending > 1 || s->octet > 255)
        return S_INVALID;
    uint32_t sum = s->sum + (s->pending ? s->octet << 8 : 0);
    sum = (sum & 65535) + (sum >> 16);
    *out = (uint16_t)(sum ^ 65535);
    return S_OK;
}

int s_paired(const uint32_t *a, const uint32_t *b, size_t n, int64_t *out,
             unsigned *wins)
{
    if (!a || !b || !out || !wins || !n || n > 32)
        return S_INVALID;
    int64_t difference[32];
    unsigned count = 0;
    for (size_t i = 0; i < n; i++)
    {
        difference[i] = (int64_t)b[i] - a[i];
        count += b[i] < a[i];
    }
    for (size_t i = 1; i < n; i++)
    {
        int64_t value = difference[i];
        size_t j = i;
        while (j && difference[j - 1] > value)
        {
            difference[j] = difference[j - 1];
            j--;
        }
        difference[j] = value;
    }
#ifdef S_MUTATE_PAIRED
    int64_t sum = 0;
    for (size_t i = 0; i < n; i++)
        sum += difference[i];
    *out = sum / (int64_t)n;
#else
    *out = difference[(n - 1) / 2];
#endif
    *wins = count;
    return S_OK;
}

int s_reduce(const unsigned char *input, size_t n, s_classify_fn classify,
             void *ctx, unsigned budget, unsigned char out[32], size_t *length,
             unsigned *calls)
{
    if ((!input && n) || !classify || !out || !length || !calls || n > 32)
        return S_INVALID;
    if (!budget)
        return S_RANGE;
    unsigned char current[32], candidate[32];
    for (size_t i = 0; i < n; i++)
        current[i] = input[i];
    unsigned used = 1;
    int result = classify(ctx, current, n);
    if (result != S_FAIL)
        return S_CONFLICT;
    size_t granularity = 2;
    while (n >= 2)
    {
        if (granularity > n)
            granularity = n;
        size_t width = (n + granularity - 1) / granularity;
        unsigned reduced = 0, unresolved = 0;
        for (size_t start = 0; start < n; start += width)
        {
            size_t end = start + width;
            if (end > n)
                end = n;
            size_t count = 0;
            for (size_t i = 0; i < n; i++)
                if (i < start || i >= end)
                    candidate[count++] = current[i];
            if (used == budget)
                return S_RANGE;
            result = classify(ctx, candidate, count);
            used++;
            if (result < 0 || result > S_UNRESOLVED)
                return S_INVALID;
            if (result == S_UNRESOLVED)
                unresolved = 1;
#ifdef S_MUTATE_REDUCE
            if (result != S_PASS)
            {
#else
            if (result == S_FAIL)
            {
#endif
                for (size_t i = 0; i < count; i++)
                    current[i] = candidate[i];
                n = count;
                granularity = granularity > 2 ? granularity - 1 : 2;
                reduced = 1;
                break;
            }
        }
        if (reduced)
            continue;
        if (granularity == n)
        {
            if (unresolved)
                return S_PENDING;
            break;
        }
        granularity *= 2;
    }
    /* Explicit empty trial closes the single-token 1-minimality boundary. */
    if (n == 1)
    {
        if (used == budget)
            return S_RANGE;
        result = classify(ctx, current, 0);
        used++;
        if (result == S_FAIL)
            n = 0;
        else if (result == S_UNRESOLVED)
            return S_PENDING;
        else if (result < 0 || result > S_UNRESOLVED)
            return S_INVALID;
    }
    for (size_t i = 0; i < n; i++)
    {
        out[i] = current[i];
    }
    *length = n;
    *calls = used;
    return S_OK;
}

int s_replace(const unsigned char *refs, size_t n, unsigned frames,
              unsigned policy, struct s_replacement *out)
{
    if ((!refs && n) || !out || n > 32 || !frames || frames > 8 ||
        policy > S_LRU)
        return S_INVALID;
    unsigned resident[8], used = 0, hand = 0, last[8] = {0};
    struct s_replacement result = {0, {0}};
    for (size_t i = 0; i < n; i++)
    {
        result.evicted[i] = -1;
        unsigned hit = used;
        for (unsigned j = 0; j < used; j++)
            if (resident[j] == refs[i])
            {
                hit = j;
                break;
            }
        if (hit < used)
        {
            last[hit] = (unsigned)i;
            continue;
        }
        result.faults++;
        unsigned slot;
        if (used < frames)
            slot = used++;
        else
        {
            slot = hand;
            if (policy == S_LRU)
            {
                slot = 0;
                for (unsigned j = 1; j < used; j++)
                    if (last[j] < last[slot])
                        slot = j;
            }
            else if (policy == S_OPTIMAL)
            {
                size_t farthest = 0;
                slot = 0;
                for (unsigned j = 0; j < used; j++)
                {
                    size_t next = i + 1;
                    while (next < n && refs[next] != resident[j])
                        next++;
                    if (next > farthest)
                    {
                        farthest = next;
                        slot = j;
                    }
                }
            }
#ifdef S_MUTATE_REPLACE
            if (policy == S_OPTIMAL)
                slot = hand;
#endif
            result.evicted[i] = (int)resident[slot];
        }
        resident[slot] = refs[i];
        last[slot] = (unsigned)i;
        if (policy == S_FIFO || policy == S_OPTIMAL)
            hand = (slot + 1) % frames;
    }
    *out = result;
    return S_OK;
}

int s_wait_event(struct s_wait *s, unsigned actor, unsigned op)
{
    if (!s || actor < 1 || actor > 2 || op > S_SIGNAL)
        return S_INVALID;
    if (op == S_LOCK)
    {
        if (s->owner)
            return S_CONFLICT;
        s->owner = actor;
        return S_OK;
    }
    if (s->owner != actor)
        return S_CONFLICT;
    if (op == S_UNLOCK)
    {
        s->owner = 0;
        return S_OK;
    }
    if (op == S_WAIT)
    {
        if (actor != 1 || s->waiting)
            return S_CONFLICT;
        if (s->predicate)
        {
            s->ready = 1;
            return S_OK;
        }
#ifndef S_MUTATE_WAIT
        s->waiting = 1;
#endif
        s->ready = 0;
        s->owner = 0;
        return S_OK;
    }
    if (actor != 2)
        return S_CONFLICT;
    if (op == S_PUBLISH)
        s->predicate = 1;
    else if (s->waiting)
    {
        s->waiting = 0;
        s->ready = 1;
    }
    return S_OK;
}

int s_shoot_begin(struct s_shootdown *s, unsigned targets)
{
    if (!s || targets > 255)
        return S_INVALID;
    if (s->active || s->generation == UINT32_MAX)
        return S_CONFLICT;
    s->generation++;
    s->active = 1;
    s->targets = s->pending = targets;
    s->reclaimed = 0;
    return S_OK;
}

int s_shoot_ack(struct s_shootdown *s, unsigned cpu, uint32_t gen)
{
    if (!s || cpu >= 8)
        return S_INVALID;
    unsigned bit = 1u << cpu;
    if (!s->active || gen != s->generation || !(s->pending & bit))
        return S_CONFLICT;
    s->pending &= ~bit;
    return S_OK;
}

int s_shoot_reclaim(struct s_shootdown *s)
{
    if (!s)
        return S_INVALID;
    if (!s->active)
        return S_CONFLICT;
#ifndef S_MUTATE_SHOOT
    if (s->pending)
        return S_PENDING;
#endif
    s->reclaimed = 1;
    s->active = 0;
    return S_OK;
}

int s_peer_submit(struct s_peer *p, uint32_t *epoch, uint32_t *sequence)
{
    if (!p || !epoch || !sequence || epoch == sequence)
        return S_INVALID;
    if (!p->ready || p->outstanding || p->sequence == UINT32_MAX)
        return S_CONFLICT;
    p->sequence++;
    p->outstanding = 1;
    *epoch = p->epoch;
    *sequence = p->sequence;
    return S_OK;
}

int s_peer_ack(struct s_peer *p, uint32_t epoch, uint32_t sequence)
{
    if (!p)
        return S_INVALID;
    if (!p->ready || !p->outstanding || sequence != p->sequence)
        return S_CONFLICT;
#ifndef S_MUTATE_PEER
    if (epoch != p->epoch)
        return S_CONFLICT;
#else
    (void)epoch;
#endif
    p->outstanding = 0;
    return S_OK;
}

int s_peer_restart(struct s_peer *p, unsigned quiet)
{
    if (!p || quiet > 1)
        return S_INVALID;
    if (!quiet)
        return S_PENDING;
    if (p->epoch == UINT32_MAX)
        return S_CONFLICT;
    p->uncertain = p->outstanding;
    p->outstanding = 0;
    p->ready = 0;
    p->sequence = 0;
    p->epoch++;
    return S_OK;
}

int s_peer_ready(struct s_peer *p, uint32_t epoch)
{
    if (!p)
        return S_INVALID;
    if (p->ready || epoch != p->epoch)
        return S_CONFLICT;
    p->ready = 1;
    return S_OK;
}
