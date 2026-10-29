#include "algo.h"
#include <math.h>
#include <string.h>
#define WORK(w, member)    \
    do                     \
    {                      \
        if (w)             \
            ++(w)->member; \
    } while (0)

int algo_ring_init(algo_ring *q, uint32_t capacity)
{
    if (!q || !capacity || capacity > 16)
        return ALGO_BAD;
    memset(q, 0, sizeof *q);
    q->capacity = capacity;
    return ALGO_OK;
}

int algo_ring_put(algo_ring *q, int32_t value)
{
    if (!q || !q->capacity || q->capacity > 16 || q->count > q->capacity ||
        q->head >= q->capacity)
        return ALGO_BAD;
    if (q->count == q->capacity)
        return ALGO_FULL;
    q->data[(q->head + q->count) % q->capacity] = value;
    ++q->count;
    return ALGO_OK;
}

int algo_ring_take(algo_ring *q, int32_t *out)
{
    if (!q || !out || !q->capacity || q->capacity > 16 ||
        q->count > q->capacity || q->head >= q->capacity)
        return ALGO_BAD;
    if (!q->count)
        return ALGO_MISSING;
    *out = q->data[q->head];
    q->head = (q->head + 1) % q->capacity;
    --q->count;
    return ALGO_OK;
}

void algo_hash_init(algo_hash *h)
{
    memset(h, 0, sizeof *h);
}

static int hash_find(const algo_hash *h, uint32_t key, int *empty, algo_work *w)
{
    *empty = -1;
    for (uint32_t i = 0; i < 16; ++i)
    {
        unsigned p = (key + i) & 15u;
        const algo_bucket *b = &h->slots[p];
        WORK(w, visits);
        if (b->state == 1 && b->key == key)
            return (int)p;
        if (b->state != 1 && *empty < 0)
            *empty = (int)p;
        if (b->state == 0)
            break;
#ifdef ALGO_MUTANT_TOMBSTONE
        if (b->state == 2)
            break;
#endif
    }
    return -1;
}

int algo_hash_put(algo_hash *h, uint32_t key, int32_t v, algo_work *w)
{
    if (!h)
        return ALGO_BAD;
    int empty, p = hash_find(h, key, &empty, w);
    if (p < 0)
    {
        if (empty < 0)
            return ALGO_FULL;
        p = empty;
        ++h->count;
    }
    h->slots[p] = (algo_bucket){key, v, 1};
    return ALGO_OK;
}

int algo_hash_get(const algo_hash *h, uint32_t key, int32_t *out, algo_work *w)
{
    if (!h || !out)
        return ALGO_BAD;
    int empty, p = hash_find(h, key, &empty, w);
    if (p < 0)
        return ALGO_MISSING;
    *out = h->slots[p].value;
    return ALGO_OK;
}

int algo_hash_delete(algo_hash *h, uint32_t key, algo_work *w)
{
    if (!h)
        return ALGO_BAD;
    int empty, p = hash_find(h, key, &empty, w);
    if (p < 0)
        return ALGO_MISSING;
    h->slots[p].state = 2;
    --h->count;
    return ALGO_OK;
}

void algo_avl_init(algo_avl *t)
{
    memset(t, 0, sizeof *t);
    t->root = -1;
}

static int height(const algo_avl *t, int p)
{
    return p < 0 ? 0 : t->nodes[p].height;
}

static void update(algo_avl *t, int p)
{
    int a = height(t, t->nodes[p].left), b = height(t, t->nodes[p].right);
    t->nodes[p].height = (int16_t)(1 + (a > b ? a : b));
}

static int rotate_right(algo_avl *t, int p)
{
    int q = t->nodes[p].left;
    t->nodes[p].left = t->nodes[q].right;
    t->nodes[q].right = (int16_t)p;
    update(t, p);
    update(t, q);
    return q;
}

static int rotate_left(algo_avl *t, int p)
{
    int q = t->nodes[p].right;
    t->nodes[p].right = t->nodes[q].left;
    t->nodes[q].left = (int16_t)p;
    update(t, p);
    update(t, q);
    return q;
}

