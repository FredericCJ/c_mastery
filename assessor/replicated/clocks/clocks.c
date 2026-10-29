#include "clocks.h"
#include <limits.h>
#ifndef RPC_MUTATION
#define RPC_MUTATION 0
#endif
static int valid_vector(const struct rpc_vector *v)
{
    if (!v || v->count < 1 || v->count > 6)
    {
        return 0;
    }
    for (unsigned i = 0; i < v->count; ++i)
    {
        if (!v->entries[i].identity ||
            (i && v->entries[i - 1].identity >= v->entries[i].identity))
        {
            return 0;
        }
    }
    return 1;
}

static int find(const struct rpc_vector *v, uint64_t id)
{
    for (unsigned i = 0; i < v->count; ++i)
    {
        if (v->entries[i].identity == id)
        {
            return (int)i;
        }
    }
    return -1;
}

static int valid(const struct rpc_clock *c)
{
    return c && c->uuid && c->self && valid_vector(&c->vector) &&
           find(&c->vector, c->self) >= 0;
}

static int insert(struct rpc_vector *v, uint64_t id, uint64_t value)
{
    int at = find(v, id);
    if (at >= 0)
    {
        if (value > v->entries[at].value)
        {
            v->entries[at].value = value;
        }
        return RPC_OK;
    }
    if (v->count == 6)
    {
        return RPC_FULL;
    }
    unsigned i = v->count++;
    while (i && v->entries[i - 1].identity > id)
    {
        v->entries[i] = v->entries[i - 1];
        --i;
    }
    v->entries[i] = (struct rpc_entry){id, value};
    return RPC_OK;
}

int rpc_make(uint64_t uuid, uint64_t id, struct rpc_clock *out)
{
    if (!uuid || !id || !out)
    {
        return RPC_INVALID;
    }
    struct rpc_clock c = {
        .uuid = uuid, .self = id, .vector = {.entries = {{id, 0}}, .count = 1}};
    *out = c;
    return RPC_OK;
}

int rpc_step(const struct rpc_clock *local, const struct rpc_clock *remote,
             uint64_t wall, uint64_t ahead, unsigned policy,
             struct rpc_clock *out)
{
    if (!valid(local) || !out || (remote && !valid(remote)) ||
        policy > RPC_WAIT_FOR_WALL)
    {
        return RPC_INVALID;
    }
    if (remote && remote->uuid != local->uuid)
    {
        return RPC_IDENTITY;
    }
    struct rpc_clock next = *local;
    uint64_t maximum = local->hlc.physical;
    if (remote && remote->hlc.physical > maximum)
    {
        maximum = remote->hlc.physical;
    }
    if (RPC_MUTATION != 5 && maximum > wall && maximum - wall > ahead)
    {
        return RPC_SKEW;
    }
    int r =
        rp_lamport(local->lamport, remote ? remote->lamport : 0, &next.lamport);
    if (r != RP_OK)
    {
        return RPC_RANGE;
    }
    if (remote)
    {
        for (unsigned i = 0; i < remote->vector.count; ++i)
        {
            struct rpc_entry e = remote->vector.entries[i];
            if (RPC_MUTATION == 1 && i < next.vector.count)
            {
                if (e.value > next.vector.entries[i].value)
                {
                    next.vector.entries[i].value = e.value;
                }
            }
            else
            {
                r = insert(&next.vector, e.identity, e.value);
                if (r)
                {
                    return r;
                }
            }
        }
    }
    int self = find(&next.vector, next.self);
    if (next.vector.entries[self].value == UINT64_MAX)
    {
        return RPC_RANGE;
    }
    ++next.vector.entries[self].value;
    r = rp_hlc(local->hlc, remote ? &remote->hlc : 0, wall, &next.hlc);
    if (r != RP_OK)
    {
        if (RPC_MUTATION == 6)
        {
            next.hlc.logical = 0;
        }
        else
        {
            return policy == RPC_WAIT_FOR_WALL && maximum < UINT64_MAX
                       ? RPC_WAIT
                       : RPC_RANGE;
        }
    }
    *out = next;
    return RPC_OK;
}

