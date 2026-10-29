/* Separate CST5 incarnation-tagged page-reference teaching fork. */
/* Original educational implementation. Fixed capacities and explicit disk
 * encoding. No library implements the tree, page cache, log, recovery or
 * checksum mechanism. */
#include "refs.h"
#include <string.h>
#define HOME 2u
#define WAL (HOME + STORE_DATA_PAGES)
#define COMMIT (WAL + STORE_DATA_PAGES)
#define SUM (STORE_PAGE_SIZE - 4u)
#define MAGIC UINT32_C(0x43535435)
#ifndef REF_MUTANT
#define REF_MUTANT 0
#endif
#define HALF ((STORE_NODE_MAX + 1u) / 2u)
#ifndef TREE_FULL_REFRESH
#define TREE_FULL_REFRESH 0
#endif
#ifndef TREE_MUTANT
#define TREE_MUTANT 0
#endif
static void mark(store *s, uint32_t id)
{
    if (id && id < 32 && !(s->touched & (UINT32_C(1) << id)))
    {
        s->touched |= UINT32_C(1) << id;
        s->touched_ids[s->touched_count++] = (uint8_t)id;
    }
}
#ifndef STORAGE_MUTANT_NO_COMMIT_FLUSH
#define STORAGE_MUTANT_NO_COMMIT_FLUSH 0
#endif
#ifndef STORAGE_MUTANT_EARLY_RECLAIM
#define STORAGE_MUTANT_EARLY_RECLAIM 0
#endif
#ifndef STORAGE_MUTANT_BAD_SEPARATOR
#define STORAGE_MUTANT_BAD_SEPARATOR 0
#endif
static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void wr32(uint8_t *p, uint32_t x)
{
    unsigned i;
    for (i = 0; i < 4; i++)
    {
        p[i] = (uint8_t)(x >> (8 * i));
    }
}

static uint64_t rd64(const uint8_t *p)
{
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}

static void wr64(uint8_t *p, uint64_t x)
{
    wr32(p, (uint32_t)x);
    wr32(p + 4, (uint32_t)(x >> 32));
}

static uint32_t crc(const uint8_t *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    size_t i;
    unsigned j;
    for (i = 0; i < n; i++)
    {
        c ^= p[i];
        for (j = 0; j < 8; j++)
        {
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
        }
    }
    return ~c;
}

static void seal(uint8_t *p)
{
    wr32(p + SUM, crc(p, SUM));
}

static int valid(const uint8_t *p, uint32_t kind)
{
    return rd32(p) == MAGIC && rd32(p + 4) == kind &&
           rd32(p + SUM) == crc(p, SUM);
}

static void header(uint8_t *p, uint32_t kind, uint64_t gen)
{
    memset(p, 0, STORE_PAGE_SIZE);
    wr32(p, MAGIC);
    wr32(p + 4, kind);
    wr64(p + 8, gen);
}

static int cmp(const void *av, size_t an, const void *bv, size_t bn)
{
    const uint8_t *a = av, *b = bv;
    size_t i, n = an < bn ? an : bn;
    for (i = 0; i < n; i++)
    {
        if (a[i] < b[i])
        {
            return -1;
        }
        if (a[i] > b[i])
        {
            return 1;
        }
    }
    return an < bn ? -1 : an > bn ? 1 : 0;
}

static int io_valid(store_io io)
{
    return io.read && io.write && io.flush &&
           io.page_count >= STORE_IMAGE_PAGES;
}

static int readp(store *s, uint32_t p, uint8_t *b)
{
    s->reads++;
    return s->io.read(s->io.ctx, p, b) ? STORE_IO : STORE_OK;
}

static int writep(store *s, uint32_t p, const uint8_t *b)
{
    s->writes++;
    return s->io.write(s->io.ctx, p, b) ? STORE_IO : STORE_OK;
}

static int flush(store *s)
{
    s->flushes++;
    return s->io.flush(s->io.ctx) ? STORE_IO : STORE_OK;
}

static void invalidate(store *s)
{
    unsigned i;
    for (i = 0; i < STORE_CACHE_PAGES; i++)
    {
        s->cache[i].valid = 0;
    }
}

static int cached(store *s, uint32_t id, uint8_t *b)
{
    uint32_t p = (s->wal_active ? WAL : HOME) + id;
    store_cache_entry *e = &s->cache[id % STORE_CACHE_PAGES];
    int r;
    if (e->valid && e->page == p)
    {
        s->cache_hits++;
        memcpy(b, e->bytes, STORE_PAGE_SIZE);
        return STORE_OK;
    }
    r = readp(s, p, b);
    if (r)
    {
        return r;
    }
    e->valid = 1;
    e->page = p;
    memcpy(e->bytes, b, STORE_PAGE_SIZE);
    return STORE_OK;
}

static void encode_node(uint32_t id, uint64_t gen, const store_node *n,
                        uint8_t *b)
{
    unsigned i;
    header(b, 11, gen);
    wr32(b + 16, id);
    b[20] = n->leaf;
    b[21] = n->n;
    wr32(b + 24, n->next);
    for (i = 0; i < 8 && i < n->n; i++)
    {
        b[32 + i] = n->klen[i];
        b[40 + i] = n->vlen[i];
        if (n->klen[i] <= STORE_KEY_MAX)
        {
            memcpy(b + 64 + i * 16, n->key[i], n->klen[i]);
        }
        if (n->vlen[i] <= STORE_VALUE_MAX)
        {
            memcpy(b + 192 + i * 64, n->value[i], n->vlen[i]);
        }
    }
    if (!n->leaf)
    {
        for (i = 0; i < 9 && i <= n->n; i++)
        {
            wr32(b + 704 + i * 4, n->child[i]);
        }
    }
    wr64(b + 768, n->incarnation);
    wr64(b + 776, n->next_incarnation);
    if (!n->leaf)
    {
        for (i = 0; i <= n->n && i < 9; i++)
        {
            wr64(b + 784 + 8 * i, n->child_incarnation[i]);
        }
    }
    seal(b);
}