static int balance(algo_avl *t, int p)
{
    if (p < 0)
        return p;
    update(t, p);
    algo_node *n = &t->nodes[p];
    int d = height(t, n->left) - height(t, n->right);
    if (d > 1)
    {
        int q = n->left;
        if (height(t, t->nodes[q].left) < height(t, t->nodes[q].right))
            n->left = (int16_t)rotate_left(t, q);
        return rotate_right(t, p);
    }
    if (d < -1)
    {
        int q = n->right;
        if (height(t, t->nodes[q].right) < height(t, t->nodes[q].left))
            n->right = (int16_t)rotate_right(t, q);
        return rotate_left(t, p);
    }
    return p;
}

int algo_avl_contains(const algo_avl *t, int32_t key)
{
    if (!t)
        return 0;
    int p = t->root;
    while (p >= 0)
    {
        if (key == t->nodes[p].key)
            return 1;
        p = key < t->nodes[p].key ? t->nodes[p].left : t->nodes[p].right;
    }
    return 0;
}

static int insert_node(algo_avl *t, int p, int q)
{
    if (p < 0)
        return q;
    if (t->nodes[q].key < t->nodes[p].key)
        t->nodes[p].left = (int16_t)insert_node(t, t->nodes[p].left, q);
    else
        t->nodes[p].right = (int16_t)insert_node(t, t->nodes[p].right, q);
    return balance(t, p);
}

int algo_avl_insert(algo_avl *t, int32_t key)
{
    if (!t)
        return ALGO_BAD;
    if (algo_avl_contains(t, key))
        return ALGO_OK;
    int q = 0;
    while (q < 64 && t->nodes[q].live)
        ++q;
    if (q == 64)
        return ALGO_FULL;
    t->nodes[q] = (algo_node){key, -1, -1, 1, 1};
    t->root = (int16_t)insert_node(t, t->root, q);
    ++t->count;
    return ALGO_OK;
}

static int delete_node(algo_avl *t, int p, int32_t key)
{
    if (p < 0)
        return -1;
    algo_node *n = &t->nodes[p];
    if (key < n->key)
        n->left = (int16_t)delete_node(t, n->left, key);
    else if (key > n->key)
        n->right = (int16_t)delete_node(t, n->right, key);
    else if (n->left < 0 || n->right < 0)
    {
        int q = n->left < 0 ? n->right : n->left;
        n->live = 0;
        --t->count;
        return q;
    }
    else
    {
        int q = n->right;
        while (t->nodes[q].left >= 0)
            q = t->nodes[q].left;
        n->key = t->nodes[q].key;
        n->right = (int16_t)delete_node(t, n->right, n->key);
    }
    return balance(t, p);
}

int algo_avl_delete(algo_avl *t, int32_t key)
{
    if (!t)
        return ALGO_BAD;
    if (!algo_avl_contains(t, key))
        return ALGO_MISSING;
    t->root = (int16_t)delete_node(t, t->root, key);
    return ALGO_OK;
}

int algo_sets_init(algo_sets *s, uint32_t n)
{
    if (!s || n > 64)
        return ALGO_BAD;
    memset(s, 0, sizeof *s);
    s->count = (uint8_t)n;
    for (uint32_t i = 0; i < n; ++i)
        s->parent[i] = (uint8_t)i;
    return ALGO_OK;
}

int algo_sets_find(algo_sets *s, uint32_t x, uint32_t *out)
{
    if (!s || !out || x >= s->count)
        return ALGO_BAD;
    uint32_t root = x;
    while (s->parent[root] != root)
        root = s->parent[root];
    while (s->parent[x] != x)
    {
        uint32_t p = s->parent[x];
        s->parent[x] = (uint8_t)root;
        x = p;
    }
    *out = root;
    return ALGO_OK;
}

int algo_sets_union(algo_sets *s, uint32_t a, uint32_t b)
{
    if (!s || a >= s->count || b >= s->count)
        return ALGO_BAD;
    uint32_t x, y;
    algo_sets_find(s, a, &x);
    algo_sets_find(s, b, &y);
    if (x == y)
        return ALGO_OK;
    if (s->rank[x] < s->rank[y])
    {
        uint32_t z = x;
        x = y;
        y = z;
    }
    s->parent[y] = (uint8_t)x;
    if (s->rank[x] == s->rank[y])
        ++s->rank[x];
    return ALGO_OK;
}

