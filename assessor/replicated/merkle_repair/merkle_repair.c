#include "merkle_repair.h"
#include "hashing.h"
#include "stream.h"
#ifndef RPK_MUTATION
#define RPK_MUTATION 0
#endif
static int leaf_valid(const struct rpk_leaf *l)
{
    return l && l->length <= 32;
}

static int tree_valid(const struct rpk_tree *t)
{
    unsigned i;
    if (!t || t->count > 16)
    {
        return 0;
    }
    for (i = 0; i < t->count; ++i)
    {
        if (!leaf_valid(&t->leaves[i]))
        {
            return 0;
        }
    }
    return 1;
}

static void copy(unsigned char *a, const unsigned char *b, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; ++i)
    {
        a[i] = b[i];
    }
}

static void put(unsigned char *p, uint32_t x)
{
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        p[i] = (unsigned char)(x >> (24 - 8 * i));
    }
}

static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
           p[3];
}

static unsigned split(unsigned n)
{
    unsigned k = 1;
    while (k * 2 < n)
    {
        k *= 2;
    }
    return k;
}

static void hash(const unsigned char *b, unsigned n, unsigned char out[32],
                 struct rpk_cost *cost)
{
    (void)rph_hash(b, n, out);
    if (cost)
    {
        ++cost->hashes;
    }
}

static void leaf_hash(const struct rpk_leaf *l, unsigned char out[32],
                      struct rpk_cost *cost)
{
    unsigned char b[33] = {0};
    copy(b + 1, l->bytes, l->length);
    hash(b + (RPK_MUTATION == 1), l->length + (RPK_MUTATION != 1), out, cost);
}

static void branch(const unsigned char a[32], const unsigned char b[32],
                   unsigned char out[32], struct rpk_cost *cost)
{
    unsigned char raw[65];
    raw[0] = 1;
    copy(raw + 1, a, 32);
    copy(raw + 33, b, 32);
    hash(raw, 65, out, cost);
}

static void subtree(const struct rpk_tree *t, unsigned first, unsigned count,
                    unsigned char out[32], struct rpk_cost *cost)
{
    unsigned char a[32], b[32];
    unsigned k;
    if (!count)
    {
        hash(NULL, 0, out, cost);
        return;
    }
    if (count == 1)
    {
        leaf_hash(&t->leaves[first], out, cost);
        return;
    }
    k = split(count);
    subtree(t, first, k, a, cost);
    subtree(t, first + k, count - k, b, cost);
    branch(a, b, out, cost);
}

static void bind(uint32_t domain, unsigned count, const unsigned char raw[32],
                 unsigned char out[32], struct rpk_cost *cost)
{
    unsigned char b[41];
    b[0] = 2;
    put(b + 1, domain);
    put(b + 5, count);
    copy(b + 9, raw, 32);
    hash(b, 41, out, cost);
}

int rpk_commit(const struct rpk_tree *t, struct rpk_commitment *out)
{
    struct rpk_commitment c;
    unsigned char raw[32];
    if (!tree_valid(t) || !out)
    {
        return RPK_INVALID;
    }
    c.domain = t->domain;
    c.count = t->count;
    subtree(t, 0, t->count, raw, NULL);
    bind(t->domain, t->count, raw, c.root, NULL);
    *out = c;
    return RPK_OK;
}

static void prove(const struct rpk_tree *t, unsigned first, unsigned n,
                  unsigned index, struct rpk_proof *p)
{
    unsigned k;
    if (n == 1)
    {
        return;
    }
    k = split(n);
    if (index < k)
    {
        prove(t, first, k, index, p);
        subtree(t, first + k, n - k, p->siblings[p->depth++], NULL);
    }
    else
    {
        prove(t, first + k, n - k, index - k, p);
        subtree(t, first, k, p->siblings[p->depth++], NULL);
    }
}

