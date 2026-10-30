/*
 * Reference implementations of the indexed containers: open-addressing hash,
 * byte-keyed map, trie, sparse set, disjoint sets with undo, CSR graph,
 * Bloom-style filters, chained hash and bit-set helpers.
 *
 * Shared contract (full text in labs/containers_depth/CONTRACTS.md): controls
 * are caller-owned and externally serialized, every capacity is a small fixed
 * bound, and a refused operation (CD_INVALID, CD_FULL, CD_MISSING, ...) leaves
 * the structure unchanged unless a function below says otherwise.
 *
 * CD_MUTANT selects one deliberately wrong variant for the mutation campaign.
 */
#include "containers.h"

#ifndef CD_MUTANT
#define CD_MUTANT 0
#endif

/* Open-addressing bucket states.  A tombstone keeps probe chains unbroken
 * after a delete; only an EMPTY bucket ends a search. */
enum
{
    BUCKET_EMPTY = 0,
    BUCKET_LIVE = 1,
    BUCKET_TOMBSTONE = 2
};

enum
{
    HASH_MAX_CAPACITY = 16,
    BYTE_KEY_MAX = 8,
    BYTEMAP_SLOTS = 4,
    BYTEMAP_HASH_MODULUS = 4,
    TRIE_NODES = 64,
    SPARSE_UNIVERSE = 32,
    SPARSE_MAX_COUNT = 16,
    SPARSE_LAST_GENERATION = 3,
    SETS_MAX_ITEMS = 16,
    SETS_LOG_CAPACITY = 128,
    CSR_MAX_VERTICES = 8,
    CSR_MAX_EDGES = 32,
    FILTER_KEYS = 16,
    FILTER_CELLS = 8,
    FILTER_CELL_LIMIT = 3,
    CHAIN_BUCKETS = 4,
    CHAIN_NODES = 16,
    BITSET_WIDTH = 32
};

static int hash_valid(const struct cd_hash *h)
{
    return h && h->slots && h->capacity && h->capacity <= HASH_MAX_CAPACITY &&
           h->live <= h->occupied && h->occupied <= h->capacity &&
           h->allocator.take && h->allocator.release;
}

/* First probe position.  The clustered mode maps every key to bucket 0 to
 * produce worst-case probe sequences on purpose. */
static unsigned start(const struct cd_hash *h, uint32_t key)
{
    return h->clustered ? 0 : key % h->capacity;
}

/* Linear probe for key, visiting at most capacity buckets.  Returns the
 * bucket index of a live match, or -1.  *empty receives the first reusable
 * bucket seen (empty or tombstone), or capacity if none; *probes counts the
 * buckets examined.  The search stops at an EMPTY bucket, never at a
 * tombstone. */
static int locate(const struct cd_hash *h, uint32_t key, unsigned *empty,
                  unsigned *probes)
{
    *empty = h->capacity;
    *probes = 0;
    unsigned at = start(h, key);
    for (unsigned i = 0; i < h->capacity; ++i)
    {
        ++*probes;
        const struct cd_bucket *b = &h->slots[at];
        if (b->state == BUCKET_LIVE && b->key == key)
        {
            return (int)at;
        }
        if (b->state != BUCKET_LIVE && *empty == h->capacity)
        {
            *empty = at;
        }
        if (b->state == BUCKET_EMPTY ||
            (CD_MUTANT == 6 && b->state == BUCKET_TOMBSTONE))
        {
            break;
        }
        if (++at == h->capacity)
        {
            at = 0;
        }
    }
    return -1;
}

/* Acquires the bucket array from the allocator (CD_NOMEM on failure, with *h
 * untouched).  The table owns the array until cd_hash_destroy. */
enum cd_status cd_hash_init(struct cd_hash *h, unsigned cap,
                            struct cd_hash_allocator allocator,
                            unsigned cluster)
{
    if (!h || !cap || cap > HASH_MAX_CAPACITY || !allocator.take ||
        !allocator.release)
    {
        return CD_INVALID;
    }
    struct cd_bucket *p = allocator.take(allocator.context, cap);
    if (!p)
    {
        return CD_NOMEM;
    }
    for (unsigned i = 0; i < cap; ++i)
    {
        p[i] = (struct cd_bucket){0, 0, BUCKET_EMPTY};
    }
    *h = (struct cd_hash){p, cap, 0, 0, cluster != 0, allocator};
    return CD_OK;
}

