#include "object_runtime.h"
#define BIT(i) ((uint16_t)(UINT32_C(1) << (i)))

uint64_t or_number_bits(uint64_t b)
{
#ifndef OR_MUTATE_VALUE
    if ((b & UINT64_C(0x7ff0000000000000)) == UINT64_C(0x7ff0000000000000) &&
        (b & UINT64_C(0xfffffffffffff)))
        return UINT64_C(0x7ff8000000000000);
#endif
    return b;
}

uint64_t or_integer(int32_t v)
{
    return UINT64_C(0x7ff9000000000000) | (uint32_t)v;
}

enum or_status or_integer_get(uint64_t v, int32_t *out)
{
    uint32_t p = (uint32_t)v;
    if (!out ||
        (v & UINT64_C(0xffffffff00000000)) != UINT64_C(0x7ff9000000000000))
        return OR_INVALID;
#ifdef OR_MUTATE_VALUE2
    *out = (int32_t)(p & INT32_MAX);
#else
    *out = p <= INT32_MAX ? (int32_t)p : -1 - (int32_t)(UINT32_MAX - p);
#endif
    return OR_OK;
}

enum or_status or_pointer_tag(void *p, unsigned tag, uintptr_t *out)
{
    uintptr_t v = (uintptr_t)p;
    if (!p || !out || tag > 3 || (v & 3u))
        return OR_INVALID;
    *out = v | tag;
    return OR_OK;
}

enum or_status or_pointer_untag(uintptr_t v, unsigned tag, void **out)
{
    if (!out || tag > 3 || (v & 3u) != tag || !(v & ~(uintptr_t)3))
        return OR_INVALID;
    *out = (void *)(v & ~(uintptr_t)3);
    return OR_OK;
}

enum or_status or_intern(struct or_intern *t, const unsigned char *b, size_t n,
                         int fail, uint8_t *out)
{
    unsigned i, j;
    if (!t || !out || (!b && n) || n > 15 || t->used > 8)
        return OR_INVALID;
    for (i = 0; i < t->used; i++)
    {
        if (t->length[i] != n)
            continue;
        for (j = 0; j < n && t->bytes[i][j] == b[j]; j++)
        {
        }
#ifdef OR_MUTATE_INTERN
        j = (unsigned)n;
#endif
        if (j == n)
        {
            *out = (uint8_t)(i + 1);
            return OR_OK;
        }
    }
    if (t->used == 8)
        return OR_FULL;
#ifndef OR_MUTATE_INTERN2
    if (fail)
        return OR_FULL;
#else
    (void)fail;
#endif
    for (j = 0; j < n; j++)
        t->bytes[t->used][j] = b[j];
    t->length[t->used] = (uint8_t)n;
    *out = ++t->used;
    return OR_OK;
}

void or_shapes_init(struct or_shapes *s)
{
    if (s)
    {
        struct or_shapes z = {0};
        *s = z;
        s->used = 1;
    }
}

static int shapes_valid(const struct or_shapes *s, const struct or_object *o)
{
    return s && o && s->used > 0 && s->used <= 16 && o->shape < s->used &&
           s->shape[o->shape].count <= 4;
}

enum or_status or_property_set(struct or_shapes *s, struct or_object *o,
                               uint8_t key, int32_t value)
{
    unsigned i, n, next;
    struct or_shape *old;
    if (!key || !shapes_valid(s, o))
        return OR_INVALID;
    old = &s->shape[o->shape];
    n = old->count;
    for (i = 0; i < n; i++)
        if (old->keys[i] == key)
        {
            o->fields[i] = value;
            return OR_OK;
        }
    if (n == 4)
        return OR_FULL;
    for (next = 1; next < s->used; next++)
        if (s->shape[next].parent == o->shape && s->shape[next].added == key)
            break;
#ifdef OR_MUTATE_SHAPES
    for (next = 1; next < s->used; next++)
        if (s->shape[next].added == key)
            break;
#endif
    if (next == s->used)
    {
        if (s->used == 16)
            return OR_FULL;
        s->shape[next] = *old;
        s->shape[next].parent = o->shape;
        s->shape[next].added = key;
        s->shape[next].keys[n] = key;
        s->shape[next].count = (uint8_t)(n + 1);
        s->used++;
    }
#ifndef OR_MUTATE_SHAPES2
    o->shape = (uint8_t)next;
#endif
    o->fields[n] = value;
    return OR_OK;
}