int rpk_prove(const struct rpk_tree *t, unsigned index, struct rpk_proof *out)
{
    struct rpk_proof p = {0};
    if (!tree_valid(t) || !out || index >= t->count)
    {
        return RPK_INVALID;
    }
    p.domain = t->domain;
    p.count = t->count;
    p.index = index;
    prove(t, 0, t->count, index, &p);
    *out = p;
    return RPK_OK;
}

int rpk_verify(const struct rpk_leaf *leaf, const struct rpk_proof *p,
               const struct rpk_commitment *trusted)
{
    unsigned n, i, k, depth = 0, sides[4], at;
    unsigned char raw[32], next[32];
    if (!leaf_valid(leaf) || !p || !trusted || trusted->count > 16 ||
        p->depth > 4 || !p->count || p->count > 16)
    {
        return RPK_INVALID;
    }
    if (p->domain != trusted->domain || p->count != trusted->count)
    {
        return RPK_PROOF;
    }
    i = p->index;
    if (i >= p->count)
    {
        if (RPK_MUTATION != 4)
        {
            return RPK_PROOF;
        }
        i %= p->count;
    }
    n = p->count;
    while (n > 1)
    {
        k = split(n);
        sides[depth++] = i >= k;
        if (i < k)
        {
            n = k;
        }
        else
        {
            i -= k;
            n -= k;
        }
    }
    if (p->depth != depth && !(RPK_MUTATION == 3 && p->depth > depth))
    {
        return RPK_PROOF;
    }
    leaf_hash(leaf, raw, NULL);
    for (at = 0; at < depth; ++at)
    {
        unsigned side = sides[depth - at - 1];
        if (RPK_MUTATION == 2)
        {
            side = 0;
        }
        if (side)
        {
            branch(p->siblings[at], raw, next, NULL);
        }
        else
        {
            branch(raw, p->siblings[at], next, NULL);
        }
        copy(raw, next, 32);
    }
    bind(p->domain, p->count, raw, next, NULL);
    return rph_equal(next, trusted->root) ? RPK_OK : RPK_PROOF;
}

static int pack(const unsigned char *raw, unsigned length, unsigned char *wire,
                size_t cap, size_t *out)
{
    unsigned i, n = 0;
    if (cap < length + length / 8)
    {
        return RPK_FULL;
    }
    for (i = 0; i < length; i += 8)
    {
        struct rf_frame f;
        struct rf_message m;
        f.length = 8;
        copy(f.bytes, raw + i, 8);
        if (rf_message_init(&m, i / 8 + 1, 1, &f))
        {
            return RPK_PROTOCOL;
        }
        copy(wire + n, m.wire, m.wire_length);
        n += m.wire_length;
    }
    *out = n;
    return RPK_OK;
}

static int unpack(const unsigned char *wire, size_t length, unsigned chunk,
                  unsigned char *raw, unsigned expected)
{
    struct rf_parser parser;
    struct rf_frame frame;
    size_t at = 0, limit, take, used;
    unsigned n = 0;
    int r;
    if (!wire || !chunk || chunk > 108 || length > 108)
    {
        return RPK_PROTOCOL;
    }
    if (rf_parser_init(&parser, 1))
    {
        return RPK_PROTOCOL;
    }
    while (at < length)
    {
        limit = length - at;
        if (limit > chunk)
        {
            limit = chunk;
        }
        take = 0;
        while (take < limit)
        {
            used = 0;
            r = rf_feed(&parser, wire + at + take, limit - take, &used, &frame);
            if (r < 0 || !used)
            {
                return RPK_PROTOCOL;
            }
            take += used;
            if (r == 1)
            {
                if (frame.length != 8 || n + 8 > expected)
                {
                    return RPK_PROTOCOL;
                }
                copy(raw + n, frame.bytes, 8);
                n += 8;
            }
        }
        at += take;
    }
    return rf_eof(&parser) || n != expected ? RPK_PROTOCOL : RPK_OK;
}