/* Inserts or updates.  An update changes neither count.  Reusing a tombstone
 * raises live but not occupied.  CD_FULL when no live match and no reusable
 * bucket exists; *probes is set on every non-INVALID outcome. */
enum cd_status cd_hash_put(struct cd_hash *h, uint32_t key, int value,
                           unsigned *probes)
{
    if (!hash_valid(h) || !probes)
    {
        return CD_INVALID;
    }
    unsigned empty;
    int at = locate(h, key, &empty, probes);
    if (at < 0)
    {
        if (empty == h->capacity)
        {
            return CD_FULL;
        }
        at = (int)empty;
        if (h->slots[at].state == BUCKET_EMPTY)
        {
            ++h->occupied;
        }
        ++h->live;
    }
    h->slots[at] = (struct cd_bucket){key, value, BUCKET_LIVE};
    return CD_OK;
}

enum cd_status cd_hash_get(const struct cd_hash *h, uint32_t key, int *out,
                           unsigned *probes)
{
    if (!hash_valid(h) || !out || !probes)
    {
        return CD_INVALID;
    }
    unsigned empty;
    int at = locate(h, key, &empty, probes);
    if (at < 0)
    {
        return CD_MISSING;
    }
    *out = h->slots[at].value;
    return CD_OK;
}

/* Leaves a tombstone; occupied is unchanged until a rebuild. */
enum cd_status cd_hash_delete(struct cd_hash *h, uint32_t key)
{
    if (!hash_valid(h))
    {
        return CD_INVALID;
    }
    unsigned empty;
    unsigned probes;
    int at = locate(h, key, &empty, &probes);
    if (at < 0)
    {
        return CD_MISSING;
    }
    h->slots[at].state = BUCKET_TOMBSTONE;
    --h->live;
    return CD_OK;
}

/* Manual maintenance: acquire a new table, reinsert every live key, publish,
 * then release the old array.  Failed acquisition keeps the old table. */
enum cd_status cd_hash_rebuild(struct cd_hash *h, unsigned cap)
{
    if (!hash_valid(h) || cap < h->live || !cap || cap > HASH_MAX_CAPACITY)
    {
        return CD_INVALID;
    }
    struct cd_hash replacement;
    enum cd_status result =
        cd_hash_init(&replacement, cap, h->allocator, h->clustered);
    if (result != CD_OK)
    {
        return result;
    }
    for (unsigned i = 0; i < h->capacity; ++i)
    {
        if (h->slots[i].state != BUCKET_LIVE)
        {
            continue;
        }
        unsigned probes;
        /* Defensive and unreachable today: cap >= live, the live keys are
         * distinct and a fresh table has no tombstones, so every put finds a
         * bucket.  The check keeps the old table intact (and releases the
         * replacement) should that ever stop being true. */
        result = cd_hash_put(&replacement, h->slots[i].key, h->slots[i].value,
                             &probes);
        if (result != CD_OK)
        {
            h->allocator.release(h->allocator.context, replacement.slots);
            return result;
        }
    }
    struct cd_bucket *old = h->slots;
    *h = replacement;
    h->allocator.release(h->allocator.context, old);
    return CD_OK;
}

/* Releases the bucket array once; a second destroy is CD_INVALID. */
enum cd_status cd_hash_destroy(struct cd_hash *h)
{
    if (!hash_valid(h))
    {
        return CD_INVALID;
    }
    struct cd_bucket *old = h->slots;
    h->slots = 0;
    h->capacity = 0;
    h->live = 0;
    h->occupied = 0;
    h->allocator.release(h->allocator.context, old);
    return CD_OK;
}

/* Deliberately weak 4-bucket hash so collisions are easy to produce. */
static unsigned key_hash(const unsigned char *p, size_t n)
{
    unsigned h = 0;
    for (size_t i = 0; i < n; ++i)
    {
        h = (h * 33u + p[i]) % BYTEMAP_HASH_MODULUS;
    }
    return h;
}

