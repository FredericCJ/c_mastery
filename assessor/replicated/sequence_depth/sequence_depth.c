#include "sequence_depth.h"
#include "hashing.h"
#include <limits.h>
#ifndef RPS_MUTATION
#define RPS_MUTATION 0
#endif
static int scalar(uint32_t x)
{
    return x <= 0x10ffffu && (RPS_MUTATION == 2 || x < 0xd800u || x > 0xdfffu);
}

static int item(const struct rps_tree *t, unsigned id)
{
    unsigned i;
    for (i = 0; i < t->count; ++i)
    {
        if (t->items[i].id == id)
        {
            return (int)i;
        }
    }
    return -1;
}

static int rebuild(struct rps_tree *t)
{
    unsigned i, j, progress = 1;
    struct rp_sequence s = {0};
    while (progress)
    {
        progress = 0;
        for (i = 0; i < t->count; ++i)
        {
            int known = 0;
            struct rps_item *x = &t->items[i];
            if (!x->inserted)
            {
                continue;
            }
            for (j = 0; j < s.count; ++j)
            {
                if (s.nodes[j].id == x->id)
                {
                    known = 1;
                }
            }
            if (known)
            {
                continue;
            }
            if (rp_sequence_insert(&s, x->id, x->parent, (unsigned char)i) ==
                RP_OK)
            {
                if (x->removed)
                {
                    (void)rp_sequence_remove(&s, x->id);
                }
                progress = 1;
            }
        }
    }
    t->topology = s;
    return RPS_OK;
}

static int tree_valid(const struct rps_tree *t)
{
    unsigned i, j;
    if (!t || !t->epoch || t->mode > 1 || t->count > 16 ||
        t->waiting_count > 16 || t->topology.count > 16 ||
        (t->mode && t->waiting_count))
    {
        return 0;
    }
    for (i = 0; i < t->count; ++i)
    {
        const struct rps_item *x = &t->items[i];
        if (!x->id || x->inserted > 1 || x->removed > 1 ||
            (!x->inserted &&
             (!t->mode || !x->removed || x->parent || x->scalar)) ||
            (x->inserted && (x->parent >= x->id || !scalar(x->scalar))))
        {
            return 0;
        }
        for (j = 0; j < i; ++j)
        {
            if (t->items[j].id == x->id)
            {
                return 0;
            }
        }
    }
    for (i = 0; i < t->waiting_count; ++i)
    {
        if (!t->waiting[i] || item(t, t->waiting[i]) >= 0)
        {
            return 0;
        }
        for (j = 0; j < i; ++j)
        {
            if (t->waiting[j] == t->waiting[i])
            {
                return 0;
            }
        }
    }
    return 1;
}

int rps_init(struct rps_tree *t, uint64_t epoch, unsigned mode)
{
    struct rps_tree x = {0};
    if (!t || !epoch || mode > 1)
    {
        return RPS_INVALID;
    }
    x.epoch = epoch;
    x.mode = mode;
    *t = x;
    return RPS_OK;
}

int rps_insert(struct rps_tree *t, uint64_t epoch, unsigned id, unsigned parent,
               uint32_t cp)
{
    struct rps_tree x;
    int n;
    unsigned i;
    if (!tree_valid(t) || !id || parent >= id || !scalar(cp))
    {
        return RPS_INVALID;
    }
    if (epoch != t->epoch)
    {
        return RPS_STALE;
    }
    x = *t;
    n = item(&x, id);
    if (n >= 0 && x.items[n].inserted)
    {
        return x.items[n].parent == parent && x.items[n].scalar == cp
                   ? RPS_OK
                   : RPS_CONFLICT;
    }
    if (n < 0)
    {
        if (x.count == 16)
        {
            return RPS_FULL;
        }
        n = (int)x.count++;
        x.items[n] = (struct rps_item){id, 0, 0, 0, 0};
    }
    x.items[n].parent = parent;
    x.items[n].scalar = cp;
    x.items[n].inserted = 1;
    for (i = 0; i < x.waiting_count; ++i)
    {
        if (x.waiting[i] == id)
        {
            if (RPS_MUTATION != 1)
            {
                x.items[n].removed = 1;
            }
            for (; i + 1 < x.waiting_count; ++i)
            {
                x.waiting[i] = x.waiting[i + 1];
            }
            --x.waiting_count;
            break;
        }
    }
    (void)rebuild(&x);
    *t = x;
    return RPS_OK;
}