static void encode(store *s, uint32_t id, uint64_t gen, uint8_t *b)
{
    if (id && (s->used & (UINT32_C(1) << id)))
    {
        encode_node(id, gen, &s->scratch[id], b);
        return;
    }
    header(b, id == 0 ? 10 : 12, gen);
    wr32(b + 16, id);
    if (id == 0)
    {
        wr32(b + 20, s->root);
        wr32(b + 24, s->used);
        wr64(b + 32, s->root_incarnation);
        for (unsigned j = 0; j < 32; j++)
        {
            wr64(b + 64 + 8 * j,
                 REF_MUTANT == 3 && !(s->used & (UINT32_C(1) << j))
                     ? 0
                     : s->incarnations[j]);
        }
    }
    seal(b);
}

static int decode_node(uint64_t generation, uint32_t id, const uint8_t *b,
                       store_node *n)
{
    unsigned i;
    if (id == 0 || id >= STORE_DATA_PAGES || !valid(b, 11) ||
        rd32(b + 16) != id || rd64(b + 8) != generation)
    {
        return STORE_CORRUPT;
    }
    memset(n, 0, sizeof(*n));
    n->leaf = b[20];
    n->n = b[21];
    n->next = rd32(b + 24);
    n->incarnation = rd64(b + 768);
    n->next_incarnation = rd64(b + 776);
    for (unsigned j = 0; j < 9; j++)
    {
        n->child_incarnation[j] = rd64(b + 784 + 8 * j);
    }
    if (!n->incarnation ||
        (n->next ? !n->next_incarnation : n->next_incarnation != 0))
    {
        return STORE_CORRUPT;
    }
    if (n->leaf > 1 || n->n > STORE_NODE_MAX || n->next >= STORE_DATA_PAGES)
    {
        return STORE_CORRUPT;
    }
    for (i = 0; i < 8; i++)
    {
        n->klen[i] = b[32 + i];
        n->vlen[i] = b[40 + i];
        memcpy(n->key[i], b + 64 + i * 16, 16);
        memcpy(n->value[i], b + 192 + i * 64, 64);
        if (i < n->n && (n->klen[i] == 0 || n->klen[i] > STORE_KEY_MAX ||
                         n->vlen[i] > STORE_VALUE_MAX))
        {
            return STORE_CORRUPT;
        }
    }
    for (i = 0; i < 9; i++)
    {
        n->child[i] = rd32(b + 704 + i * 4);
    }
    if (!n->leaf)
    {
        for (i = 0; i <= n->n; i++)
        {
            if (!n->child[i] || n->child[i] >= STORE_DATA_PAGES ||
                !n->child_incarnation[i])
            {
                return STORE_CORRUPT;
            }
        }
    }
    if (!n->leaf && (n->next || n->next_incarnation))
    {
        return STORE_CORRUPT;
    }
    for (i = n->leaf ? 0u : n->n + 1u; i < 9; i++)
    {
        if (n->child_incarnation[i])
        {
            return STORE_CORRUPT;
        }
    }
    for (i = 1; i < n->n; i++)
    {
        if (cmp(n->key[i - 1], n->klen[i - 1], n->key[i], n->klen[i]) >= 0)
        {
            return STORE_CORRUPT;
        }
    }
    return STORE_OK;
}

int store_page_decode(uint32_t id, uint64_t generation,
                      const uint8_t bytes[STORE_PAGE_SIZE], store_node *out)
{
    if (!bytes || !out || !id || id >= STORE_DATA_PAGES)
    {
        return STORE_INVALID;
    }
    store_node node;
    int r = decode_node(generation, id, bytes, &node);
    if (r == STORE_OK)
    {
        *out = node;
    }
    return r;
}

int store_page_encode(uint32_t id, uint64_t generation, const store_node *node,
                      uint8_t out[STORE_PAGE_SIZE])
{
    if (!node || !out || !id || id >= STORE_DATA_PAGES)
    {
        return STORE_INVALID;
    }
    uint8_t bytes[STORE_PAGE_SIZE];
    store_node checked;
    encode_node(id, generation, node, bytes);
    if (decode_node(generation, id, bytes, &checked) != STORE_OK)
    {
        return STORE_INVALID;
    }
    memcpy(out, bytes, STORE_PAGE_SIZE);
    return STORE_OK;
}

int store_page_read(store *s, uint32_t id, uint8_t out[STORE_PAGE_SIZE])
{
    if (!s || !out || id >= STORE_DATA_PAGES || !io_valid(s->io))
    {
        return STORE_INVALID;
    }
    if (s->poisoned)
    {
        return STORE_STATE;
    }
    uint8_t bytes[STORE_PAGE_SIZE];
    int r = cached(s, id, bytes);
    if (r == STORE_OK)
    {
        memcpy(out, bytes, STORE_PAGE_SIZE);
    }
    return r;
}