static int equal(const struct cd_key *k, const unsigned char *p, size_t n)
{
    if (k->length != n)
    {
        return 0;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (k->bytes[i] != p[i])
        {
            return 0;
        }
    }
    return 1;
}

/* Keys are length-delimited byte strings (embedded zeros and the empty key
 * are valid) and are copied into the map, so the caller may reuse its buffer. */
enum cd_status cd_bytemap_put(struct cd_byte_map *m, const unsigned char *p,
                              size_t n, int value)
{
    if (!m || (!p && n) || n > BYTE_KEY_MAX)
    {
        return CD_INVALID;
    }
    unsigned at = key_hash(p, n);
    for (unsigned i = 0; i < BYTEMAP_SLOTS; ++i)
    {
        struct cd_key *k = &m->keys[at];
        if (!m->live[at] || equal(k, p, n))
        {
            for (size_t j = 0; j < n; ++j)
            {
                k->bytes[j] = p[j];
            }
            k->length = n;
            k->value = value;
            m->live[at] = 1;
            return CD_OK;
        }
        at = (at + 1) % BYTEMAP_SLOTS;
    }
    return CD_FULL;
}

enum cd_status cd_bytemap_get(const struct cd_byte_map *m,
                              const unsigned char *p, size_t n, int *out)
{
    if (!m || !out || (!p && n) || n > BYTE_KEY_MAX)
    {
        return CD_INVALID;
    }
    unsigned at = key_hash(p, n);
    for (unsigned i = 0; i < BYTEMAP_SLOTS; ++i)
    {
        if (!m->live[at])
        {
            return CD_MISSING;
        }
        if (equal(&m->keys[at], p, n))
        {
            *out = m->keys[at].value;
            return CD_OK;
        }
        at = (at + 1) % BYTEMAP_SLOTS;
    }
    return CD_MISSING;
}

/* Trie of TRIE_NODES slot-indexed nodes; slot 0 is the root (the empty key).
 * Each node keeps its first child and a sibling chain; -1 means none. */
void cd_trie_init(struct cd_trie *t)
{
    for (unsigned i = 0; i < TRIE_NODES; ++i)
    {
        t->nodes[i] = (struct cd_trie_node){-1, -1, -1, 0, 0, 0};
    }
    t->nodes[0].live = 1;
    t->generation = 1;
    t->count = 1;
}

static int child(const struct cd_trie *t, int parent, unsigned char byte)
{
    int at = t->nodes[parent].child;
    while (at >= 0 && t->nodes[at].byte != byte)
    {
        at = t->nodes[at].sibling;
    }
    return at;
}

/* Node reached by following all n bytes from the root, or -1. */
static int path(const struct cd_trie *t, const unsigned char *p, size_t n)
{
    int at = 0;
    for (size_t i = 0; i < n && at >= 0; ++i)
    {
        at = child(t, at, p[i]);
    }
    return at;
}

/* Follows p from the root as far as existing nodes allow.  Returns the number
 * of bytes matched; *last is the node reached. */
static size_t trie_match(const struct cd_trie *t, const unsigned char *p,
                         size_t n, int *last)
{
    int parent = 0;
    size_t matched = 0;
    while (matched < n)
    {
        int next = child(t, parent, p[matched]);
        if (next < 0)
        {
            break;
        }
        parent = next;
        ++matched;
    }
    *last = parent;
    return matched;
}

/* Finds `needed` free node slots before anything is modified, so a failure at
 * any acquisition leaves the trie as it was.  `fail` (1-based) simulates the
 * failure of that acquisition and returns CD_NOMEM. */
static enum cd_status trie_plan_slots(const struct cd_trie *t, unsigned needed,
                                      unsigned fail,
                                      unsigned slots[BYTE_KEY_MAX])
{
    unsigned made = 0;
    for (unsigned i = 1; i < TRIE_NODES && made < needed; ++i)
    {
        if (t->nodes[i].live)
        {
            continue;
        }
        if (fail == made + 1)
        {
            return CD_NOMEM;
        }
        slots[made++] = i;
    }
    return made == needed ? CD_OK : CD_FULL;
}

