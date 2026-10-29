#include "b04.h"
#include <limits.h>

int b4_serial(uint32_t a, uint32_t b, unsigned bits, int *out)
{
    if (!out || bits < 2 || bits > 32)
        return B4_INVALID;
    uint32_t mask = bits == 32 ? UINT32_MAX : (UINT32_C(1) << bits) - 1;
    if (a > mask || b > mask)
        return B4_RANGE;
    uint32_t diff = (b - a) & mask, half = UINT32_C(1) << (bits - 1);
    if (diff == half)
        return B4_CONFLICT;
#ifdef B4_MUTATE_SERIAL
    *out = a < b ? -1 : a > b ? 1 : 0;
#else
    *out = diff == 0 ? 0 : diff < half ? -1 : 1;
#endif
    return B4_OK;
}

int b4_window(uint32_t base, uint32_t seq, uint32_t span, unsigned bits,
              int *out)
{
    if (!out || bits < 2 || bits > 32)
        return B4_INVALID;
    uint32_t mask = bits == 32 ? UINT32_MAX : (UINT32_C(1) << bits) - 1;
    if (base > mask || seq > mask || span >= (UINT32_C(1) << (bits - 1)))
        return B4_RANGE;
    *out = ((seq - base) & mask) < span;
    return B4_OK;
}

int b4_certificate(const struct b4_graph *g, unsigned s, const uint32_t *d,
                   const unsigned *p)
{
    if (!g || !d || !p || g->vertices == 0 || g->vertices > 6 ||
        s >= g->vertices)
        return B4_INVALID;
    for (unsigned u = 0; u < g->vertices; u++)
        for (unsigned v = 0; v < g->vertices; v++)
            if (g->cost[u][v] != UINT32_MAX && g->cost[u][v] > 1000000)
                return B4_RANGE;
    if (d[s] != 0 || p[s] != UINT_MAX)
        return B4_CONFLICT;
    for (unsigned v = 0; v < g->vertices; v++)
    {
        if (v != s && d[v] != UINT32_MAX)
        {
            unsigned u = p[v];
            if (u >= g->vertices || d[u] == UINT32_MAX ||
                g->cost[u][v] == UINT32_MAX ||
                g->cost[u][v] > UINT32_MAX - d[u] ||
                d[u] + g->cost[u][v] != d[v])
                return B4_CONFLICT;
            /* Parent equality alone admits zero-cost cycles disconnected from s. */
            unsigned x = v, steps = 0;
            while (x != s && steps++ < g->vertices)
            {
                if (p[x] >= g->vertices)
                    return B4_CONFLICT;
                x = p[x];
            }
            if (x != s)
                return B4_CONFLICT;
        }
        else if (v != s && p[v] != UINT_MAX)
            return B4_CONFLICT;
    }
#ifndef B4_MUTATE_CERTIFICATE
    for (unsigned u = 0; u < g->vertices; u++)
        for (unsigned v = 0; v < g->vertices; v++)
        {
            uint32_t w = g->cost[u][v];
            if (d[u] != UINT32_MAX && w != UINT32_MAX &&
                w < UINT32_MAX - d[u] && d[v] > d[u] + w)
                return B4_CONFLICT;
        }
#endif
    return B4_OK;
}

static unsigned lower(unsigned x)
{
    return x >= 'A' && x <= 'Z' ? x + ('a' - 'A') : x;
}

static int word(const unsigned char *p, size_t n, const char *w)
{
    size_t i = 0;
    for (; i < n && w[i]; i++)
        if (lower(p[i]) != (unsigned char)w[i])
            return 0;
    return i == n && !w[i];
}