enum or_status or_property_get(const struct or_shapes *s,
                               const struct or_object *o, uint8_t key,
                               struct or_cache *c, int32_t *out)
{
    unsigned i, n;
    if (!key || !c || !out || !shapes_valid(s, o) || c->count > 2)
        return OR_INVALID;
    if (c->owner != s)
    {
        struct or_cache z = {0};
        *c = z;
        c->owner = s;
    }
    for (i = 0; i < c->count && !c->mega; i++)
    {
        int guard = c->shape[i] == o->shape && c->key[i] == key;
#ifdef OR_MUTATE_CACHE
        guard = c->key[i] == key;
#endif
#ifdef OR_MUTATE_CACHE2
        guard = c->shape[i] == o->shape;
#endif
        if (guard)
        {
            *out = o->fields[c->offset[i]];
            c->hits++;
            return OR_OK;
        }
    }
    c->misses++;
    n = s->shape[o->shape].count;
    for (i = 0; i < n; i++)
        if (s->shape[o->shape].keys[i] == key)
            break;
    if (i == n)
        return OR_MISS;
    if (!c->mega)
    {
        if (c->count == 2)
            c->mega = 1;
        else
        {
            unsigned k = c->count++;
            c->shape[k] = o->shape;
            c->key[k] = key;
            c->offset[k] = (uint8_t)i;
        }
    }
    *out = o->fields[i];
    return OR_OK;
}

static int graph_valid(const struct or_graph *g)
{
    unsigned i, j;
    if (!g || (g->roots & ~g->live))
        return 0;
    for (i = 0; i < 16; i++)
        if (g->live & BIT(i))
            for (j = 0; j < 2; j++)
                if (g->edge[i][j] > 16 ||
                    (g->edge[i][j] && !(g->live & BIT(g->edge[i][j] - 1))))
                    return 0;
    return 1;
}

enum or_status or_mark_begin(struct or_graph *g)
{
    unsigned i;
    if (!graph_valid(g))
        return OR_INVALID;
    if (g->active)
        return OR_STATE;
    for (i = 0; i < 16; i++)
        g->color[i] = (g->roots & BIT(i)) ? 1 : 0;
    g->active = 1;
    return OR_OK;
}

enum or_status or_mark_step(struct or_graph *g, unsigned budget,
                            unsigned *scanned)
{
    unsigned i, j, n = 0;
    if (!scanned || !graph_valid(g))
        return OR_INVALID;
    if (!g->active)
        return OR_STATE;
    while (n < budget)
    {
        for (i = 0; i < 16; i++)
            if ((g->live & BIT(i)) && g->color[i] == 1)
                break;
        if (i == 16)
            break;
        for (j = 0; j < 2; j++)
        {
            unsigned t = g->edge[i][j];
            if (t && !g->color[t - 1])
                g->color[t - 1] = 1;
        }
        g->color[i] = 2;
        n++;
    }
    *scanned = n;
    for (i = 0; i < 16; i++)
        if ((g->live & BIT(i)) && g->color[i] == 1)
            return OR_OK;
    g->active = 0;
    return OR_DONE;
}

enum or_status or_graph_store(struct or_graph *g, unsigned owner,
                              unsigned field, unsigned target)
{
    if (!graph_valid(g) || owner < 1 || owner > 16 || field > 1 ||
        target > 16 || !(g->live & BIT(owner - 1)) ||
        (target && !(g->live & BIT(target - 1))))
        return OR_INVALID;
#ifndef OR_MUTATE_MARK
    if (g->active && g->color[owner - 1] == 2 && target &&
        !g->color[target - 1])
        g->color[target - 1] = 1;
#endif
    g->edge[owner - 1][field] = (uint8_t)target;
    return OR_OK;
}

enum or_status or_graph_root(struct or_graph *g, unsigned target, int present)
{
    if (!graph_valid(g) || !target || target > 16 ||
        !(g->live & BIT(target - 1)))
        return OR_INVALID;
    if (present)
    {
        g->roots |= BIT(target - 1);
#ifndef OR_MUTATE_MARK2
        if (g->active && !g->color[target - 1])
            g->color[target - 1] = 1;
#endif
    }
    else
        g->roots &= (uint16_t)~BIT(target - 1);
    return OR_OK;
}