static int graph_valid(const algo_graph *g, uint32_t start, uint32_t *out)
{
    return g && out && g->n <= 16 && start < g->n;
}

int algo_bfs(const algo_graph *g, uint32_t start, uint32_t out[16],
             algo_work *w)
{
    if (!graph_valid(g, start, out))
        return ALGO_BAD;
    uint32_t d[16], q[16], head = 0, tail = 0;
    for (unsigned i = 0; i < 16; ++i)
        d[i] = ALGO_INF;
    d[start] = 0;
    q[tail++] = start;
    while (head < tail)
    {
        unsigned u = q[head++];
        for (unsigned v = 0; v < g->n; ++v)
        {
            WORK(w, visits);
            if (g->weight[u][v] != ALGO_INF && d[v] == ALGO_INF)
            {
                d[v] = d[u] + 1;
                q[tail++] = v;
            }
        }
    }
    memcpy(out, d, sizeof d);
    return ALGO_OK;
}

int algo_dijkstra(const algo_graph *g, uint32_t start, uint32_t out[16],
                  algo_work *w)
{
    if (!graph_valid(g, start, out))
        return ALGO_BAD;
    uint32_t d[16];
    uint8_t done[16] = {0};
    for (unsigned i = 0; i < 16; ++i)
        d[i] = ALGO_INF;
    d[start] = 0;
    for (unsigned step = 0; step < g->n; ++step)
    {
        int u = -1;
        for (unsigned v = 0; v < g->n; ++v)
            if (!done[v] && d[v] != ALGO_INF && (u < 0 || d[v] < d[u]))
                u = (int)v;
        if (u < 0)
            break;
        done[u] = 1;
        for (unsigned v = 0; v < g->n; ++v)
        {
            uint32_t edge = g->weight[u][v];
            WORK(w, visits);
            if (edge == ALGO_INF || done[v])
                continue;
            if (edge >= ALGO_INF - d[u])
                return ALGO_OVERFLOW;
            uint32_t candidate = d[u] + edge;
            if (candidate < d[v])
                d[v] = candidate;
        }
    }
    memcpy(out, d, sizeof d);
    return ALGO_OK;
}

int algo_lower(const uint32_t *a, uint32_t n, uint32_t key, uint32_t *out,
               algo_work *w)
{
    if (!out || n > 1024 || (n && !a))
        return ALGO_BAD;
    for (uint32_t i = 1; i < n; ++i)
        if (a[i] < a[i - 1])
            return ALGO_BAD;
    uint32_t lo = 0, hi = n;
    while (lo < hi)
    {
        uint32_t mid = lo + (hi - lo) / 2;
        WORK(w, comparisons);
        if (a[mid] < key)
            lo = mid + 1;
        else
            hi = mid;
    }
    *out = lo;
    return ALGO_OK;
}

static int sort_valid(const algo_pair *a, uint32_t n, uint32_t cap)
{
    return n <= 1024 && n <= cap && (!n || a);
}

int algo_insertion(algo_pair *a, uint32_t n, uint32_t cap, algo_work *w)
{
    if (!sort_valid(a, n, cap))
        return ALGO_BAD;
    for (uint32_t i = 1; i < n; ++i)
    {
        algo_pair v = a[i];
        uint32_t j = i;
        while (j)
        {
            WORK(w, comparisons);
#ifdef ALGO_MUTANT_STABILITY
            if (a[j - 1].key < v.key)
                break;
#else
            if (a[j - 1].key <= v.key)
                break;
#endif
            a[j] = a[j - 1];
            WORK(w, moves);
            --j;
        }
        a[j] = v;
        WORK(w, moves);
    }
    return ALGO_OK;
}

static void merge_rec(algo_pair *a, algo_pair *tmp, uint32_t lo, uint32_t hi,
                      algo_work *w)
{
    if (hi - lo < 2)
        return;
    uint32_t mid = lo + (hi - lo) / 2;
    merge_rec(a, tmp, lo, mid, w);
    merge_rec(a, tmp, mid, hi, w);
    uint32_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
    {
        WORK(w, comparisons);
        tmp[k++] = a[i].key <= a[j].key ? a[i++] : a[j++];
        WORK(w, moves);
    }
    while (i < mid)
    {
        tmp[k++] = a[i++];
        WORK(w, moves);
    }
    while (j < hi)
    {
        tmp[k++] = a[j++];
        WORK(w, moves);
    }
    for (i = lo; i < hi; ++i)
    {
        a[i] = tmp[i];
        WORK(w, moves);
    }
}