static int decode(store *s, uint32_t id, const uint8_t *b, store_node *n)
{
    return store_page_decode(id, s->generation, b, n);
}

static int reference_ok(store *s, uint32_t id, uint64_t incarnation)
{
    return id > 0 && id < 32 && (s->used & (UINT32_C(1) << id)) &&
           incarnation &&
           (REF_MUTANT == 1 || s->incarnations[id] == incarnation);
}

static int node(store *s, uint32_t id, store_node *out)
{
    uint8_t bytes[4096];
    store_node n;
    int r = STORE_OK;
    if (!id || id >= 32 || !(s->used & (UINT32_C(1) << id)))
    {
        return STORE_CORRUPT;
    }
    if (s->active)
    {
        n = s->scratch[id];
    }
    else
    {
        r = cached(s, id, bytes);
        if (!r)
        {
            r = decode(s, id, bytes, &n);
        }
    }
    if (r)
    {
        return r;
    }
    if (!reference_ok(s, id, n.incarnation) ||
        (id == s->root && !reference_ok(s, id, s->root_incarnation)))
    {
        return STORE_CORRUPT;
    }
    if (n.leaf)
    {
        if (n.next && !reference_ok(s, n.next, n.next_incarnation))
        {
            return STORE_CORRUPT;
        }
    }
    else
    {
        for (unsigned i = 0; i <= n.n; i++)
        {
            if (!reference_ok(s, n.child[i], n.child_incarnation[i]))
            {
                return STORE_CORRUPT;
            }
        }
    }
    *out = n;
    return STORE_OK;
}

static int metadata(store *s)
{
    uint8_t b[STORE_PAGE_SIZE];
    int r = cached(s, 0, b);
    if (r)
    {
        return r;
    }
    if (!valid(b, 10) || rd64(b + 8) != s->generation || rd32(b + 16) != 0)
    {
        return STORE_CORRUPT;
    }
    s->root = rd32(b + 20);
    s->used = rd32(b + 24);
    s->root_incarnation = rd64(b + 32);
    for (unsigned j = 0; j < 32; j++)
    {
        s->incarnations[j] = rd64(b + 64 + 8 * j);
    }
    if (s->incarnations[0])
    {
        return STORE_CORRUPT;
    }
    if (!s->root || s->root >= STORE_DATA_PAGES || !(s->used & 1u) ||
        !(s->used & (UINT32_C(1) << s->root)))
    {
        return STORE_CORRUPT;
    }
    return STORE_OK;
}

static void super(uint8_t *b, uint64_t gen)
{
    header(b, 20, gen);
    wr32(b + 16, STORE_DATA_PAGES);
    seal(b);
}

static int install(store *s, const uint8_t *commit)
{
    uint8_t b[STORE_PAGE_SIZE], z[STORE_PAGE_SIZE] = {0};
    uint32_t i;
    uint64_t gen = rd64(commit + 8);
    int r;
    /* Validate the ENTIRE redo batch before changing any home page. */
    for (i = 0; i < STORE_DATA_PAGES; i++)
    {
        r = readp(s, WAL + i, b);
        if (r)
        {
            return r;
        }
        if (rd32(b + SUM) != rd32(commit + 32 + i * 4) ||
            rd32(b + SUM) != crc(b, SUM) || rd64(b + 8) != gen ||
            rd32(b + 16) != i)
        {
            return STORE_CORRUPT;
        }
    }
    if (STORAGE_MUTANT_EARLY_RECLAIM)
    {
        if (writep(s, COMMIT, z) || flush(s))
        {
            return STORE_IO;
        }
    }
    for (i = 0; i < STORE_DATA_PAGES; i++)
    {
        if (readp(s, WAL + i, b) || writep(s, HOME + i, b))
        {
            return STORE_IO;
        }
    }
    if (flush(s))
    {
        return STORE_IO;
    }
    super(b, gen);
    if (writep(s, (uint32_t)(gen & 1u), b) || flush(s))
    {
        return STORE_IO;
    }
    if (writep(s, COMMIT, z) || flush(s))
    {
        return STORE_IO;
    }
    s->checkpoint_generation = gen;
    s->generation = gen;
    s->wal_active = 0;
    invalidate(s);
    return STORE_OK;
}

int store_format(store_io io)
{
    store s;
    uint8_t b[STORE_PAGE_SIZE];
    uint32_t i;
    if (!io_valid(io))
    {
        return STORE_INVALID;
    }
    memset(&s, 0, sizeof(s));
    s.io = io;
    s.root = 1;
    s.used = 3;
    s.scratch[1].leaf = 1;
    s.incarnations[1] = s.root_incarnation = s.scratch[1].incarnation = 1;
    for (i = 0; i < STORE_DATA_PAGES; i++)
    {
        encode(&s, i, 0, b);
        if (writep(&s, HOME + i, b))
        {
            return STORE_IO;
        }
    }
    memset(b, 0, sizeof(b));
    if (writep(&s, COMMIT, b) || flush(&s))
    {
        return STORE_IO;
    }
    super(b, 0);
    if (writep(&s, 0, b) || flush(&s) || writep(&s, 1, b) || flush(&s))
    {
        return STORE_IO;
    }
    return STORE_OK;
}