static int nursery_ref(const struct or_nursery *h, unsigned r)
{
    return !r || (r <= 8 ? r <= h->used : r <= 8u + h->old_used);
}

static enum or_status evacuate(const struct or_nursery *before,
                               struct or_nursery *after, unsigned r,
                               uint8_t forwarding[8], unsigned yc, unsigned oc,
                               uint8_t queue[8], unsigned *count, uint8_t *out)
{
    struct or_cell cell;
    unsigned n;
    if (!r || r > 8)
    {
        *out = (uint8_t)r;
        return OR_OK;
    }
    if (forwarding[r - 1])
    {
        *out = forwarding[r - 1];
        return OR_OK;
    }
    cell = before->young[before->active][r - 1];
    if (cell.age)
    {
        if (after->old_used >= oc)
            return OR_FULL;
        n = 9u + after->old_used;
        after->old[after->old_used++] = cell;
    }
    else
    {
        if (after->used >= yc)
            return OR_FULL;
        n = 1u + after->used;
        cell.age = 1;
        after->young[after->active][after->used++] = cell;
    }
    forwarding[r - 1] = (uint8_t)n;
    queue[(*count)++] = (uint8_t)n;
    *out = (uint8_t)n;
    return OR_OK;
}

enum or_status or_minor(struct or_nursery *h, unsigned yc, unsigned oc,
                        unsigned *copied)
{
    struct or_nursery t;
    uint8_t forward[8] = {0}, queue[8];
    unsigned i, j, count = 0, scan = 0;
    enum or_status r;
    if (!h || !copied || h->active > 1 || h->used > 8 || h->old_used > 8 ||
        yc > 8 || oc > 8 || h->old_used > oc)
        return OR_INVALID;
    for (i = 0; i < 4; i++)
        if (!nursery_ref(h, h->roots[i]))
            return OR_INVALID;
    for (i = 0; i < h->used + h->old_used; i++)
        for (j = 0; j < 2; j++)
            if (!nursery_ref(h, i < h->used ? h->young[h->active][i].edge[j]
                                            : h->old[i - h->used].edge[j]))
                return OR_INVALID;
    t = *h;
    t.active ^= 1;
    t.used = 0;
    for (i = 0; i < 4; i++)
    {
        r = evacuate(h, &t, h->roots[i], forward, yc, oc, queue, &count,
                     &t.roots[i]);
        if (r)
            return r;
    }
#ifndef OR_MUTATE_NURSERY
    for (i = 0; i < h->old_used; i++)
        for (j = 0; j < 2; j++)
        {
            r = evacuate(h, &t, h->old[i].edge[j], forward, yc, oc, queue,
                         &count, &t.old[i].edge[j]);
            if (r)
                return r;
        }
#endif
    while (scan < count)
    {
        unsigned id = queue[scan++];
        struct or_cell *cell =
            id <= 8 ? &t.young[t.active][id - 1] : &t.old[id - 9];
        for (j = 0; j < 2; j++)
        {
            r = evacuate(h, &t, cell->edge[j], forward, yc, oc, queue, &count,
                         &cell->edge[j]);
            if (r)
                return r;
        }
    }
#ifdef OR_MUTATE_NURSERY2
    for (i = 0; i < 4; i++)
        t.roots[i] = h->roots[i];
#endif
    *h = t;
    *copied = count;
    return OR_OK;
}

enum or_status or_card_store(struct or_cards *c, unsigned object,
                             unsigned field, unsigned target)
{
    if (!c || object >= 8 || field >= 2 || target > 16)
        return OR_INVALID;
    c->edge[object][field] = (uint8_t)target;
#ifndef OR_MUTATE_CARDS
    if (target && target <= 8)
        c->dirty |= (uint8_t)(1u << (object / 2));
#endif
    return OR_OK;
}