int rps_remove(struct rps_tree *t, uint64_t epoch, unsigned id)
{
    struct rps_tree x;
    int n;
    unsigned i;
    if (!tree_valid(t) || !id)
    {
        return RPS_INVALID;
    }
    if (epoch != t->epoch)
    {
        return RPS_STALE;
    }
    x = *t;
    n = item(&x, id);
    if (n >= 0)
    {
        x.items[n].removed = 1;
    }
    else if (x.mode)
    {
        if (x.count == 16)
        {
            return RPS_FULL;
        }
        x.items[x.count++] = (struct rps_item){id, 0, 0, 0, 1};
    }
    else
    {
        for (i = 0; i < x.waiting_count; ++i)
        {
            if (x.waiting[i] == id)
            {
                return RPS_OK;
            }
        }
        if (x.waiting_count == 16)
        {
            return RPS_FULL;
        }
        x.waiting[x.waiting_count++] = id;
    }
    (void)rebuild(&x);
    *t = x;
    return RPS_OK;
}

static unsigned utf8(uint32_t cp, unsigned char *out)
{
    if (cp < 128)
    {
        out[0] = (unsigned char)cp;
        return 1;
    }
    if (cp < 2048)
    {
        out[0] = (unsigned char)(0xc0 | (cp >> 6));
        out[1] = (unsigned char)(0x80 | (cp & 63));
        return 2;
    }
    if (cp < 65536)
    {
        out[0] = (unsigned char)(0xe0 | (cp >> 12));
        out[1] = (unsigned char)(0x80 | ((cp >> 6) & 63));
        out[2] = (unsigned char)(0x80 | (cp & 63));
        return 3;
    }
    out[0] = (unsigned char)(0xf0 | (cp >> 18));
    out[1] = (unsigned char)(0x80 | ((cp >> 12) & 63));
    out[2] = (unsigned char)(0x80 | ((cp >> 6) & 63));
    out[3] = (unsigned char)(0x80 | (cp & 63));
    return 4;
}

int rps_render(const struct rps_tree *t, unsigned char *out, size_t capacity,
               size_t *length)
{
    unsigned char order[16], bytes[64];
    size_t n = 0, k = 0, i;
    struct rps_tree x;
    if (!tree_valid(t) || !length || (!out && capacity))
    {
        return RPS_INVALID;
    }
    x = *t;
    (void)rebuild(&x);
    (void)rp_sequence_render(&x.topology, order, 16, &n);
    for (i = 0; i < n; ++i)
    {
        k += utf8(x.items[order[i]].scalar, bytes + k);
    }
    if (k > capacity)
    {
        return RPS_FULL;
    }
    for (i = 0; i < k; ++i)
    {
        out[i] = bytes[i];
    }
    *length = k;
    return RPS_OK;
}

static void put32(unsigned char *p, uint32_t x)
{
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        p[i] = (unsigned char)(x >> (24 - 8 * i));
    }
}

static uint32_t get32(const unsigned char *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
           p[3];
}

static void put64(unsigned char *p, uint64_t x)
{
    put32(p, (uint32_t)(x >> 32));
    put32(p + 4, (uint32_t)x);
}

static uint64_t get64(const unsigned char *p)
{
    return (uint64_t)get32(p) << 32 | get32(p + 4);
}

