#include "cache.h"
#include <string.h>
#ifndef SC_MUTANT
#define SC_MUTANT 0
#endif
int sc_init(struct sc_cache *c, unsigned policy)
{
    if (!c || policy > SC_PIN_ROOT)
    {
        return STORE_INVALID;
    }
    struct sc_cache next = {0};
    next.policy = policy;
    *c = next;
    return STORE_OK;
}

int sc_read(struct sc_cache *c, struct sc_view v, uint32_t logical,
            uint8_t out[STORE_PAGE_SIZE])
{
    if (!c || !out || c->policy > SC_PIN_ROOT || !v.io.read ||
        v.io.page_count < STORE_IMAGE_PAGES || !v.identity || !v.root ||
        v.root >= 32 || (v.wal != 0 && v.wal != 1) || logical >= 32)
    {
        return STORE_INVALID;
    }
    unsigned first, count = 1, set = logical % 2;
    uint32_t physical = (v.wal ? 34u : 2u) + logical;
    if (c->policy == SC_DIRECT)
    {
        first = logical % 4;
    }
    else if (c->policy == SC_TWO_WAY)
    {
        first = 2 * set;
        count = 2;
    }
    else
    {
        first = logical == v.root ? 0 : 1 + logical % 3;
    }
    for (unsigned i = first; i < first + count; i++)
    {
        struct sc_entry *e = &c->entries[i];
        if (e->valid && (SC_MUTANT == 1 || e->physical == physical) &&
            (SC_MUTANT == 2 || e->generation == v.generation) &&
            (SC_MUTANT == 3 || e->identity == v.identity))
        {
            if (c->hits == UINT64_MAX)
            {
                return STORE_FULL;
            }
            c->hits++;
            if (c->policy == SC_TWO_WAY)
            {
                c->recent[set] = i - first;
            }
            memcpy(out, e->bytes, STORE_PAGE_SIZE);
            return STORE_OK;
        }
    }
    if (c->reads == UINT64_MAX)
    {
        return STORE_FULL;
    }
    uint8_t bytes[STORE_PAGE_SIZE] = {0};
    c->reads++;
    int status = v.io.read(v.io.ctx, physical, bytes);
    if (status && SC_MUTANT != 4)
    {
        return STORE_IO;
    }
    unsigned chosen = first;
    if (count == 2)
    {
        if (!c->entries[first].valid)
        {
            chosen = first;
        }
        else if (!c->entries[first + 1].valid)
        {
            chosen = first + 1;
        }
        else
        {
            chosen =
                first + (SC_MUTANT == 5 ? c->recent[set] : 1 - c->recent[set]);
        }
    }
    struct sc_entry *e = &c->entries[chosen];
    e->valid = 1;
    e->identity = v.identity;
    e->generation = v.generation;
    e->physical = physical;
    memcpy(e->bytes, bytes, STORE_PAGE_SIZE);
    if (count == 2)
    {
        c->recent[set] = chosen - first;
    }
    memcpy(out, bytes, STORE_PAGE_SIZE);
    return status ? STORE_IO : STORE_OK;
}