/* A mutation advances the generation, which invalidates open cursors; a
 * saturated generation refuses further mutation (CD_FULL). */
enum cd_status cd_trie_insert(struct cd_trie *t, const unsigned char *p,
                              size_t n, unsigned fail)
{
    if (!t || (!p && n) || n > BYTE_KEY_MAX || !t->generation)
    {
        return CD_INVALID;
    }
    int parent;
    size_t matched = trie_match(t, p, n, &parent);
    if (matched == n && t->nodes[parent].terminal)
    {
        return CD_DUPLICATE;
    }
    if (t->generation == UINT32_MAX)
    {
        return CD_FULL;
    }
    unsigned needed = (unsigned)(n - matched);
    unsigned slots[BYTE_KEY_MAX];
    enum cd_status planned = trie_plan_slots(t, needed, fail, slots);
    if (planned != CD_OK)
    {
        return planned;
    }
    for (unsigned i = 0; i < needed; ++i)
    {
        unsigned at = slots[i];
        t->nodes[at] = (struct cd_trie_node){
            parent, -1, t->nodes[parent].child, p[matched + i], 1, 0};
        t->nodes[parent].child = (int)at;
        parent = (int)at;
    }
    t->nodes[parent].terminal = 1;
    t->count += needed;
    ++t->generation;
    return CD_OK;
}

/* Unlinks node `at` from its parent's child chain and frees it. */
static void trie_free_leaf(struct cd_trie *t, int at)
{
    int parent = t->nodes[at].parent;
    int *edge = &t->nodes[parent].child;
    while (*edge != at)
    {
        edge = &t->nodes[*edge].sibling;
    }
    *edge = t->nodes[at].sibling;
    t->nodes[at].live = 0;
    --t->count;
}

/* Clears the terminal mark, then prunes upward while a node has no children
 * and is not itself terminal.  Descendants of a terminal are never removed. */
enum cd_status cd_trie_delete(struct cd_trie *t, const unsigned char *p,
                              size_t n)
{
    if (!t || (!p && n) || n > BYTE_KEY_MAX)
    {
        return CD_INVALID;
    }
    int at = path(t, p, n);
    if (at < 0 || !t->nodes[at].terminal)
    {
        return CD_MISSING;
    }
    if (t->generation == UINT32_MAX)
    {
        return CD_FULL;
    }
    t->nodes[at].terminal = 0;
    while (at && t->nodes[at].child < 0 && !t->nodes[at].terminal)
    {
        int parent = t->nodes[at].parent;
        trie_free_leaf(t, at);
        at = parent;
    }
    ++t->generation;
    return CD_OK;
}

static int descendant(const struct cd_trie *t, int at, int root)
{
    while (at >= 0)
    {
        if (at == root)
        {
            return 1;
        }
        at = t->nodes[at].parent;
    }
    return 0;
}

/* Snapshots, in node-slot order (not lexicographic), the terminal nodes at or
 * below the prefix.  The cursor borrows the trie and records its generation. */
enum cd_status cd_trie_prefix(const struct cd_trie *t, const unsigned char *p,
                              size_t n, struct cd_trie_cursor *out)
{
    if (!t || !out || (!p && n) || n > BYTE_KEY_MAX)
    {
        return CD_INVALID;
    }
    int root = path(t, p, n);
    if (root < 0)
    {
        return CD_MISSING;
    }
    struct cd_trie_cursor cursor = {t, t->generation, {0}, 0, 0};
    for (unsigned i = 0; i < TRIE_NODES; ++i)
    {
        if (t->nodes[i].live && t->nodes[i].terminal &&
            descendant(t, (int)i, root))
        {
            cursor.nodes[cursor.count++] = i;
        }
    }
    *out = cursor;
    return CD_OK;
}

/* Rebuilds the key of node `at` into *key by walking to the root, which
 * yields the bytes in reverse.  Returns 0 if the path exceeds BYTE_KEY_MAX. */