int store_open(store *s, store_io io)
{
    uint8_t a[STORE_PAGE_SIZE], b[STORE_PAGE_SIZE], c[STORE_PAGE_SIZE];
    int va, vb, r;
    if (!s || !io_valid(io))
    {
        return STORE_INVALID;
    }
    memset(s, 0, sizeof(*s));
    s->io = io;
    s->poisoned = 1;
    if (readp(s, 0, a) || readp(s, 1, b) || readp(s, COMMIT, c))
    {
        return STORE_IO;
    }
    va = valid(a, 20) && rd32(a + 16) == STORE_DATA_PAGES;
    vb = valid(b, 20) && rd32(b + 16) == STORE_DATA_PAGES;
    if (!va && !vb)
    {
        return STORE_CORRUPT;
    }
    s->generation =
        (!vb || (va && rd64(a + 8) >= rd64(b + 8))) ? rd64(a + 8) : rd64(b + 8);
    s->checkpoint_generation = s->generation;
    if (valid(c, 30))
    {
        uint64_t g = rd64(c + 8);
        if (g > s->generation)
        {
            if (s->generation == UINT64_MAX || g != s->generation + 1 ||
                rd32(c + 16) != STORE_DATA_PAGES)
            {
                return STORE_CORRUPT;
            }
            r = install(s, c);
            if (r)
            {
                return r;
            }
        }
    }
    r = metadata(s);
    if (r)
    {
        return r;
    }
    s->poisoned = 0;
    r = store_check(s);
    if (r)
    {
        s->poisoned = 1;
    }
    return r;
}

int store_checkpoint(store *s)
{
    uint8_t c[STORE_PAGE_SIZE];
    int r;
    if (!s || s->poisoned || s->active)
    {
        return STORE_STATE;
    }
    if (!s->wal_active)
    {
        return STORE_OK;
    }
    r = readp(s, COMMIT, c);
    if (r)
    {
        s->poisoned = 1;
        return r;
    }
    if (!valid(c, 30) || rd64(c + 8) != s->generation)
    {
        s->poisoned = 1;
        return STORE_CORRUPT;
    }
    r = install(s, c);
    if (r)
    {
        s->poisoned = 1;
    }
    return r;
}

int store_begin(store *s)
{
    uint32_t i;
    int r;
    if (!s || s->poisoned || s->active)
    {
        return STORE_STATE;
    }
    r = s->wal_active ? store_checkpoint(s) : STORE_OK;
    if (r)
    {
        return r;
    }
    memset(s->scratch, 0, sizeof(s->scratch));
    for (i = 1; i < STORE_DATA_PAGES; i++)
    {
        if (s->used & (UINT32_C(1) << i))
        {
            r = node(s, i, &s->scratch[i]);
            if (r)
            {
                return r;
            }
        }
    }
    s->saved_root = s->root;
    s->saved_used = s->used;
    s->saved_root_incarnation = s->root_incarnation;
    memcpy(s->saved_incarnations, s->incarnations, sizeof s->incarnations);
    s->active = 1;
    return STORE_OK;
}

void store_abort(store *s)
{
    if (s && s->active)
    {
        s->root = s->saved_root;
        s->used = s->saved_used;
        s->root_incarnation = s->saved_root_incarnation;
        memcpy(s->incarnations, s->saved_incarnations, sizeof s->incarnations);
        s->active = 0;
    }
}

static uint32_t alloc_node(store *s, int leaf)
{
    uint32_t i;
    for (i = 1; i < STORE_DATA_PAGES; i++)
    {
        if (!(s->used & (UINT32_C(1) << i)) &&
            (REF_MUTANT == 2 || s->incarnations[i] < UINT64_MAX))
        {
            if (!tree_page_admit(i, leaf))
            {
                return 0;
            }
            s->used |= UINT32_C(1) << i;
            mark(s, i);
            tree_event(TREE_ALLOC, i);
            memset(&s->scratch[i], 0, sizeof(store_node));
            s->scratch[i].leaf = (uint8_t)leaf;
            if (s->incarnations[i] == UINT64_MAX)
            {
                s->incarnations[i] = 1;
            }
            else
            {
                s->incarnations[i]++;
            }
            s->scratch[i].incarnation = s->incarnations[i];
            return i;
        }
    }
    return 0;
}

static void entry(store_node *d, unsigned di, const store_node *a, unsigned ai)
{
    d->klen[di] = a->klen[ai];
    d->vlen[di] = a->vlen[ai];
    memcpy(d->key[di], a->key[ai], STORE_KEY_MAX);
    memcpy(d->value[di], a->value[ai], STORE_VALUE_MAX);
}

typedef struct store_split
{
    uint32_t right;
    uint8_t key[STORE_KEY_MAX], len;
} split;

static unsigned choose(const store_node *n, const void *k, size_t len)
{
    unsigned i = 0;
    while (i < n->n)
    {
        int c = cmp(k, len, n->key[i], n->klen[i]);
        if (STORAGE_MUTANT_BAD_SEPARATOR ? c <= 0 : c < 0)
        {
            break;
        }
        i++;
    }
    return i;
}