int rpc_replace(const struct rpc_clock *local, uint64_t id,
                struct rpc_clock *out)
{
    if (!valid(local) || !id || !out)
    {
        return RPC_INVALID;
    }
    if (find(&local->vector, id) >= 0)
    {
        return RPC_IDENTITY;
    }
    struct rpc_clock next = *local;
    if (RPC_MUTATION == 2)
    {
        int at = find(&next.vector, next.self);
        next.vector.entries[at] = (struct rpc_entry){id, 0};
    }
    else
    {
        int r = insert(&next.vector, id, 0);
        if (r)
        {
            return r;
        }
    }
    next.self = id;
    *out = next;
    return RPC_OK;
}

int rpc_order(const struct rpc_vector *a, const struct rpc_vector *b, int *out)
{
    if (!valid_vector(a) || !valid_vector(b) || !out)
    {
        return RPC_INVALID;
    }
    int less = 0, more = 0;
    for (unsigned i = 0; i < a->count; ++i)
    {
        int j = find(b, a->entries[i].identity);
        uint64_t bv = j < 0 ? 0 : b->entries[j].value;
        less |= a->entries[i].value < bv;
        more |= a->entries[i].value > bv;
    }
    for (unsigned i = 0; i < b->count; ++i)
    {
        if (find(a, b->entries[i].identity) < 0)
        {
            less |= b->entries[i].value > 0;
        }
    }
    *out = less && more ? 2 : less ? -1 : more ? 1 : 0;
    return RPC_OK;
}

static void put(unsigned char *p, uint64_t value, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        p[n - 1 - i] = (unsigned char)(value >> (8 * i));
    }
}

static uint64_t get(const unsigned char *p, unsigned n)
{
    uint64_t v = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        v = v * 256 + p[i];
    }
    return v;
}

static uint32_t crc(const unsigned char *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    for (size_t i = 0; i < n; ++i)
    {
        c ^= p[i];
        for (unsigned j = 0; j < 8; ++j)
        {
            c = (c >> 1) ^ ((c & 1) ? UINT32_C(0xedb88320) : 0);
        }
    }
    return ~c;
}

int rpc_encode(const struct rpc_clock *c, unsigned char out[160])
{
    if (!valid(c) || !out)
    {
        return RPC_INVALID;
    }
    unsigned char p[160] = {'R', 'P', 'C', '1', 1};
    p[5] = (unsigned char)c->vector.count;
    put(p + 8, c->uuid, 8);
    put(p + 16, c->self, 8);
    put(p + 24, c->lamport, 8);
    put(p + 32, c->hlc.physical, 8);
    put(p + 40, c->hlc.logical, 4);
    for (unsigned i = 0; i < c->vector.count; ++i)
    {
        put(p + 48 + 16 * i, c->vector.entries[i].identity, 8);
        put(p + 56 + 16 * i, c->vector.entries[i].value, 8);
    }
    put(p + 156, crc(p, 156), 4);
    for (unsigned i = 0; i < 160; ++i)
    {
        out[i] = p[i];
    }
    return RPC_OK;
}

int rpc_decode(uint64_t uuid, const unsigned char in[160],
               struct rpc_clock *out)
{
    if (!uuid || !in || !out)
    {
        return RPC_INVALID;
    }
    if (in[0] != 'R' || in[1] != 'P' || in[2] != 'C' || in[3] != '1' ||
        in[4] != 1 || in[5] < 1 || in[5] > 6 || in[6] || in[7])
    {
        return RPC_CORRUPT;
    }
    if (RPC_MUTATION != 3 && get(in + 156, 4) != crc(in, 156))
    {
        return RPC_CORRUPT;
    }
    if (get(in + 8, 8) != uuid)
    {
        return RPC_IDENTITY;
    }
    for (unsigned i = 44; i < 48; ++i)
    {
        if (in[i])
        {
            return RPC_CORRUPT;
        }
    }
    for (unsigned i = 48 + 16 * in[5]; i < 156; ++i)
    {
        if (in[i])
        {
            return RPC_CORRUPT;
        }
    }
    struct rpc_clock c = {.uuid = uuid,
                          .self = get(in + 16, 8),
                          .lamport = get(in + 24, 8),
                          .hlc = {get(in + 32, 8), (uint32_t)get(in + 40, 4)},
                          .vector = {.count = in[5]}};
    for (unsigned i = 0; i < c.vector.count; ++i)
    {
        c.vector.entries[i] = (struct rpc_entry){get(in + 48 + 16 * i, 8),
                                                 get(in + 56 + 16 * i, 8)};
    }
    if (!valid(&c))
    {
        return RPC_CORRUPT;
    }
    *out = c;
    return RPC_OK;
}