static int trie_key_of(const struct cd_trie *t, int at, struct cd_key *key)
{
    unsigned char reverse[BYTE_KEY_MAX];
    size_t n = 0;
    while (at)
    {
        if (n == BYTE_KEY_MAX)
        {
            return 0;
        }
        reverse[n++] = t->nodes[at].byte;
        at = t->nodes[at].parent;
    }
    *key = (struct cd_key){{0}, n, 0};
    for (size_t i = 0; i < n; ++i)
    {
        key->bytes[i] = reverse[n - 1 - i];
    }
    return 1;
}

/* CD_BUSY (nothing published) once the trie has been mutated since the
 * cursor was made; CD_EMPTY when exhausted. */
enum cd_status cd_trie_next(struct cd_trie_cursor *c, struct cd_key *out)
{
    if (!c || !out || !c->owner)
    {
        return CD_INVALID;
    }
    if (c->generation != c->owner->generation && CD_MUTANT != 7)
    {
        return CD_BUSY;
    }
    if (c->position == c->count)
    {
        return CD_EMPTY;
    }
    struct cd_key key;
    if (!trie_key_of(c->owner, (int)c->nodes[c->position], &key))
    {
        return CD_INVALID;
    }
    *out = key;
    ++c->position;
    return CD_OK;
}

/* Sparse set over 0..SPARSE_UNIVERSE-1.  Membership needs the generation tag
 * and reciprocal agreement between the sparse and dense arrays, so stale
 * entries are harmless and clear is O(1) until the generation wraps. */
void cd_sparse_init(struct cd_sparse *s)
{
    s->count = 0;
    s->generation = 1;
    for (unsigned i = 0; i < SPARSE_UNIVERSE; ++i)
    {
        s->tags[i] = 0;
        s->sparse[i] = 0;
    }
}

int cd_sparse_has(const struct cd_sparse *s, unsigned key)
{
    return s && key < SPARSE_UNIVERSE && s->tags[key] == s->generation &&
           s->sparse[key] < s->count && s->dense[s->sparse[key]] == key;
}

enum cd_status cd_sparse_insert(struct cd_sparse *s, unsigned key)
{
    if (!s || key >= SPARSE_UNIVERSE)
    {
        return CD_INVALID;
    }
    if (cd_sparse_has(s, key))
    {
        return CD_DUPLICATE;
    }
    if (s->count == SPARSE_MAX_COUNT)
    {
        return CD_FULL;
    }
    s->sparse[key] = s->count;
    s->tags[key] = s->generation;
    s->dense[s->count++] = key;
    return CD_OK;
}

/* Moves the last dense element into the hole. */
enum cd_status cd_sparse_remove(struct cd_sparse *s, unsigned key)
{
    if (!s || key >= SPARSE_UNIVERSE)
    {
        return CD_INVALID;
    }
    if (!cd_sparse_has(s, key))
    {
        return CD_MISSING;
    }
    unsigned at = s->sparse[key];
    unsigned last = s->dense[--s->count];
    s->dense[at] = last;
    s->sparse[last] = at;
    s->tags[key] = 0;
    return CD_OK;
}

/* Generations run 1..SPARSE_LAST_GENERATION (a two-bit model).  On wrap every
 * tag is zeroed before generation 1 is reused, the one O(universe) clear. */
void cd_sparse_clear(struct cd_sparse *s)
{
    s->count = 0;
    if (s->generation == SPARSE_LAST_GENERATION)
    {
        for (unsigned i = 0; i < SPARSE_UNIVERSE; ++i)
        {
            s->tags[i] = 0;
        }
        s->generation = 1;
    }
    else
    {
        ++s->generation;
    }
}

enum cd_status cd_sets_init(struct cd_sets *s, unsigned n)
{
    if (!s || n > SETS_MAX_ITEMS)
    {
        return CD_INVALID;
    }
    s->count = n;
    s->used = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        s->parent[i] = i;
        s->size[i] = 1;
    }
    return CD_OK;
}