static int insert(store *s, uint32_t id, const void *k, size_t kl,
                  const void *v, size_t vl, split *sp, unsigned depth)
{
    store_node *n = &s->scratch[id];
    unsigned i, j;
    uint32_t right;
    store_node *q;
    split sub = {0};
    int r;
    sp->right = 0;
    mark(s, id);
    if (depth >= STORE_DATA_PAGES)
    {
        return STORE_CORRUPT;
    }
    if (n->leaf)
    {
        for (i = 0; i < n->n && cmp(n->key[i], n->klen[i], k, kl) < 0; i++)
        {
        }
        if (i < n->n && cmp(n->key[i], n->klen[i], k, kl) == 0)
        {
            n->vlen[i] = (uint8_t)vl;
            memset(n->value[i], 0, STORE_VALUE_MAX);
            if (vl)
            {
                memcpy(n->value[i], v, vl);
            }
            return STORE_OK;
        }
        for (j = n->n; j > i; j--)
        {
            entry(n, j, n, j - 1);
        }
        memset(n->key[i], 0, STORE_KEY_MAX);
        memset(n->value[i], 0, STORE_VALUE_MAX);
        memcpy(n->key[i], k, kl);
        if (vl)
        {
            memcpy(n->value[i], v, vl);
        }
        n->klen[i] = (uint8_t)kl;
        n->vlen[i] = (uint8_t)vl;
        n->n++;
    }
    else
    {
        i = choose(n, k, kl);
        r = insert(s, n->child[i], k, kl, v, vl, &sub, depth + 1);
        if (r || !sub.right)
        {
            return r;
        }
        for (j = n->n; j > i; j--)
        {
            entry(n, j, n, j - 1);
            n->child[j + 1] = n->child[j];
            n->child_incarnation[j + 1] = n->child_incarnation[j];
        }
        memcpy(n->key[i], sub.key, STORE_KEY_MAX);
        n->klen[i] = sub.len;
        n->child[i + 1] = sub.right;
        n->child_incarnation[i + 1] = s->incarnations[sub.right];
        n->n++;
    }
    if (n->n <= STORE_NODE_MAX)
    {
        return STORE_OK;
    }
    right = alloc_node(s, n->leaf);
    if (!right)
    {
        return STORE_FULL;
    }
    q = &s->scratch[right];
    if (n->leaf)
    {
        tree_event(TREE_LEAF_SPLIT, id);
        q->n = HALF;
        for (j = 0; j < HALF; j++)
        {
            entry(q, j, n, j + HALF);
        }
        q->next = n->next;
        q->next_incarnation = n->next_incarnation;
        n->next = right;
        n->next_incarnation = s->incarnations[right];
        n->n = HALF;
        sp->len = q->klen[0];
        memcpy(sp->key, q->key[0], STORE_KEY_MAX);
    }
    else
    {
        tree_event(TREE_INTERNAL_SPLIT, id);
        sp->len = n->klen[HALF];
        memcpy(sp->key, n->key[HALF], STORE_KEY_MAX);
        q->n = HALF - 1;
        for (j = 0; j < HALF - 1; j++)
        {
            entry(q, j, n, j + HALF + 1);
        }
        for (j = 0; j < HALF; j++)
        {
            q->child[j] = n->child[j + HALF + 1];
            q->child_incarnation[j] = n->child_incarnation[j + HALF + 1];
        }
        n->n = HALF;
    }
    sp->right = right;
    return STORE_OK;
}

/* Recompute separator minima after mutation. Deliberately O(allocated pages),
 * separately charged from logarithmic descent and local split/merge work. */
static int refresh(store *s, uint32_t id, uint8_t *k, uint8_t *len,
                   unsigned depth)
{
    store_node *n;
    unsigned i;
    uint8_t tmp[STORE_KEY_MAX], tl;
    if (!id || id >= STORE_DATA_PAGES || depth >= STORE_DATA_PAGES)
    {
        return STORE_CORRUPT;
    }
    tree_event(TREE_FULL_VISIT, id);
    n = &s->scratch[id];
    if (n->leaf)
    {
        if (n->n)
        {
            *len = n->klen[0];
            memcpy(k, n->key[0], *len);
        }
        else
        {
            *len = 0;
        }
        return STORE_OK;
    }
    for (i = 0; i <= n->n; i++)
    {
        int r = refresh(s, n->child[i], tmp, &tl, depth + 1);
        if (r || !tl)
        {
            return STORE_CORRUPT;
        }
        if (i == 0)
        {
            *len = tl;
            memcpy(k, tmp, tl);
        }
        else
        {
            n->klen[i - 1] = tl;
            memset(n->key[i - 1], 0, STORE_KEY_MAX);
            memcpy(n->key[i - 1], tmp, tl);
        }
    }
    return STORE_OK;
}

/* Repair only directly changed nodes. Minima descend child0 without
 * consulting stale separators; complexity O(touched * fanout * height).
 * All access is externally serialized: touched is a per-call work set. */
static int minimum(store *s, uint32_t id, uint8_t *key, uint8_t *length)
{
    for (unsigned depth = 0; depth < 32; depth++)
    {
        if (!id || id >= 32 || !(s->used & (UINT32_C(1) << id)))
        {
            return STORE_CORRUPT;
        }
        tree_event(TREE_MIN_VISIT, id);
        store_node *n = &s->scratch[id];
        if (n->leaf)
        {
            if (!n->n)
            {
                return STORE_CORRUPT;
            }
            *length = n->klen[0];
            memcpy(key, n->key[0], *length);
            return STORE_OK;
        }
        id = n->child[0];
    }
    return STORE_CORRUPT;
}