int b4_framing(const unsigned char *p, size_t n, size_t limit,
               struct b4_framing *out)
{
    if (!p || !out || n > 512)
        return B4_INVALID;
    size_t pos = 0, body = 0;
    unsigned cl = 0, te = 0;
    while (pos < n)
    {
        size_t start = pos;
        while (pos < n && p[pos] != '\r' && p[pos] != '\n')
            pos++;
        if (pos + 1 >= n || p[pos] != '\r' || p[pos + 1] != '\n')
            return B4_INVALID;
        size_t stop = pos;
        pos += 2;
        if (stop == start)
        {
            if (pos != n)
                return B4_INVALID;
#ifndef B4_MUTATE_FRAMING
            if (cl && te)
                return B4_CONFLICT;
#endif
            out->chunked = te;
            out->body = body;
            return B4_OK;
        }
        size_t colon = start;
        while (colon < stop && p[colon] != ':')
        {
            unsigned c = p[colon];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-'))
                return B4_INVALID;
            colon++;
        }
        if (colon == start || colon == stop)
            return B4_INVALID;
        size_t first = colon + 1, last = stop;
        while (first < last && (p[first] == ' ' || p[first] == '\t'))
            first++;
        while (last > first && (p[last - 1] == ' ' || p[last - 1] == '\t'))
            last--;
        if (word(p + start, colon - start, "content-length"))
        {
            if (cl++)
                return B4_CONFLICT;
            if (first == last)
                return B4_INVALID;
            for (size_t i = first; i < last; i++)
            {
                if (p[i] < '0' || p[i] > '9')
                    return B4_INVALID;
                unsigned digit = p[i] - '0';
                if (digit > limit || body > (limit - digit) / 10)
                    return B4_RANGE;
                body = body * 10 + digit;
            }
        }
        else if (word(p + start, colon - start, "transfer-encoding"))
        {
            if (te++ || !word(p + first, last - first, "chunked"))
                return B4_CONFLICT;
        }
        else
        {
            /* This original boundary accepts no other fields; not a full HTTP parser. */
            return B4_INVALID;
        }
    }
    return B4_INVALID;
}

int b4_dma(struct b4_dma *d, unsigned event, uint32_t gen)
{
    if (!d || event > B4_RELEASE)
        return B4_INVALID;
    if (gen != d->generation)
        return B4_STATE;
    switch (event)
    {
        case B4_WRITE:
            if (d->owner != B4_CPU)
                return B4_STATE;
            d->dirty = 1;
            d->valid = 1;
            break;
        case B4_CLEAN:
            if (d->owner != B4_CPU)
                return B4_STATE;
            d->dirty = 0;
            break;
        case B4_SUBMIT:
            if (d->owner != B4_CPU)
                return B4_STATE;
#ifndef B4_MUTATE_DMA
            if (d->dirty)
                return B4_STATE;
#endif
            d->owner = B4_DEVICE;
            d->valid = 0;
            break;
        case B4_COMPLETE:
            if (d->owner != B4_DEVICE)
                return B4_STATE;
            d->owner = B4_DONE;
            break;
        case B4_INVALIDATE:
            if (d->owner != B4_DONE)
                return B4_STATE;
            d->valid = 1;
            break;
        case B4_RELEASE:
            if (d->owner != B4_DONE || !d->valid || d->generation == UINT32_MAX)
                return B4_STATE;
            d->owner = B4_CPU;
            d->generation++;
            break;
    }
    return B4_OK;
}

int b4_cache_extent(size_t offset, size_t n, size_t line, size_t cap,
                    size_t *first, size_t *last)
{
    if (!first || !last || first == last || !line || (line & (line - 1)))
        return B4_INVALID;
    if (offset > cap || n > cap - offset)
        return B4_RANGE;
    if (!n)
    {
        *first = offset;
        *last = offset;
        return B4_OK;
    }
    size_t end = offset + n, padding = (line - (end & (line - 1))) & (line - 1);
    if (padding > cap - end)
        return B4_RANGE;
    *first = offset & ~(line - 1);
    *last = end + padding;
    return B4_OK;
}

int b4_memory(struct b4_ledger *l, unsigned op, uint32_t id, size_t bytes,
              int succeeded)
{
    if (!l || op > B4_FREE || !id)
        return B4_INVALID;
    unsigned index = 4, free_slot = 4;
    for (unsigned i = 0; i < 4; i++)
    {
        if (l->slots[i].live && l->slots[i].identity == id)
            index = i;
        if (!l->slots[i].live && free_slot == 4)
            free_slot = i;
    }
    if (op == B4_ALLOC)
    {
        if (index != 4)
            return B4_STATE;
        if (!succeeded)
            return B4_OK;
        if (free_slot == 4 || bytes > SIZE_MAX - l->live)
            return B4_RANGE;
        index = free_slot;
        l->slots[index].identity = id;
        l->slots[index].live = 1;
        l->slots[index].bytes = bytes;
        l->live += bytes;
    }
    else
    {
        if (index == 4)
            return B4_STATE;
        if (op == B4_RESIZE && !succeeded)
        {
#ifdef B4_MUTATE_MEMORY
            l->live -= l->slots[index].bytes;
            l->slots[index].live = 0;
#endif
            return B4_OK;
        }
        size_t remaining = l->live - l->slots[index].bytes;
        if (op == B4_FREE)
        {
            l->live = remaining;
            l->slots[index].live = 0;
        }
        else
        {
            if (bytes > SIZE_MAX - remaining)
                return B4_RANGE;
            l->slots[index].bytes = bytes;
            l->live = remaining + bytes;
        }
    }
    if (l->live > l->peak)
        l->peak = l->live;
    return B4_OK;
}