int rps_snapshot(const struct rps_tree *t, unsigned char out[368])
{
    unsigned char b[368] = {0};
    unsigned i, j;
    struct rps_tree x;
    if (!tree_valid(t) || !out)
    {
        return RPS_INVALID;
    }
    x = *t;
    for (i = 0; i < x.count; ++i)
    {
        for (j = i + 1; j < x.count; ++j)
        {
            if (x.items[i].id > x.items[j].id)
            {
                struct rps_item a = x.items[i];
                x.items[i] = x.items[j];
                x.items[j] = a;
            }
        }
    }
    for (i = 0; i < x.waiting_count; ++i)
    {
        for (j = i + 1; j < x.waiting_count; ++j)
        {
            if (x.waiting[i] > x.waiting[j])
            {
                unsigned a = x.waiting[i];
                x.waiting[i] = x.waiting[j];
                x.waiting[j] = a;
            }
        }
    }
    b[0] = 'R';
    b[1] = 'P';
    b[2] = 'S';
    b[3] = '1';
    put64(b + 4, x.epoch);
    b[12] = (unsigned char)x.mode;
    b[13] = (unsigned char)x.count;
    b[14] = (unsigned char)x.waiting_count;
    for (i = 0; i < x.count; ++i)
    {
        unsigned char *p = b + 16 + 16 * i;
        put32(p, x.items[i].id);
        put32(p + 4, x.items[i].parent);
        put32(p + 8, x.items[i].scalar);
        p[12] = (unsigned char)x.items[i].inserted;
        p[13] = (unsigned char)x.items[i].removed;
    }
    for (i = 0; i < x.waiting_count; ++i)
    {
        put32(b + 272 + 4 * i, x.waiting[i]);
    }
    (void)rph_hash(b, 336, b + 336);
    for (i = 0; i < 368; ++i)
    {
        out[i] = b[i];
    }
    return RPS_OK;
}

int rps_restore(const unsigned char b[368], struct rps_tree *out)
{
    struct rps_tree t = {0};
    unsigned char check[368];
    unsigned i;
    if (!b || !out)
    {
        return RPS_INVALID;
    }
    if (b[0] != 'R' || b[1] != 'P' || b[2] != 'S' || b[3] != '1' ||
        b[13] > 16 || b[14] > 16)
    {
        return RPS_INVALID;
    }
    t.epoch = get64(b + 4);
    t.mode = b[12];
    t.count = b[13];
    t.waiting_count = b[14];
    for (i = 0; i < t.count; ++i)
    {
        const unsigned char *p = b + 16 + 16 * i;
        t.items[i] = (struct rps_item){get32(p), get32(p + 4), get32(p + 8),
                                       p[12], p[13]};
    }
    for (i = 0; i < t.waiting_count; ++i)
    {
        t.waiting[i] = get32(b + 272 + 4 * i);
    }
    if (!tree_valid(&t))
    {
        return RPS_INVALID;
    }
    (void)rps_snapshot(&t, check);
    for (i = 0; i < 368; ++i)
    {
        if (b[i] != check[i])
        {
            return RPS_INVALID;
        }
    }
    (void)rebuild(&t);
    *out = t;
    return RPS_OK;
}

int rps_compact(struct rps_tree *a, struct rps_tree *b, struct rps_tree *c,
                size_t outstanding)
{
    struct rps_tree next[3], *all[3] = {a, b, c};
    unsigned char wire[3][368], order[16];
    size_t n = 0;
    unsigned i, j;
    if (a == b || a == c || b == c || !tree_valid(a) || !tree_valid(b) ||
        !tree_valid(c))
    {
        return RPS_INVALID;
    }
    if (outstanding && RPS_MUTATION != 3)
    {
        return RPS_BUSY;
    }
    if (a->epoch == UINT64_MAX)
    {
        return RPS_RANGE;
    }
    for (i = 0; i < 3; ++i)
    {
        struct rps_tree x = *all[i];
        (void)rebuild(&x);
        if (x.waiting_count || x.topology.count != x.count)
        {
            return RPS_GAP;
        }
        (void)rps_snapshot(&x, wire[i]);
        wire[i][12] = 0;
    }
    for (i = 1; i < 3; ++i)
    {
        for (j = 0; j < 336; ++j)
        {
            if (wire[i][j] != wire[0][j])
            {
                return RPS_GAP;
            }
        }
    }
    (void)rp_sequence_render(&a->topology, order, 16, &n);
    for (i = 0; i < 3; ++i)
    {
        (void)rps_init(&next[i], a->epoch + (RPS_MUTATION == 4 ? 0 : 1),
                       all[i]->mode);
        for (j = 0; j < n; ++j)
        {
            (void)rps_insert(&next[i], next[i].epoch, j + 1, j,
                             a->items[order[j]].scalar);
        }
    }
    for (i = 0; i < 3; ++i)
    {
        *all[i] = next[i];
    }
    return RPS_OK;
}

static int sameop(struct rps_op a, struct rps_op b)
{
    return a.kind == b.kind && a.position == b.position && a.scalar == b.scalar;
}