/* Root of `at` in the working copy p, with full path compression. */
static unsigned root(unsigned *p, unsigned at)
{
    unsigned r = at;
    while (p[r] != r)
    {
        r = p[r];
    }
    while (p[at] != at)
    {
        unsigned next = p[at];
        p[at] = r;
        at = next;
    }
    return r;
}

/* Copies the live arrays into scratch so an operation can be computed fully
 * before anything is published. */
static void sets_snapshot(const struct cd_sets *s, unsigned *parent,
                          unsigned *size)
{
    for (unsigned i = 0; i < s->count; ++i)
    {
        parent[i] = s->parent[i];
        size[i] = s->size[i];
    }
}

/* Publishes the scratch arrays, logging the old value of every changed entry.
 * The undo-log capacity is checked first, so either all changes and all log
 * records land or neither does (CD_FULL). */
static enum cd_status commit(struct cd_sets *s, const unsigned *p,
                             const unsigned *size)
{
    size_t required = 0;
    for (unsigned i = 0; i < s->count; ++i)
    {
        required += (p[i] != s->parent[i]) + (size[i] != s->size[i]);
    }
    if (required > SETS_LOG_CAPACITY - s->used)
    {
        return CD_FULL;
    }
    for (unsigned i = 0; i < s->count; ++i)
    {
        if (p[i] != s->parent[i])
        {
            s->log[s->used++] = (struct cd_undo){0, i, s->parent[i]};
            s->parent[i] = p[i];
        }
        if (size[i] != s->size[i])
        {
            s->log[s->used++] = (struct cd_undo){1, i, s->size[i]};
            s->size[i] = size[i];
        }
    }
    return CD_OK;
}

/* Path compression changes parents, so find itself logs and can return
 * CD_FULL when the undo log has no room. */
enum cd_status cd_sets_find(struct cd_sets *s, unsigned at, unsigned *out)
{
    if (!s || !out || at >= s->count || s->used > SETS_LOG_CAPACITY)
    {
        return CD_INVALID;
    }
    unsigned p[SETS_MAX_ITEMS];
    unsigned sizes[SETS_MAX_ITEMS];
    sets_snapshot(s, p, sizes);
    unsigned r = root(p, at);
    enum cd_status status = commit(s, p, sizes);
    if (status == CD_OK)
    {
        *out = r;
    }
    return status;
}

/* Union by size: the smaller root hangs under the larger.  Joining two
 * elements already in one set changes no size. */
enum cd_status cd_sets_union(struct cd_sets *s, unsigned a, unsigned b)
{
    if (!s || a >= s->count || b >= s->count || s->used > SETS_LOG_CAPACITY)
    {
        return CD_INVALID;
    }
    unsigned p[SETS_MAX_ITEMS];
    unsigned sizes[SETS_MAX_ITEMS];
    sets_snapshot(s, p, sizes);
    unsigned x = root(p, a);
    unsigned y = root(p, b);
    if (x != y)
    {
        if (sizes[x] < sizes[y])
        {
            unsigned swap = x;
            x = y;
            y = swap;
        }
        p[y] = x;
        sizes[x] += sizes[y];
    }
    return commit(s, p, sizes);
}

/* mark must be a log length previously observed at an operation boundary. */
enum cd_status cd_sets_rollback(struct cd_sets *s, size_t mark)
{
    if (!s || mark > s->used)
    {
        return CD_INVALID;
    }
    while (s->used > mark)
    {
        struct cd_undo change = s->log[--s->used];
        if (change.field)
        {
            s->size[change.index] = change.old;
        }
        else
        {
            s->parent[change.index] = change.old;
        }
    }
    return CD_OK;
}

/* Validates every endpoint (and, unless parallel edges are allowed, rejects a
 * repeated from/to pair) while counting the edges leaving each vertex into
 * result->offset[from + 1]. */
static enum cd_status csr_count_edges(struct cd_csr *result, unsigned vertices,
                                      const struct cd_edge *edges, size_t n,
                                      unsigned parallel)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (edges[i].from >= vertices || edges[i].to >= vertices)
        {
            return CD_INVALID;
        }
        if (!parallel)
        {
            for (size_t j = 0; j < i; ++j)
            {
                if (edges[j].from == edges[i].from &&
                    edges[j].to == edges[i].to)
                {
                    return CD_DUPLICATE;
                }
            }
        }
        ++result->offset[edges[i].from + 1];
    }
    return CD_OK;
}