static int repair(store *s)
{
    uint8_t key[16], length = 0;
    if (TREE_MUTANT == 2)
    {
        return STORE_OK;
    }
    if (TREE_FULL_REFRESH)
    {
        return refresh(s, s->root, key, &length, 0);
    }
    for (unsigned pos = 0; pos < s->touched_count; pos++)
    {
        uint32_t id = s->touched_ids[pos];
        if (!(s->used & (UINT32_C(1) << id)))
        {
            continue;
        }
        store_node *n = &s->scratch[id];
        if (n->leaf)
        {
            continue;
        }
        tree_event(TREE_REPAIR_NODE, id);
        for (unsigned i = 1; i <= n->n; i++)
        {
            int r = minimum(s, n->child[i], key, &length);
            if (r)
            {
                return r;
            }
            if (TREE_MUTANT == 5)
            {
                key[length - 1] ^= 1u;
            }
            n->klen[i - 1] = length;
            memset(n->key[i - 1], 0, 16);
            memcpy(n->key[i - 1], key, length);
        }
    }
    return STORE_OK;
}

int store_put(store *s, const void *k, size_t kl, const void *v, size_t vl)
{
    split sp = {0};
    uint32_t root;
    int r;
    if (!s || s->poisoned || !s->active)
    {
        return STORE_STATE;
    }
    if (!k || !kl || kl > STORE_KEY_MAX || vl > STORE_VALUE_MAX || (!v && vl))
    {
        return STORE_INVALID;
    }
    s->touched = 0;
    s->touched_count = 0;
    r = insert(s, s->root, k, kl, v, vl, &sp, 0);
    if (!r && sp.right)
    {
        root = alloc_node(s, 0);
        if (!root)
        {
            r = STORE_FULL;
        }
        else
        {
            s->scratch[root].n = 1;
            s->scratch[root].child[0] = s->root;
            s->scratch[root].child[1] = sp.right;
            s->scratch[root].child_incarnation[0] = s->incarnations[s->root];
            s->scratch[root].child_incarnation[1] = s->incarnations[sp.right];
            s->scratch[root].klen[0] = sp.len;
            memcpy(s->scratch[root].key[0], sp.key, STORE_KEY_MAX);
            s->root = root;
            s->root_incarnation = s->incarnations[root];
            tree_event(TREE_ROOT_GROW, root);
        }
    }
    if (!r)
    {
        r = repair(s);
    }
    if (r)
    {
        store_abort(s);
    }
    return r;
}

static void remove_parent(store_node *p, unsigned sep)
{
    unsigned j;
    for (j = sep; j + 1 < p->n; j++)
    {
        entry(p, j, p, j + 1);
        p->child[j + 1] = p->child[j + 2];
        p->child_incarnation[j + 1] = p->child_incarnation[j + 2];
    }
    p->n--;
}