enum or_status or_card_scan(struct or_cards *c, uint16_t *roots,
                            unsigned *scanned)
{
    unsigned card, i, j, n = 0;
    uint16_t r = 0;
    uint8_t dirty = 0;
    if (!c || !roots || !scanned || (c->dirty & 0xf0))
        return OR_INVALID;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 2; j++)
            if (c->edge[i][j] > 16)
                return OR_INVALID;
    for (card = 0; card < 4; card++)
        if (c->dirty & (1u << card))
        {
            for (i = 2 * card; i < 2 * card + 2; i++)
                for (j = 0; j < 2; j++)
                {
                    unsigned t = c->edge[i][j];
                    n++;
                    if (t && t <= 8)
                    {
                        r |= BIT(t - 1);
                        dirty |= (uint8_t)(1u << card);
                    }
                }
        }
#ifdef OR_MUTATE_CARDS2
    dirty = 0;
#endif
    c->dirty = dirty;
    *roots = r;
    *scanned = n;
    return OR_OK;
}

enum or_status or_compact(struct or_compact *h, uint8_t forwarding[16])
{
    struct or_compact t;
    uint8_t f[16] = {0};
    unsigned i, j, n = 0;
    if (!h || !forwarding)
        return OR_INVALID;
#ifndef OR_MUTATE_COMPACT2
    if (h->borrows)
        return OR_STATE;
#endif
    for (i = 0; i < 16; i++)
    {
        unsigned slot = h->handle[i];
        if (slot > 16 || (slot && !(h->live & BIT(slot - 1))))
            return OR_INVALID;
        if (h->live & BIT(i))
            for (j = 0; j < 2; j++)
                if (h->cell[i].edge[j] > 16 ||
                    (h->cell[i].edge[j] && !h->handle[h->cell[i].edge[j] - 1]))
                    return OR_INVALID;
    }
    t = *h;
    t.live = 0;
    for (i = 0; i < 16; i++)
        if (h->live & BIT(i))
        {
            f[i] = (uint8_t)(n + 1);
            t.cell[n] = h->cell[i];
            t.live |= BIT(n);
            n++;
        }
#ifndef OR_MUTATE_COMPACT
    for (i = 0; i < 16; i++)
        if (h->handle[i])
            t.handle[i] = f[h->handle[i] - 1];
#endif
    *h = t;
    for (i = 0; i < 16; i++)
        forwarding[i] = f[i];
    return OR_OK;
}

static uint16_t closure(uint8_t edge[16][2], uint16_t seeds, uint16_t allowed)
{
    uint16_t last;
    unsigned i, j;
    seeds &= allowed;
    do
    {
        last = seeds;
        for (i = 0; i < 16; i++)
            if (seeds & BIT(i))
                for (j = 0; j < 2; j++)
                {
                    unsigned t = edge[i][j];
                    if (t && (allowed & BIT(t - 1)))
                        seeds |= BIT(t - 1);
                }
    } while (last != seeds);
    return seeds;
}

enum or_status or_weak_collect(struct or_weak *h, uint16_t *reclaimed)
{
    uint16_t reachable, last, keep, dead, queue;
    unsigned i;
    if (!h || !reclaimed || !graph_valid(&h->graph) || h->graph.active ||
        (h->pending & ~h->graph.live) || (h->finalizable & ~h->graph.live) ||
        (h->finalized & ~h->graph.live))
        return OR_INVALID;
    if (h->pending)
        return OR_STATE;
    for (i = 0; i < 4; i++)
        if (h->key[i] > 16 || h->value[i] > 16 || h->weak[i] > 16 ||
            (h->key[i] && !(h->graph.live & BIT(h->key[i] - 1))) ||
            (h->value[i] && !(h->graph.live & BIT(h->value[i] - 1))) ||
            (h->weak[i] && !(h->graph.live & BIT(h->weak[i] - 1))))
            return OR_INVALID;
    reachable = closure(h->graph.edge, h->graph.roots, h->graph.live);
    do
    {
        last = reachable;
        for (i = 0; i < 4; i++)
            if (h->key[i] && h->value[i] && (reachable & BIT(h->key[i] - 1)))
                reachable |= BIT(h->value[i] - 1);
        reachable = closure(h->graph.edge, reachable, h->graph.live);
#ifdef OR_MUTATE_WEAK
        break;
#endif
    } while (reachable != last);
    for (i = 0; i < 4; i++)
        if (h->weak[i] && !(reachable & BIT(h->weak[i] - 1)))
            h->weak[i] = 0;
    queue = (uint16_t)(h->finalizable & ~h->finalized & ~reachable);
    keep = closure(h->graph.edge, (uint16_t)(reachable | queue), h->graph.live);
    dead = (uint16_t)(h->graph.live & ~keep);
    h->pending = queue;
    h->graph.live &= keep;
    h->finalizable &= keep;
    h->finalized &= keep;
    for (i = 0; i < 4; i++)
        if ((h->key[i] && (dead & BIT(h->key[i] - 1))) ||
            (h->value[i] && (dead & BIT(h->value[i] - 1))))
            h->key[i] = h->value[i] = 0;
    *reclaimed = dead;
    return OR_OK;
}

