#include "diagnosis.h"
#include <limits.h>

void dx_log(struct dx_log *r, unsigned event)
{
    if (!r || !event)
        return;
    if (!r->first)
        r->first = event;
    if (r->count < 3)
        r->ring[r->count++] = event;
    else
    {
        r->ring[0] = r->ring[1];
        r->ring[1] = r->ring[2];
        r->ring[2] = event;
        if (r->dropped < UINT_MAX)
            r->dropped++;
    }
}

struct dx_cache
{
    uint16_t masks[512];
    unsigned char result[512];
    unsigned used, budget, hits;
    dx_oracle_fn oracle;
    void *arg;
};

static int trial(struct dx_cache *c, uint16_t m)
{
    for (unsigned i = 0; i < c->used; i++)
        if (c->masks[i] == m)
        {
            c->hits++;
            return c->result[i];
        }
    if (c->used == c->budget)
        return DX_UNRESOLVED;
    int r = c->oracle(m, c->arg);
    if (r < DX_PASS || r > DX_UNRESOLVED)
        r = DX_UNRESOLVED;
    c->masks[c->used] = m;
    c->result[c->used++] = (unsigned char)r;
    return r;
}

static unsigned bits(uint16_t m)
{
    unsigned n = 0;
    for (; m; m = (uint16_t)(m & (uint16_t)(m - 1)))
        n++;
    return n;
}

int dx_reduce(unsigned width, unsigned budget, dx_oracle_fn oracle, void *arg,
              struct dx_reduction *out)
{
    if (!width || width > 16 || !budget || budget > 512 || !oracle || !out)
        return -1;
    struct dx_cache c = {{0}, {0}, 0, budget, 0, oracle, arg};
    uint16_t current = (uint16_t)(UINT16_MAX >> (16 - width));
    if (trial(&c, current) != DX_FAIL)
        return -1;
    unsigned granularity = 2;
    while (bits(current) >= 2 && c.used < c.budget)
    {
        unsigned count = bits(current);
        if (granularity > count)
            granularity = count;
        uint16_t groups[16] = {0};
        unsigned ordinal = 0;
        for (unsigned bit = 0; bit < width; bit++)
            if (current & (1u << bit))
            {
                groups[ordinal * granularity / count] |= (uint16_t)(1u << bit);
                ordinal++;
            }
        int reduced = 0;
        for (unsigned complement = 0; complement < 2 && !reduced; complement++)
            for (unsigned i = 0; i < granularity; i++)
            {
                uint16_t candidate =
                    complement ? (uint16_t)(current & ~groups[i]) : groups[i];
                if (candidate != current && trial(&c, candidate) == DX_FAIL)
                {
                    current = candidate;
                    granularity =
                        complement && granularity > 2 ? granularity - 1 : 2;
                    reduced = 1;
                    break;
                }
            }
        if (!reduced)
        {
            if (granularity == count)
                break;
            granularity = granularity > count / 2 ? count : granularity * 2;
        }
    }
    *out = (struct dx_reduction){current, c.used, c.hits, c.used == c.budget};
    return 0;
}

int dx_memory(struct dx_memory *m, unsigned action, unsigned generation,
              unsigned index, unsigned extent)
{
    if (!m || !m->live || generation != m->generation ||
        action > DX_RESIZE_SUCCESS)
        return -1;
    if (action == DX_READ || action == DX_WRITE)
    {
        if (index >= m->extent || index >= 32)
            return -1;
        if (action == DX_READ)
            return (m->initialized & (1u << index)) ? 0 : -1;
        m->initialized |= 1u << index;
        return 0;
    }
    if (action == DX_FREE)
    {
        m->live = 0;
        return 0;
    }
    if (action == DX_RESIZE_FAIL)
        return 0;
    if (!extent || extent > 32 || m->generation == UINT_MAX)
        return -1;
    m->generation++;
    m->extent = extent;
    m->initialized &= extent == 32 ? UINT_MAX : (1u << extent) - 1;
    return 0;
}

unsigned dx_saturate(unsigned a, unsigned b)
{
    if (a > 255 || b > 255)
        return UINT_MAX;
    unsigned sum = a + b;
    return sum > 255 ? 255 : sum;
}

int dx_checked(unsigned a, unsigned b, unsigned *out)
{
    if (!out || a > 255 || b > 255 || b > 255 - a)
        return -1;
    *out = a + b;
    return 0;
}

int dx_close(struct dx_descriptor *d, unsigned ticket, unsigned policy,
             int result)
{
    if (!d || !d->live || d->generation != ticket || policy > 1)
        return -1;
    if (policy == 1 && result == -2)
        return 1;
    d->live = 0;
    return result ? 1 : 0;
}

int dx_write_step(struct dx_writer *w, int result, unsigned budget)
{
    if (!w || w->offset > w->total || w->attempts >= budget)
        return -1;
    w->attempts++;
    if (result > 0)
    {
        if ((size_t)result > w->total - w->offset)
            return -1;
        w->offset += (size_t)result;
        return w->offset == w->total ? 0 : 1;
    }
    return result == -1 || result == -2 ? 1 : -1;
}