static int same_request(const struct rps_request *a,
                        const struct rps_request *b)
{
    unsigned i;
    if (a->epoch != b->epoch || a->base != b->base || a->site != b->site ||
        a->sequence != b->sequence || !sameop(a->op, b->op))
    {
        return 0;
    }
    for (i = 0; i < 3; ++i)
    {
        if (a->vector[i] != b->vector[i])
        {
            return 0;
        }
    }
    return 1;
}

static int same_commit(const struct rps_commit *a, const struct rps_commit *b)
{
    unsigned i;
    if (a->epoch != b->epoch || a->revision != b->revision ||
        a->site != b->site || a->sequence != b->sequence ||
        !sameop(a->op, b->op))
    {
        return 0;
    }
    for (i = 0; i < 3; ++i)
    {
        if (a->before[i] != b->before[i])
        {
            return 0;
        }
    }
    return 1;
}

static int apply(uint32_t *doc, unsigned *n, struct rps_op op)
{
    unsigned i;
    if (op.kind == 0)
    {
        return RPS_OK;
    }
    if (op.kind == 1)
    {
        if (!scalar(op.scalar) || op.position > *n)
        {
            return RPS_INVALID;
        }
        if (*n == 32)
        {
            return RPS_FULL;
        }
        for (i = *n; i > op.position; --i)
        {
            doc[i] = doc[i - 1];
        }
        doc[op.position] = op.scalar;
        ++*n;
        return RPS_OK;
    }
    if (op.kind != 2 || op.scalar || op.position >= *n)
    {
        return RPS_INVALID;
    }
    for (i = op.position; i + 1 < *n; ++i)
    {
        doc[i] = doc[i + 1];
    }
    --*n;
    return RPS_OK;
}

int rps_server_init(struct rps_server *s, uint64_t epoch)
{
    struct rps_server t = {0};
    if (!s || !epoch)
    {
        return RPS_INVALID;
    }
    t.epoch = epoch;
    *s = t;
    return RPS_OK;
}

int rps_client_init(struct rps_client *c, uint64_t epoch, unsigned site)
{
    struct rps_client t = {0};
    if (!c || !epoch || site > 2)
    {
        return RPS_INVALID;
    }
    t.epoch = epoch;
    t.site = site;
    *c = t;
    return RPS_OK;
}

int rps_prepare(struct rps_client *c, unsigned kind, unsigned position,
                uint32_t cp, struct rps_request *out)
{
    struct rps_request q = {0};
    unsigned i;
    if (!c || !out || !c->epoch || c->site > 2 || c->revision > 16 ||
        c->length > 32 || kind < 1 || kind > 2)
    {
        return RPS_INVALID;
    }
    if (c->pending)
    {
        return RPS_BUSY;
    }
    if (c->vector[c->site] == UINT_MAX)
    {
        return RPS_RANGE;
    }
    if ((kind == 1 && (!scalar(cp) || position > c->length)) ||
        (kind == 2 && (cp || position >= c->length)))
    {
        return RPS_INVALID;
    }
    q.epoch = c->epoch;
    q.base = c->revision;
    q.site = c->site;
    q.sequence = c->vector[c->site] + 1;
    q.op = (struct rps_op){kind, position, cp};
    for (i = 0; i < 3; ++i)
    {
        q.vector[i] = c->vector[i];
    }
    c->request = q;
    c->pending = 1;
    *out = q;
    return RPS_OK;
}