int store_delete(store *s, const void *k, size_t kl)
{
    uint32_t path[STORE_DATA_PAGES], id, par, left, right;
    unsigned at[STORE_DATA_PAGES], d = 0, i, j, min;
    store_node *n, *p, *q;
    if (!s || s->poisoned || !s->active)
    {
        return STORE_STATE;
    }
    if (!k || !kl || kl > STORE_KEY_MAX)
    {
        return STORE_INVALID;
    }
    s->touched = 0;
    s->touched_count = 0;
    id = s->root;
    while (!s->scratch[id].leaf)
    {
        mark(s, id);
        if (d >= STORE_DATA_PAGES)
        {
            return STORE_CORRUPT;
        }
        path[d] = id;
        at[d] = choose(&s->scratch[id], k, kl);
        id = s->scratch[id].child[at[d++]];
    }
    n = &s->scratch[id];
    mark(s, id);
    for (i = 0; i < n->n && cmp(n->key[i], n->klen[i], k, kl) < 0; i++)
    {
    }
    if (i == n->n || cmp(n->key[i], n->klen[i], k, kl))
    {
        return STORE_NOT_FOUND;
    }
    for (j = i; j + 1 < n->n; j++)
    {
        entry(n, j, n, j + 1);
    }
    n->n--;
    while (d)
    {
        min = n->leaf ? HALF : HALF - 1;
        if (n->n >= min)
        {
            break;
        }
        par = path[d - 1];
        i = at[d - 1];
        p = &s->scratch[par];
        left = i ? p->child[i - 1] : 0;
        right = i < p->n ? p->child[i + 1] : 0;
        if (left && s->scratch[left].n > min)
        {
            q = &s->scratch[left];
            if (TREE_MUTANT != 1)
            {
                mark(s, left);
            }
            tree_event(TREE_BORROW_LEFT, id);
            for (j = n->n; j > 0; j--)
            {
                entry(n, j, n, j - 1);
            }
            if (n->leaf)
            {
                entry(n, 0, q, q->n - 1);
            }
            else
            {
                for (j = n->n + 1; j > 0; j--)
                {
                    n->child[j] = n->child[j - 1];
                    n->child_incarnation[j] = n->child_incarnation[j - 1];
                }
                entry(n, 0, p, i - 1);
                n->child[0] = q->child[q->n];
                n->child_incarnation[0] = q->child_incarnation[q->n];
                entry(p, i - 1, q, q->n - 1);
            }
            q->n--;
            n->n++;
            break;
        }
        if (right && s->scratch[right].n > min)
        {
            q = &s->scratch[right];
            if (TREE_MUTANT != 1)
            {
                mark(s, right);
            }
            tree_event(TREE_BORROW_RIGHT, id);
            if (n->leaf)
            {
                entry(n, n->n, q, 0);
            }
            else
            {
                entry(n, n->n, p, i);
                n->child[n->n + 1] = q->child[0];
                n->child_incarnation[n->n + 1] = q->child_incarnation[0];
                entry(p, i, q, 0);
                for (j = 0; j < q->n; j++)
                {
                    q->child[j] = q->child[j + 1];
                    q->child_incarnation[j] = q->child_incarnation[j + 1];
                }
            }
            for (j = 0; j + 1 < q->n; j++)
            {
                entry(q, j, q, j + 1);
            }
            q->n--;
            n->n++;
            break;
        }
        if (left)
        {
            q = &s->scratch[left];
            if (TREE_MUTANT != 1)
            {
                mark(s, left);
            }
            tree_event(TREE_MERGE_LEFT, id);
            if (!n->leaf)
            {
                entry(q, q->n, p, i - 1);
                q->n++;
                q->child[q->n] = n->child[0];
                q->child_incarnation[q->n] = n->child_incarnation[0];
            }
            for (j = 0; j < n->n; j++)
            {
                entry(q, q->n, n, j);
                if (!n->leaf)
                {
                    q->child[q->n + 1] = n->child[j + 1];
                    q->child_incarnation[q->n + 1] =
                        n->child_incarnation[j + 1];
                }
                q->n++;
            }
            if (n->leaf && TREE_MUTANT != 3)
            {
                q->next = n->next;
                q->next_incarnation = n->next_incarnation;
            }
            s->used &= ~(UINT32_C(1) << id);
            memset(n, 0, sizeof(*n));
            remove_parent(p, i - 1);
        }
        else if (right)
        {
            q = &s->scratch[right];
            if (TREE_MUTANT != 1)
            {
                mark(s, right);
            }
            tree_event(TREE_MERGE_RIGHT, id);
            if (!n->leaf)
            {
                entry(n, n->n, p, i);
                n->n++;
                n->child[n->n] = q->child[0];
                n->child_incarnation[n->n] = q->child_incarnation[0];
            }
            for (j = 0; j < q->n; j++)
            {
                entry(n, n->n, q, j);
                if (!n->leaf)
                {
                    n->child[n->n + 1] = q->child[j + 1];
                    n->child_incarnation[n->n + 1] =
                        q->child_incarnation[j + 1];
                }
                n->n++;
            }
            if (n->leaf && TREE_MUTANT != 3)
            {
                n->next = q->next;
                n->next_incarnation = q->next_incarnation;
            }
            s->used &= ~(UINT32_C(1) << right);
            memset(q, 0, sizeof(*q));
            remove_parent(p, i);
        }
        else
        {
            return STORE_CORRUPT;
        }
        id = par;
        n = p;
        d--;
    }
    n = &s->scratch[s->root];
    if (!n->leaf && n->n == 0)
    {
        id = s->root;
        s->root = n->child[0];
        s->root_incarnation = n->child_incarnation[0];
        tree_event(TREE_ROOT_SHRINK, id);
        s->used &= ~(UINT32_C(1) << id);
        memset(n, 0, sizeof(*n));
    }
    return repair(s);
}

int store_get(store *s, const void *k, size_t kl, void *v, size_t cap,
              size_t *len)
{
    store_node n;
    uint32_t id;
    unsigned depth, i;
    int r;
    if (!s || s->poisoned)
    {
        return STORE_STATE;
    }
    if (!k || !kl || kl > STORE_KEY_MAX || !len || (!v && cap))
    {
        return STORE_INVALID;
    }
    id = s->root;
    for (depth = 0; depth < STORE_DATA_PAGES; depth++)
    {
        r = node(s, id, &n);
        if (r)
        {
            return r;
        }
        if (n.leaf)
        {
            for (i = 0; i < n.n; i++)
            {
                if (!cmp(k, kl, n.key[i], n.klen[i]))
                {
                    if (cap < n.vlen[i])
                    {
                        return STORE_FULL;
                    }
                    *len = n.vlen[i];
                    if (*len)
                    {
                        memcpy(v, n.value[i], *len);
                    }
                    return STORE_OK;
                }
            }
            return STORE_NOT_FOUND;
        }
        id = n.child[choose(&n, k, kl)];
    }
    return STORE_CORRUPT;
}

int store_scan(store *s, const void *lo, size_t kl, store_visit visit,
               void *ctx)
{
    store_node n;
    uint32_t id, seen = 0;
    unsigned d, i;
    int r;
    if (!s || s->poisoned)
    {
        return STORE_STATE;
    }
    if (kl > STORE_KEY_MAX || (!lo && kl) || !visit)
    {
        return STORE_INVALID;
    }
    id = s->root;
    for (d = 0; d < STORE_DATA_PAGES; d++)
    {
        r = node(s, id, &n);
        if (r)
        {
            return r;
        }
        if (n.leaf)
        {
            break;
        }
        id = n.child[kl ? choose(&n, lo, kl) : 0];
    }
    if (d == STORE_DATA_PAGES)
    {
        return STORE_CORRUPT;
    }
    while (id)
    {
        if (seen & (UINT32_C(1) << id))
        {
            return STORE_CORRUPT;
        }
        seen |= UINT32_C(1) << id;
        r = node(s, id, &n);
        if (r || !n.leaf)
        {
            return r ? r : STORE_CORRUPT;
        }
        for (i = 0; i < n.n; i++)
        {
            if (!kl || cmp(n.key[i], n.klen[i], lo, kl) >= 0)
            {
                if (visit(ctx, n.key[i], n.klen[i], n.value[i], n.vlen[i]))
                {
                    return STORE_OK;
                }
            }
        }
        id = n.next;
    }
    return STORE_OK;
}