static int range(unsigned start, unsigned n, unsigned want, unsigned count)
{
    unsigned k;
    if (start == want && n == count)
    {
        return 1;
    }
    if (n < 2)
    {
        return 0;
    }
    k = split(n);
    return range(start, k, want, count) || range(start + k, n - k, want, count);
}

int rpk_serve(const struct rpk_tree *t, const unsigned char *request,
              size_t request_length, unsigned chunk, unsigned char *reply,
              size_t capacity, size_t *reply_length)
{
    unsigned char q[24], b[96] = {0}, wire[108];
    unsigned start, n, k;
    size_t length;
    int r;
    if (!tree_valid(t) || !request || !reply || !reply_length)
    {
        return RPK_INVALID;
    }
    r = unpack(request, request_length, chunk, q, 24);
    if (r)
    {
        return r;
    }
    if (q[0] != 'R' || q[1] != 'P' || q[2] != 'Q' || q[3] != '1' ||
        get(q + 4) != t->domain || get(q + 8) != t->count || !get(q + 20))
    {
        return RPK_PROTOCOL;
    }
    start = get(q + 12);
    n = get(q + 16);
    if (!n || !range(0, t->count, start, n))
    {
        return RPK_PROTOCOL;
    }
    copy(b, q, 24);
    b[2] = 'N';
    if (n == 1)
    {
        b[24] = 1;
        b[25] = (unsigned char)t->leaves[start].length;
        copy(b + 32, t->leaves[start].bytes, b[25]);
    }
    else
    {
        b[24] = 2;
        k = split(n);
        subtree(t, start, k, b + 32, NULL);
        subtree(t, start + k, n - k, b + 64, NULL);
    }
    r = pack(b, 96, wire, sizeof wire, &length);
    if (r)
    {
        return r;
    }
    if (capacity < length)
    {
        return RPK_FULL;
    }
    copy(reply, wire, (unsigned)length);
    *reply_length = length;
    return RPK_OK;
}

struct rp_repair
{
    struct rpk_tree staged;
    const struct rpk_tree *original;
    const struct rpk_commitment *trusted;
    unsigned chunk, full;
    rpk_exchange_fn exchange;
    void *context;
    struct rpk_cost cost;
};