int rps_submit(struct rps_server *s, const struct rps_request *q,
               struct rps_commit *out)
{
    struct rps_server t;
    struct rps_commit c = {0};
    struct rps_op op;
    unsigned i, j, n;
    uint32_t scratch[32];
    int r;
    if (!s || !q || !out || !s->epoch || s->count > 16 || q->site > 2 ||
        q->op.kind < 1 || q->op.kind > 2)
    {
        return RPS_INVALID;
    }
    if (q->epoch != s->epoch)
    {
        return RPS_STALE;
    }
    for (i = 0; i < s->count; ++i)
    {
        if (s->originals[i].site == q->site &&
            s->originals[i].sequence == q->sequence)
        {
            if (!same_request(&s->originals[i], q))
            {
                return RPS_CONFLICT;
            }
            *out = s->commits[i];
            return RPS_OK;
        }
    }
    if (q->base > s->count)
    {
        return RPS_GAP;
    }
    for (i = 0; i < 3; ++i)
    {
        if (RPS_MUTATION != 5 && q->vector[i] != s->vectors[q->base][i])
        {
            return RPS_GAP;
        }
    }
    if (q->sequence != s->vectors[s->count][q->site] + 1 ||
        s->vectors[q->base][q->site] != s->vectors[s->count][q->site])
    {
        return RPS_GAP;
    }
    if (s->count == 16)
    {
        return RPS_FULL;
    }
    n = s->lengths[q->base];
    for (i = 0; i < n; ++i)
    {
        scratch[i] = s->documents[q->base][i];
    }
    r = apply(scratch, &n, q->op);
    if (r)
    {
        return r;
    }
    op = q->op;
    for (i = q->base; i < s->count && op.kind; ++i)
    {
        struct rps_op past = s->commits[i].op;
        if (RPS_MUTATION == 6 || !past.kind)
        {
            continue;
        }
        if (op.kind == 1 && past.kind == 1)
        {
            unsigned p;
            int v = rp_ot_insert(op.position, q->site, past.position,
                                 s->commits[i].site, &p);
            if (v)
            {
                return RPS_INVALID;
            }
            op.position = p;
        }
        else if (op.kind == 1 && past.kind == 2)
        {
            if (op.position > past.position)
            {
                --op.position;
            }
        }
        else if (op.kind == 2 && past.kind == 1)
        {
            if (op.position >= past.position)
            {
                ++op.position;
            }
        }
        else if (op.position == past.position)
        {
            if (RPS_MUTATION != 9)
            {
                op = (struct rps_op){0, 0, 0};
            }
        }
        else if (op.position > past.position)
        {
            --op.position;
        }
    }
    t = *s;
    n = t.lengths[t.count];
    for (i = 0; i < n; ++i)
    {
        t.documents[t.count + 1][i] = t.documents[t.count][i];
    }
    r = apply(t.documents[t.count + 1], &n, op);
    if (r)
    {
        return r;
    }
    t.lengths[t.count + 1] = n;
    c.epoch = s->epoch;
    c.revision = s->count + 1;
    c.site = q->site;
    c.sequence = q->sequence;
    c.op = op;
    for (j = 0; j < 3; ++j)
    {
        c.before[j] = s->vectors[s->count][j];
        t.vectors[t.count + 1][j] = c.before[j] + (j == q->site);
    }
    t.originals[t.count] = *q;
    t.commits[t.count] = c;
    ++t.count;
    *s = t;
    *out = c;
    return RPS_OK;
}

int rps_deliver(struct rps_client *c, const struct rps_commit *q)
{
    struct rps_client t;
    unsigned i;
    int r;
    if (!c || !q || !c->epoch || c->revision > 16 || c->length > 32 ||
        q->site > 2 || !q->revision || q->revision > 16)
    {
        return RPS_INVALID;
    }
    if (q->epoch != c->epoch)
    {
        return RPS_STALE;
    }
    if (q->revision <= c->revision)
    {
        return RPS_MUTATION == 8 || same_commit(&c->commits[q->revision - 1], q)
                   ? RPS_OK
                   : RPS_CONFLICT;
    }
    if (q->revision != c->revision + 1 && RPS_MUTATION != 7)
    {
        return RPS_GAP;
    }
    for (i = 0; i < 3; ++i)
    {
        if (q->before[i] != c->vector[i] && RPS_MUTATION != 7)
        {
            return RPS_GAP;
        }
    }
    if (q->sequence != q->before[q->site] + 1)
    {
        return RPS_GAP;
    }
    if (q->site == c->site &&
        (!c->pending || q->sequence != c->request.sequence))
    {
        return RPS_CONFLICT;
    }
    t = *c;
    r = apply(t.document, &t.length, q->op);
    if (r)
    {
        return r;
    }
    for (i = 0; i < 3; ++i)
    {
        t.vector[i] = q->before[i] + (i == q->site);
    }
    t.revision = q->revision;
    t.commits[q->revision - 1] = *q;
    if (q->site == c->site || RPS_MUTATION == 10)
    {
        t.pending = 0;
    }
    *c = t;
    return RPS_OK;
}