int store_commit(store *s)
{
    uint8_t b[STORE_PAGE_SIZE], c[STORE_PAGE_SIZE], z[STORE_PAGE_SIZE] = {0};
    uint32_t i;
    uint64_t gen;
    if (!s || s->poisoned || !s->active)
    {
        return STORE_STATE;
    }
    if (s->generation == UINT64_MAX)
    {
        store_abort(s);
        return STORE_FULL;
    }
    gen = s->generation + 1;
    header(c, 30, gen);
    wr32(c + 16, STORE_DATA_PAGES);
    if (writep(s, COMMIT, z) || flush(s))
    {
        store_abort(s);
        s->poisoned = 1;
        return STORE_IO;
    }
    for (i = 0; i < STORE_DATA_PAGES; i++)
    {
        encode(s, i, gen, b);
        wr32(c + 32 + i * 4, rd32(b + SUM));
        if (writep(s, WAL + i, b))
        {
            store_abort(s);
            s->poisoned = 1;
            return STORE_IO;
        }
    }
    if (flush(s))
    {
        store_abort(s);
        s->poisoned = 1;
        return STORE_IO;
    }
    seal(c);
    if (writep(s, COMMIT, c) || (!STORAGE_MUTANT_NO_COMMIT_FLUSH && flush(s)))
    {
        store_abort(s);
        s->poisoned = 1;
        return STORE_INDETERMINATE;
    }
    s->active = 0;
    s->generation = gen;
    s->wal_active = 1;
    invalidate(s);
    return STORE_OK;
}

typedef struct check_state
{
    uint32_t seen, last_leaf;
    unsigned leaf_depth;
    uint8_t last[STORE_KEY_MAX];
    size_t last_len;
    int any;
} check_state;

static int check_node(store *s, uint32_t id, unsigned depth, check_state *c,
                      uint8_t *first, size_t *flen)
{
    store_node n;
    unsigned i;
    uint8_t ck[STORE_KEY_MAX];
    size_t cn = 0;
    int r;
    if (depth >= STORE_DATA_PAGES || !id || id >= STORE_DATA_PAGES ||
        (c->seen & (UINT32_C(1) << id)))
    {
        return STORE_CORRUPT;
    }
    c->seen |= UINT32_C(1) << id;
    r = node(s, id, &n);
    if (r)
    {
        return r;
    }
    if (id != s->root && n.n < (n.leaf ? HALF : HALF - 1))
    {
        return STORE_CORRUPT;
    }
    if (!n.leaf && !n.n)
    {
        return STORE_CORRUPT;
    }
    if (n.leaf)
    {
        if (c->last_leaf)
        {
            store_node prev;
            if (node(s, c->last_leaf, &prev) || prev.next != id ||
                c->leaf_depth != depth)
            {
                return STORE_CORRUPT;
            }
        }
        else
        {
            c->leaf_depth = depth;
        }
        for (i = 0; i < n.n; i++)
        {
            if (c->any && cmp(c->last, c->last_len, n.key[i], n.klen[i]) >= 0)
            {
                return STORE_CORRUPT;
            }
            memcpy(c->last, n.key[i], n.klen[i]);
            c->last_len = n.klen[i];
            c->any = 1;
        }
        c->last_leaf = id;
        *flen = n.n ? n.klen[0] : 0;
        if (*flen)
        {
            memcpy(first, n.key[0], *flen);
        }
        return STORE_OK;
    }
    for (i = 0; i <= n.n; i++)
    {
        r = check_node(s, n.child[i], depth + 1, c, ck, &cn);
        if (r || !cn)
        {
            return STORE_CORRUPT;
        }
        if (i && cmp(n.key[i - 1], n.klen[i - 1], ck, cn))
        {
            return STORE_CORRUPT;
        }
        if (!i)
        {
            *flen = cn;
            memcpy(first, ck, cn);
        }
    }
    return STORE_OK;
}

int store_check(store *s)
{
    check_state c = {0};
    uint8_t k[STORE_KEY_MAX];
    size_t n = 0;
    store_node last;
    int r;
    if (!s || s->poisoned)
    {
        return STORE_STATE;
    }
    r = check_node(s, s->root, 0, &c, k, &n);
    if (r)
    {
        return r;
    }
    if ((c.seen | 1u) != s->used)
    {
        return STORE_CORRUPT;
    }
    if (node(s, c.last_leaf, &last) || last.next)
    {
        return STORE_CORRUPT;
    }
    return STORE_OK;
}

int store_ref_make(store *s, uint32_t id, struct store_ref *out)
{
    if (!s || !out || s->poisoned || s->active)
    {
        return STORE_STATE;
    }
    store_node n;
    int r = node(s, id, &n);
    if (r)
    {
        return r;
    }
    *out = (struct store_ref){s, id, n.incarnation};
    return STORE_OK;
}

int store_ref_read(struct store_ref ref, store_node *out)
{
    store *s = ref.owner;
    if (!s || !out || s->poisoned || s->active)
    {
        return STORE_STATE;
    }
    if (!reference_ok(s, ref.id, ref.incarnation))
    {
        return STORE_STALE;
    }
    store_node n;
    int r = node(s, ref.id, &n);
    if (r)
    {
        return r;
    }
    *out = n;
    return STORE_OK;
}