static int status(store *p, int r)
{
    if (r == STORE_OK)
    {
        return RPC_OK;
    }
    if (p->poisoned)
    {
        return RPC_UNCERTAIN;
    }
    return r == STORE_FULL ? RPC_FULL : r == STORE_STATE ? RPC_STATE : RPC_IO;
}

int rpc_save(store *p, const struct rpc_clock *c)
{
    if (!p || !valid(c))
    {
        return RPC_INVALID;
    }
    if (p->active || p->poisoned)
    {
        return RPC_STATE;
    }
    unsigned char bytes[160];
    int r = rpc_encode(c, bytes);
    if (r)
    {
        return r;
    }
    r = store_begin(p);
    for (unsigned i = 0; !r && i < 3; ++i)
    {
        unsigned char key[2] = {'C', (unsigned char)i};
        r = store_put(p, key, 2, bytes + 64 * i, i == 2 ? 32 : 64);
    }
    if (!r)
    {
        r = store_commit(p);
    }
    if (r)
    {
        store_abort(p);
    }
    return status(p, r);
}

int rpc_load(store *p, uint64_t uuid, struct rpc_clock *out)
{
    if (!p || !uuid || !out)
    {
        return RPC_INVALID;
    }
    if (p->active || p->poisoned)
    {
        return RPC_STATE;
    }
    unsigned char bytes[160];
    for (unsigned i = 0; i < 3; ++i)
    {
        unsigned char key[2] = {'C', (unsigned char)i};
        size_t n = 99;
        int r = store_get(p, key, 2, bytes + 64 * i, i == 2 ? 32 : 64, &n);
        if (r || n != (i == 2 ? 32u : 64u))
        {
            return r == STORE_IO ? RPC_IO : RPC_CORRUPT;
        }
    }
    return rpc_decode(uuid, bytes, out);
}

static int current(store *p, const struct rpc_clock *local)
{
    if (!p || !valid(local))
    {
        return RPC_INVALID;
    }
    struct rpc_clock stored;
    int r = rpc_load(p, local->uuid, &stored);
    if (r)
    {
        return r;
    }
    unsigned char a[160], b[160];
    if (rpc_encode(local, a) || rpc_encode(&stored, b))
    {
        return RPC_CORRUPT;
    }
    for (unsigned i = 0; i < 160; ++i)
    {
        if (a[i] != b[i])
        {
            return RPC_STATE;
        }
    }
    return RPC_OK;
}

int rpc_step_durable(store *p, const struct rpc_clock *local,
                     const struct rpc_clock *remote, uint64_t wall,
                     uint64_t ahead, unsigned policy, struct rpc_clock *out)
{
    if (!out)
    {
        return RPC_INVALID;
    }
    int r = current(p, local);
    if (r)
    {
        return r;
    }
    struct rpc_clock next;
    r = rpc_step(local, remote, wall, ahead, policy, &next);
    if (r)
    {
        return r;
    }
    if (RPC_MUTATION == 4)
    {
        *out = next;
    }
    r = rpc_save(p, &next);
    if (!r)
    {
        *out = next;
    }
    return r;
}

int rpc_replace_durable(store *p, const struct rpc_clock *local, uint64_t id,
                        struct rpc_clock *out)
{
    if (!out)
    {
        return RPC_INVALID;
    }
    int r = current(p, local);
    if (r)
    {
        return r;
    }
    struct rpc_clock next;
    r = rpc_replace(local, id, &next);
    if (r)
    {
        return r;
    }
    r = rpc_save(p, &next);
    if (!r)
    {
        *out = next;
    }
    return r;
}