int algo_merge(algo_pair *a, uint32_t n, uint32_t cap, algo_work *w)
{
    if (!sort_valid(a, n, cap))
        return ALGO_BAD;
    algo_pair tmp[1024];
    merge_rec(a, tmp, 0, n, w);
    return ALGO_OK;
}

int algo_sift(uint32_t *a, uint32_t n, uint32_t cap, uint32_t p, algo_work *w)
{
    if (n > 1024 || n > cap || (n && (!a || p >= n)))
        return ALGO_BAD;
    if (!n)
        return ALGO_OK;
    for (uint32_t i = 1; i < n; ++i)
    {
        uint32_t parent = (i - 1) / 2;
        if (parent != p && a[parent] < a[i])
            return ALGO_BAD;
        if (p && parent == p && a[(p - 1) / 2] < a[i])
            return ALGO_BAD;
    }
    for (;;)
    {
        uint32_t child = 2 * p + 1;
        if (child >= n)
            break;
        if (child + 1 < n)
        {
            WORK(w, comparisons);
            if (a[child + 1] > a[child])
                ++child;
        }
        WORK(w, comparisons);
        if (a[p] >= a[child])
            break;
        uint32_t v = a[p];
        a[p] = a[child];
        a[child] = v;
        WORK(w, moves);
        p = child;
    }
    return ALGO_OK;
}

int algo_match(const uint8_t *t, size_t n, const uint8_t *p, size_t m,
               size_t *out, algo_work *w)
{
    if (!out || (n && !t) || (m && !p))
        return ALGO_BAD;
    if (!m)
    {
        *out = 0;
        return ALGO_OK;
    }
    if (m > n)
        return ALGO_MISSING;
    for (size_t i = 0; i <= n - m; ++i)
    {
        size_t j = 0;
        while (j < m)
        {
            WORK(w, comparisons);
            if (t[i + j] != p[j])
                break;
            ++j;
        }
        if (j == m)
        {
            *out = i;
            return ALGO_OK;
        }
    }
    return ALGO_MISSING;
}

int algo_arena_init(algo_arena *a, uint8_t *data, size_t capacity)
{
    if (!a || (capacity && !data))
        return ALGO_BAD;
    *a = (algo_arena){data, capacity, 0};
    return ALGO_OK;
}

int algo_arena_take(algo_arena *a, size_t bytes, size_t alignment, size_t *out)
{
    if (!a || !out || !alignment || (alignment & (alignment - 1)) ||
        a->used > a->capacity)
        return ALGO_BAD;
    /* Align the actual byte address; no promise of effective type for objects. */
    uintptr_t base = (uintptr_t)a->data;
    if (a->used > UINTPTR_MAX - base)
        return ALGO_OVERFLOW;
    size_t pad = (size_t)(-(base + a->used)) & (alignment - 1);
    if (pad > a->capacity - a->used || bytes > a->capacity - a->used - pad)
        return ALGO_FULL;
    size_t at = a->used + pad;
    a->used = at + bytes;
    *out = at;
    return ALGO_OK;
}

int algo_arena_rewind(algo_arena *a, size_t mark)
{
    if (!a || mark > a->used)
        return ALGO_BAD;
    a->used = mark;
    return ALGO_OK;
}

void algo_pool_init(algo_pool *p)
{
    memset(p, 0, sizeof *p);
    for (unsigned i = 0; i < 16; ++i)
        p->generation[i] = 1;
}

int algo_pool_take(algo_pool *p, int32_t value, uint64_t *out)
{
    if (!p || !out)
        return ALGO_BAD;
    for (unsigned i = 0; i < 16; ++i)
        if (!p->state[i])
        {
            p->state[i] = 1;
            p->value[i] = value;
            *out = ((uint64_t)p->generation[i] << 32) | i;
            return ALGO_OK;
        }
    return ALGO_FULL;
}