/* Compressed sparse row: count per source, prefix-sum into offsets, then
 * scatter, which keeps input order within each source.  The result is built
 * aside and copied out last, so a rejected build preserves the old snapshot. */
enum cd_status cd_csr_build(struct cd_csr *out, unsigned vertices,
                            const struct cd_edge *edges, size_t n,
                            unsigned parallel)
{
    if (!out || vertices > CSR_MAX_VERTICES || n > CSR_MAX_EDGES ||
        (!edges && n) || parallel > 1)
    {
        return CD_INVALID;
    }
    struct cd_csr result = {vertices, (unsigned)n, {0}, {{0}}};
    enum cd_status status =
        csr_count_edges(&result, vertices, edges, n, parallel);
    if (status != CD_OK)
    {
        return status;
    }
    for (unsigned i = 1; i <= vertices; ++i)
    {
        result.offset[i] += result.offset[i - 1];
    }
    unsigned next[CSR_MAX_VERTICES];
    for (unsigned i = 0; i < vertices; ++i)
    {
        next[i] = result.offset[i];
    }
    for (size_t i = 0; i < n; ++i)
    {
        result.edges[next[edges[i].from]++] = edges[i];
    }
    *out = result;
    return CD_OK;
}

/* The filter sets two of eight bits per key: bit key mod 8 and bit
 * (3 * key + 1) mod 8.  Keys are 0..FILTER_KEYS-1. */
static unsigned filter_cell_a(unsigned key)
{
    return key % FILTER_CELLS;
}

static unsigned filter_cell_b(unsigned key)
{
    return (3 * key + 1) % FILTER_CELLS;
}

static unsigned filter_mask(unsigned key)
{
    return (1u << filter_cell_a(key)) | (1u << filter_cell_b(key));
}

/* No deletion is offered: clearing a bit could erase another key's evidence. */
enum cd_status cd_filter_insert(unsigned char *bits, unsigned key)
{
    if (!bits || key >= FILTER_KEYS)
    {
        return CD_INVALID;
    }
    *bits |= (unsigned char)filter_mask(key);
    return CD_OK;
}

/* *out is 1 when both bits are set (maybe present) and 0 when definitely
 * absent; a 1 does not prove the key was inserted. */
enum cd_status cd_filter_query(unsigned char bits, unsigned key, unsigned *out)
{
    if (!out || key >= FILTER_KEYS)
    {
        return CD_INVALID;
    }
    unsigned mask = filter_mask(key);
    *out = (bits & mask) == mask;
    return CD_OK;
}

/* Counting variant: each cell saturates at FILTER_CELL_LIMIT, and a per-key
 * multiplicity ledger justifies deletes.  All three counters are checked
 * before any increment, so an overflow changes nothing. */
enum cd_status cd_counting_insert(struct cd_counting_filter *f, unsigned key)
{
    if (!f || key >= FILTER_KEYS)
    {
        return CD_INVALID;
    }
    unsigned a = filter_cell_a(key);
    unsigned b = filter_cell_b(key);
    if (f->cells[a] == FILTER_CELL_LIMIT || f->cells[b] == FILTER_CELL_LIMIT ||
        f->keys[key] == FILTER_CELL_LIMIT)
    {
        return CD_FULL;
    }
    ++f->cells[a];
    ++f->cells[b];
    ++f->keys[key];
    return CD_OK;
}

enum cd_status cd_counting_delete(struct cd_counting_filter *f, unsigned key)
{
    if (!f || key >= FILTER_KEYS)
    {
        return CD_INVALID;
    }
    if (!f->keys[key])
    {
        return CD_MISSING;
    }
    --f->cells[filter_cell_a(key)];
    --f->cells[filter_cell_b(key)];
    --f->keys[key];
    return CD_OK;
}

/* Separate chaining over a node pool: CHAIN_BUCKETS list heads, CHAIN_NODES
 * nodes, -1 ends a chain. */