enum or_status or_final_next(struct or_weak *h, unsigned *out)
{
    unsigned i;
    if (!h || !out)
        return OR_INVALID;
    if (!h->pending)
        return OR_DONE;
    for (i = 0; i < 16; i++)
        if (h->pending & BIT(i))
            break;
    h->pending &= (uint16_t)~BIT(i);
#ifndef OR_MUTATE_WEAK2
    h->finalized |= BIT(i);
#endif
    *out = i + 1;
    return OR_OK;
}

enum or_status or_trial_delete(struct or_counts *h, uint16_t candidates,
                               uint16_t *reclaimed)
{
    uint16_t trial[16], incoming[16] = {0}, seeds = 0, keep, dead;
    unsigned i, j;
    if (!h || !reclaimed || (candidates & ~h->live))
        return OR_INVALID;
    for (i = 0; i < 16; i++)
    {
        trial[i] = h->refs[i];
        if (h->live & BIT(i))
            for (j = 0; j < 2; j++)
            {
                unsigned t = h->edge[i][j];
                if (t > 16 || (t && !(h->live & BIT(t - 1))))
                    return OR_INVALID;
                if (t)
                    incoming[t - 1]++;
            }
    }
    for (i = 0; i < 16; i++)
        if ((h->live & BIT(i)) && h->refs[i] < incoming[i])
            return OR_INVALID;
    for (i = 0; i < 16; i++)
        if (candidates & BIT(i))
            for (j = 0; j < 2; j++)
            {
                unsigned t = h->edge[i][j];
                if (t && (candidates & BIT(t - 1)))
                {
#ifndef OR_MUTATE_TRIAL
                    trial[t - 1]--;
#endif
                }
            }
    for (i = 0; i < 16; i++)
        if ((candidates & BIT(i)) && trial[i])
            seeds |= BIT(i);
    keep = closure(h->edge, seeds, candidates);
#ifdef OR_MUTATE_TRIAL2
    keep = seeds;
#endif
    dead = (uint16_t)(candidates & ~keep);
    for (i = 0; i < 16; i++)
        if (dead & BIT(i))
            for (j = 0; j < 2; j++)
            {
                unsigned t = h->edge[i][j];
                if (t && !(dead & BIT(t - 1)))
                    h->refs[t - 1]--;
            }
    h->live &= (uint16_t)~dead;
    for (i = 0; i < 16; i++)
        if (dead & BIT(i))
        {
            h->refs[i] = 0;
            h->edge[i][0] = h->edge[i][1] = 0;
        }
    *reclaimed = dead;
    return OR_OK;
}

enum or_status or_safepoint(struct or_frame *f, const uint8_t forward[16],
                            const uint8_t lengths[16])
{
    struct or_frame t;
    unsigned i;
    if (!f || !forward || !lengths)
        return OR_INVALID;
    if (!f->at_safepoint)
        return OR_STATE;
    if (f->root_map & ~f->initialized)
        return OR_INVALID;
    t = *f;
    for (i = 0; i < 8; i++)
        if (f->initialized & (1u << i))
        {
            struct or_slot *s = &t.slot[i];
            int root = (f->root_map & (1u << i)) != 0;
#ifndef OR_MUTATE_SAFEPOINT
            if (root != (s->tag == 1))
                return OR_INVALID;
#endif
            if (s->tag > 1)
                return OR_INVALID;
            if (root)
            {
                if (!s->base || s->base > 16 || !forward[s->base - 1] ||
                    forward[s->base - 1] > 16 ||
                    s->offset > lengths[s->base - 1])
                    return OR_INVALID;
#ifndef OR_MUTATE_SAFEPOINT2
                s->base = forward[s->base - 1];
#endif
            }
        }
    *f = t;
    return OR_OK;
}