static int walk(struct rp_repair *r, unsigned start, unsigned n,
                const unsigned char *expected, int root)
{
    unsigned char local[32], q[24] = {0}, wire[27], reply[108], b[96], raw[32],
                             bound[32];
    size_t qn, rn = 0;
    unsigned i, k;
    int status;
    if (!r->full && r->original->domain == r->trusted->domain &&
        r->original->count == r->trusted->count)
    {
        subtree(r->original, start, n, local, &r->cost);
        if (root)
        {
            bind(r->trusted->domain, r->trusted->count, local, bound, &r->cost);
            if (RPK_MUTATION != 9 && rph_equal(bound, r->trusted->root))
            {
                return RPK_OK;
            }
        }
        else if (RPK_MUTATION != 10 && rph_equal(local, expected))
        {
            return RPK_OK;
        }
    }
    q[0] = 'R';
    q[1] = 'P';
    q[2] = 'Q';
    q[3] = '1';
    put(q + 4, r->trusted->domain);
    put(q + 8, r->trusted->count);
    put(q + 12, start);
    put(q + 16, n);
    put(q + 20, r->cost.requests + 1);
    status = pack(q, 24, wire, sizeof wire, &qn);
    if (status)
    {
        return status;
    }
    ++r->cost.requests;
    r->cost.request_bytes += (unsigned)qn;
    if (r->exchange(r->context, wire, qn, reply, sizeof reply, &rn))
    {
        return RPK_MISSING;
    }
    if (rn > sizeof reply)
    {
        return RPK_PROTOCOL;
    }
    r->cost.response_bytes += (unsigned)rn;
    status = unpack(reply, rn, r->chunk, b, 96);
    if (status)
    {
        return status;
    }
    if (b[0] != 'R' || b[1] != 'P' || b[2] != 'N' || b[3] != '1' ||
        get(b + 4) != r->trusted->domain || get(b + 8) != r->trusted->count ||
        get(b + 12) != start || get(b + 16) != n ||
        (RPK_MUTATION != 5 && get(b + 20) != r->cost.requests))
    {
        return RPK_PROTOCOL;
    }
    for (i = 26; i < 32; ++i)
    {
        if (b[i] && RPK_MUTATION != 6)
        {
            return RPK_PROTOCOL;
        }
    }
    if (n == 1)
    {
        struct rpk_leaf leaf = {0};
        if (b[24] != 1 || b[25] > 32)
        {
            return RPK_PROTOCOL;
        }
        leaf.length = b[25];
        copy(leaf.bytes, b + 32, leaf.length);
        for (i = 32 + leaf.length; i < 96; ++i)
        {
            if (b[i] && RPK_MUTATION != 6)
            {
                return RPK_PROTOCOL;
            }
        }
        leaf_hash(&leaf, raw, &r->cost);
        if (root)
        {
            bind(r->trusted->domain, r->trusted->count, raw, bound, &r->cost);
        }
        if (r->full != 2 && RPK_MUTATION != 8 &&
            !rph_equal(root ? bound : raw, root ? r->trusted->root : expected))
        {
            return RPK_PROOF;
        }
        r->staged.leaves[start] = leaf;
        ++r->cost.leaves_received;
        if (RPK_MUTATION == 7)
        {
            ((struct rpk_tree *)r->original)->leaves[start] = leaf;
        }
        return RPK_OK;
    }
    if (b[24] != 2 || b[25])
    {
        return RPK_PROTOCOL;
    }
    branch(b + 32, b + 64, raw, &r->cost);
    if (root)
    {
        bind(r->trusted->domain, r->trusted->count, raw, bound, &r->cost);
    }
    if (RPK_MUTATION != 8 &&
        !rph_equal(root ? bound : raw, root ? r->trusted->root : expected))
    {
        return RPK_PROOF;
    }
    k = split(n);
    status = walk(r, start, k, b + 32, 0);
    return status ? status : walk(r, start + k, n - k, b + 64, 0);
}

int rpk_repair(struct rpk_tree *target, const struct rpk_commitment *trusted,
               unsigned chunk, unsigned full, rpk_exchange_fn exchange,
               void *context, struct rpk_cost *cost)
{
    struct rp_repair r;
    unsigned char raw[32], bound[32];
    int status;
    unsigned i;
    if (!tree_valid(target) || !trusted || trusted->count > 16 || !chunk ||
        chunk > 108 || full > 2 || !exchange || !cost)
    {
        return RPK_INVALID;
    }
    r = (struct rp_repair){0};
    r.original = target;
    r.staged = *target;
    r.staged.count = trusted->count;
    r.staged.domain = trusted->domain;
    r.trusted = trusted;
    r.chunk = chunk;
    r.full = full;
    r.exchange = exchange;
    r.context = context;
    if (trusted->count && full == 2)
    {
        status = RPK_OK;
        for (i = 0; i < trusted->count && !status; ++i)
        {
            status = walk(&r, i, 1, NULL, 0);
        }
    }
    else if (trusted->count)
    {
        status = walk(&r, 0, trusted->count, NULL, 1);
    }
    else
    {
        hash(NULL, 0, raw, &r.cost);
        bind(trusted->domain, 0, raw, bound, &r.cost);
        status = rph_equal(bound, trusted->root) ? RPK_OK : RPK_PROOF;
    }
    if (status)
    {
        return status;
    }
    subtree(&r.staged, 0, r.staged.count, raw, &r.cost);
    bind(trusted->domain, trusted->count, raw, bound, &r.cost);
    if (RPK_MUTATION != 8 && !rph_equal(bound, trusted->root))
    {
        return RPK_PROOF;
    }
    *target = r.staged;
    *cost = r.cost;
    return RPK_OK;
}