void cd_chain_init(struct cd_chain *c)
{
    c->count = 0;
    for (unsigned i = 0; i < CHAIN_BUCKETS; ++i)
    {
        c->head[i] = -1;
    }
    for (unsigned i = 0; i < CHAIN_NODES; ++i)
    {
        c->nodes[i] = (struct cd_chain_node){0, 0, -1, 0};
    }
}

enum cd_status cd_chain_put(struct cd_chain *c, uint32_t key, int value)
{
    if (!c)
    {
        return CD_INVALID;
    }
    unsigned bucket = key % CHAIN_BUCKETS;
    for (int i = c->head[bucket]; i >= 0; i = c->nodes[i].next)
    {
        if (c->nodes[i].key == key)
        {
            c->nodes[i].value = value;
            return CD_OK;
        }
    }
    unsigned free_slot = 0;
    while (free_slot < CHAIN_NODES && c->nodes[free_slot].live)
    {
        ++free_slot;
    }
    if (free_slot == CHAIN_NODES)
    {
        return CD_FULL;
    }
    c->nodes[free_slot] =
        (struct cd_chain_node){key, value, c->head[bucket], 1};
    c->head[bucket] = (int)free_slot;
    ++c->count;
    return CD_OK;
}

/* *probes counts chain nodes examined, on a hit and on a miss. */
enum cd_status cd_chain_get(const struct cd_chain *c, uint32_t key, int *out,
                            unsigned *probes)
{
    if (!c || !out || !probes)
    {
        return CD_INVALID;
    }
    *probes = 0;
    for (int i = c->head[key % CHAIN_BUCKETS]; i >= 0; i = c->nodes[i].next)
    {
        ++*probes;
        if (c->nodes[i].key == key)
        {
            *out = c->nodes[i].value;
            return CD_OK;
        }
    }
    return CD_MISSING;
}

enum cd_status cd_chain_delete(struct cd_chain *c, uint32_t key)
{
    if (!c)
    {
        return CD_INVALID;
    }
    int *edge = &c->head[key % CHAIN_BUCKETS];
    while (*edge >= 0)
    {
        struct cd_chain_node *n = &c->nodes[*edge];
        if (n->key == key)
        {
            *edge = n->next;
            n->live = 0;
            n->next = -1;
            --c->count;
            return CD_OK;
        }
        edge = &n->next;
    }
    return CD_MISSING;
}

/* Mask of the low `universe` bits.  A full 32-bit universe is special-cased
 * because shifting a 32-bit value by 32 is undefined. */
enum cd_status cd_bitset_mask(unsigned universe, uint32_t *out)
{
    if (!out || universe > BITSET_WIDTH)
    {
        return CD_INVALID;
    }
    *out =
        universe == BITSET_WIDTH ? UINT32_MAX : ((UINT32_C(1) << universe) - 1);
    return CD_OK;
}

/* Join, meet and difference (a minus b), each restricted to the universe. */
enum cd_status cd_bitset_algebra(uint32_t a, uint32_t b, unsigned universe,
                                 uint32_t *join, uint32_t *meet,
                                 uint32_t *difference)
{
    uint32_t mask;
    if (!join || !meet || !difference ||
        cd_bitset_mask(universe, &mask) != CD_OK)
    {
        return CD_INVALID;
    }
    *join = (a | b) & mask;
    *meet = (a & b) & mask;
    *difference = (a & ~b) & mask;
    return CD_OK;
}

/* Iterator: *position is the next bit index to examine, so a caller loops
 * until CD_EMPTY.  *position is advanced past the returned bit. */
enum cd_status cd_bitset_next(uint32_t bits, unsigned universe,
                              unsigned *position, unsigned *out)
{
    if (!position || !out || universe > BITSET_WIDTH || *position > universe)
    {
        return CD_INVALID;
    }
    unsigned at = *position;
    while (at < universe && !(bits & (UINT32_C(1) << at)))
    {
        ++at;
    }
    if (at == universe)
    {
        *position = at;
        return CD_EMPTY;
    }
    *out = at;
    *position = at + 1;
    return CD_OK;
}