static int pool_slot(const algo_pool *p, uint64_t h)
{
    uint32_t i = (uint32_t)h, g = (uint32_t)(h >> 32);
    return p && i < 16 && g && p->state[i] == 1 && p->generation[i] == g
               ? (int)i
               : -1;
}

int algo_pool_get(const algo_pool *p, uint64_t h, int32_t *out)
{
    if (!out)
        return ALGO_BAD;
    int i = pool_slot(p, h);
    if (i < 0)
        return ALGO_MISSING;
    *out = p->value[i];
    return ALGO_OK;
}

int algo_pool_release(algo_pool *p, uint64_t h)
{
    int i = pool_slot(p, h);
    if (i < 0)
        return ALGO_MISSING;
#ifdef ALGO_MUTANT_GENERATION
    p->state[i] = 0;
#else
    if (p->generation[i] == UINT32_MAX)
        p->state[i] = 2;
    else
    {
        ++p->generation[i];
        p->state[i] = 0;
    }
#endif
    return ALGO_OK;
}

int algo_sum(const double *a, uint32_t n, double *out)
{
    if (!out || n > 1024 || (n && !a))
        return ALGO_BAD;
    double s = 0, c = 0;
    for (uint32_t i = 0; i < n; ++i)
    {
        if (!isfinite(a[i]))
            return ALGO_BAD;
        double t = s + a[i];
        if (!isfinite(t))
            return ALGO_OVERFLOW;
        c += fabs(s) >= fabs(a[i]) ? (s - t) + a[i] : (a[i] - t) + s;
        s = t;
    }
    if (!isfinite(s + c))
        return ALGO_OVERFLOW;
    *out = s + c;
    return ALGO_OK;
}

int algo_q7(const int8_t *a, const int8_t *b, uint32_t n, int8_t *out)
{
    if (!out || n > 16 || (n && (!a || !b)))
        return ALGO_BAD;
    int32_t s = 0;
    for (uint32_t i = 0; i < n; ++i)
        s += (int32_t)a[i] * b[i];
    int32_t q = s >= 0 ? (s + 64) / 128 : -((-s + 64) / 128);
    if (q > 127)
        q = 127;
    if (q < -128)
        q = -128;
    *out = (int8_t)q;
    return ALGO_OK;
}

algo_alu_result algo_alu4(uint8_t a, uint8_t b, uint8_t carry)
{
    uint8_t value = 0, cin = carry & 1u, before = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        uint8_t x = (a >> i) & 1u, y = (b >> i) & 1u;
        if (i == 3)
            before = cin;
        value |= (uint8_t)((x ^ y ^ cin) << i);
        cin = (x & y) | (x & cin) | (y & cin);
    }
    return (algo_alu_result){value, cin, (uint8_t)(before ^ cin)};
}

int algo_cache(const uint32_t *a, uint32_t n, algo_cache_result *out)
{
    if (!out || n > 32 || (n && !a))
        return ALGO_BAD;
    uint32_t direct[4] = {0}, shadow[4] = {0}, seen[32] = {0};
    uint8_t valid[4] = {0};
    unsigned used = 0, count = 0;
    algo_cache_result r = {0};
    for (unsigned i = 0; i < n; ++i)
    {
        uint32_t block = a[i] / 16, slot = block % 4;
        int old = 0, pos = -1;
        for (unsigned j = 0; j < count; ++j)
            if (seen[j] == block)
                old = 1;
        for (unsigned j = 0; j < used; ++j)
            if (shadow[j] == block)
                pos = (int)j;
        if (valid[slot] && direct[slot] == block)
            ++r.hits;
        else
        {
            ++r.misses;
            if (!old)
                ++r.compulsory;
            else if (pos >= 0)
                ++r.conflict;
            else
                ++r.capacity;
        }
        valid[slot] = 1;
        direct[slot] = block;
        if (!old)
            seen[count++] = block;
        if (pos < 0)
        {
            if (used < 4)
                ++used;
            pos = (int)used - 1;
        }
        for (int j = pos; j > 0; --j)
            shadow[j] = shadow[j - 1];
        shadow[0] = block;
    }
    *out = r;
    return ALGO_OK;
}
