#include "object_runtime_generational.h"
#ifndef ORGEN_MUTANT
#define ORGEN_MUTANT 0
#endif
#define B(i) ((uint16_t)(UINT32_C(1) << (i)))

static int reference(const struct or_nursery *h, unsigned r)
{
    return !r || (r <= 8 ? r <= h->used : r <= 8u + h->old_used);
}

static int admitted(const struct or_nursery *h)
{
    unsigned i, j;
    if (!h || h->active > 1 || h->used > 8 || h->old_used > 8)
        return 0;
    for (i = 0; i < 4; i++)
        if (!reference(h, h->roots[i]))
            return 0;
    for (i = 0; i < h->used + h->old_used; i++)
    {
        const struct or_cell *p =
            i < h->used ? &h->young[h->active][i] : &h->old[i - h->used];
        if (p->age > 1)
            return 0;
        for (j = 0; j < 2; j++)
            if (!reference(h, p->edge[j]))
                return 0;
    }
    return 1;
}

static void rebuild(const struct or_nursery *h, struct or_cards *c)
{
    unsigned i, j;
    *c = (struct or_cards){0};
    for (i = 0; i < h->old_used; i++)
        for (j = 0; j < 2; j++)
            (void)or_card_store(c, i, j, h->old[i].edge[j]);
}

static int coherent(const struct or_nursery *h, const struct or_cards *c)
{
    unsigned i, j;
    if (c->dirty & 0xf0)
        return 0;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 2; j++)
            if (c->edge[i][j] != (i < h->old_used ? h->old[i].edge[j] : 0))
                return 0;
    /* Dirty-card completeness is established by initialization and every
       adapter store. Do not repair a missing barrier by silently scanning all. */
    return 1;
}

enum or_status or_nursery_cards_init(const struct or_nursery *h,
                                     struct or_cards *out)
{
    if (!out || !admitted(h))
        return OR_INVALID;
    rebuild(h, out);
    return OR_OK;
}

enum or_status or_nursery_old_store(struct or_nursery *h,
                                    struct or_cards *cards, unsigned object,
                                    unsigned field, unsigned target)
{
    enum or_status status;
    if (!admitted(h) || !cards || object >= h->old_used || field >= 2 ||
        !reference(h, target) || !coherent(h, cards))
        return OR_INVALID;
    if (ORGEN_MUTANT == 3)
    {
        cards->edge[object][field] = (uint8_t)target;
    }
    else
    {
        status = or_card_store(cards, object, field, target);
        if (status != OR_OK)
            return status;
    }
    h->old[object].edge[field] = (uint8_t)target;
    return OR_OK;
}

static void discover(unsigned r, uint8_t seen[8], uint8_t queue[8], unsigned *n)
{
    if (r && r <= 8 && !seen[r - 1])
    {
        seen[r - 1] = 1;
        queue[(*n)++] = (uint8_t)r;
    }
}

static uint8_t moved(unsigned r, const uint8_t forwarding[8])
{
    return r && r <= 8 ? forwarding[r - 1] : (uint8_t)r;
}

enum or_status or_minor_preflight(struct or_nursery *h, struct or_cards *cards,
                                  unsigned yc, unsigned oc, unsigned *copied,
                                  unsigned *scanned)
{
    uint8_t seen[8] = {0}, queue[8], forward[8] = {0};
    unsigned n = 0, scan = 0, young = 0, old, i, j, fields;
    unsigned destination;
    struct or_cards card_snapshot;
    uint16_t old_roots = 0;
    if (!copied || !scanned || !admitted(h) || yc > 8 || oc > 8 ||
        h->old_used > oc || (cards && !coherent(h, cards)))
        return OR_INVALID;
    old = h->old_used;
    fields = 2u * old;
    if (cards)
    {
        card_snapshot = *cards;
        if (or_card_scan(&card_snapshot, &old_roots, &fields) != OR_OK)
            return OR_INVALID;
    }
    for (i = 0; i < 4; i++)
        discover(h->roots[i], seen, queue, &n);
    if (ORGEN_MUTANT != 1)
        for (i = 0; i < old; i++)
            for (j = 0; j < 2; j++)
            {
                unsigned r = h->old[i].edge[j];
                if (!cards || (r && r <= 8 && (old_roots & B(r - 1))))
                    discover(r, seen, queue, &n);
            }
    while (scan < n)
    {
        const struct or_cell *cell = &h->young[h->active][queue[scan++] - 1];
        for (j = 0; j < 2; j++)
            discover(cell->edge[j], seen, queue, &n);
    }
    for (i = 0; i < n; i++)
    {
        unsigned source = queue[i];
        const struct or_cell *cell = &h->young[h->active][source - 1];
        if (cell->age)
        {
            if (old >= oc)
            {
                if (ORGEN_MUTANT == 2)
                    h->roots[0] = 0;
                return OR_FULL;
            }
            forward[source - 1] = (uint8_t)(9u + old++);
        }
        else
        {
            if (young >= yc)
            {
                if (ORGEN_MUTANT == 2)
                    h->roots[0] = 0;
                return OR_FULL;
            }
            forward[source - 1] = (uint8_t)(1u + young++);
        }
    }
    /* All validation/capacity failures precede this point. From-space remains
       untouched while destination young cells and appended old cells are built.
     */
    destination = h->active ^ 1u;
    for (i = 0; i < n; i++)
    {
        unsigned source = queue[i], target = forward[source - 1];
        struct or_cell cell = h->young[h->active][source - 1];
        cell.age = 1;
        for (j = 0; j < 2; j++)
            cell.edge[j] = moved(cell.edge[j], forward);
        if (target <= 8)
            h->young[destination][target - 1] = cell;
        else
            h->old[target - 9] = cell;
    }
    for (i = 0; i < h->old_used; i++)
        for (j = 0; j < 2; j++)
            h->old[i].edge[j] = moved(h->old[i].edge[j], forward);
    for (i = 0; i < 4; i++)
        h->roots[i] = moved(h->roots[i], forward);
    h->active = (uint8_t)destination;
    h->used = (uint8_t)young;
    h->old_used = (uint8_t)old;
    if (cards)
        rebuild(h, cards);
    *copied = n;
    *scanned = fields;
    return OR_OK;
}

static enum or_status major(struct or_nursery *h, struct or_cards *cards,
                            uint16_t retained, unsigned *reclaimed,
                            uint8_t forwarding[16])
{
    struct or_graph graph = {0};
    uint8_t map[16] = {0};
    unsigned i, j, young = 0, old = 0, scans, before, destination;
    enum or_status status;
    if (!reclaimed || !forwarding || !admitted(h) ||
        (cards && !coherent(h, cards)))
        return OR_INVALID;
    before = h->used + h->old_used;
    for (i = 0; i < h->used + h->old_used; i++)
    {
        unsigned id = i < h->used ? i + 1 : i - h->used + 9;
        const struct or_cell *cell =
            i < h->used ? &h->young[h->active][i] : &h->old[i - h->used];
        graph.live |= B(id - 1);
        for (j = 0; j < 2; j++)
            graph.edge[id - 1][j] = cell->edge[j];
        if (ORGEN_MUTANT == 4 && id > 8)
            graph.roots |= B(id - 1);
    }
    for (i = 0; i < 4; i++)
        if (h->roots[i])
            graph.roots |= B(h->roots[i] - 1);
    if (retained & (uint16_t)~graph.live)
        return OR_INVALID;
    graph.roots |= retained;
    status = or_mark_begin(&graph);
    if (status != OR_OK)
        return status;
    do
    {
        status = or_mark_step(&graph, 16, &scans);
    } while (status == OR_OK);
    if (status != OR_DONE)
        return status;
    for (i = 0; i < 16; i++)
        if ((graph.live & B(i)) && graph.color[i] == 2)
            map[i] = (uint8_t)(i < 8 ? ++young : 9u + old++);
    destination = h->active ^ 1u;
    for (i = 0; i < h->used; i++)
        if (map[i])
        {
            struct or_cell cell = h->young[h->active][i];
            for (j = 0; j < 2; j++)
                if (cell.edge[j])
                    cell.edge[j] = map[cell.edge[j] - 1];
            h->young[destination][map[i] - 1] = cell;
        }
    /* Stable left compaction: destination index never exceeds source index. */
    for (i = 0; i < h->old_used; i++)
        if (map[i + 8])
        {
            struct or_cell cell = h->old[i];
            for (j = 0; j < 2; j++)
                if (cell.edge[j])
                    cell.edge[j] = map[cell.edge[j] - 1];
            h->old[map[i + 8] - 9] = cell;
        }
    for (i = old; i < h->old_used; i++)
        h->old[i] = (struct or_cell){0};
    for (i = 0; i < 4; i++)
        if (h->roots[i])
            h->roots[i] = map[h->roots[i] - 1];
    h->active = (uint8_t)destination;
    h->used = (uint8_t)young;
    h->old_used = (uint8_t)old;
    if (cards)
        rebuild(h, cards);
    for (i = 0; i < 16; i++)
        forwarding[i] = map[i];
    *reclaimed = before - young - old;
    return OR_OK;
}

enum or_status or_major_preflight(struct or_nursery *h, struct or_cards *cards,
                                  unsigned *reclaimed, uint8_t forwarding[16])
{
    return major(h, cards, 0, reclaimed, forwarding);
}

static int large_valid(const struct or_large_heap *h, struct or_large_handle p)
{
    return h && p.slot >= 1 && p.slot <= 4 && p.generation &&
           h->block[p.slot - 1].active &&
           h->block[p.slot - 1].generation == p.generation;
}

static unsigned descriptor(const struct or_large_heap *h, unsigned block)
{
    unsigned i;
    for (i = 0; i < h->heap.old_used; i++)
        if (h->heap.old[i].value == (int32_t)(1000 + block))
            return i + 9;
    return 0;
}

void or_large_init(struct or_large_heap *h)
{
    unsigned i;
    if (!h)
        return;
    *h = (struct or_large_heap){0};
    for (i = 0; i < 4; i++)
    {
        h->block[i].generation = 1;
        h->ticket[i].generation = 1;
    }
}

enum or_status or_large_allocate(struct or_large_heap *h, unsigned length,
                                 uint32_t pattern, struct or_large_handle *out)
{
    unsigned block, root, start, i, j, used = 0, mask;
    if (!h || !out || !length || length > 4)
        return OR_INVALID;
    for (block = 0; block < 4; block++)
        if (!h->block[block].active && h->block[block].generation <= 3)
            break;
    for (root = 0; root < 4; root++)
        if (!h->heap.roots[root])
            break;
    if (block == 4 || root == 4 || h->heap.old_used == 8)
        return OR_FULL;
    for (i = 0; i < 4; i++)
        if (h->block[i].active)
            for (j = 0; j < h->block[i].length; j++)
                used |= 1u << (h->block[i].start + j);
    for (start = 0; start + length <= 8; start++)
    {
        mask = ((1u << length) - 1) << start;
        if (!(mask & used))
            break;
    }
    if (start + length > 8)
    {
        if (ORGEN_MUTANT == 6)
            start = 0;
        else
            return OR_FULL;
    }
    for (i = 0; i < length; i++)
        h->words[start + i] = pattern;
    h->block[block] = (struct or_large_block){(uint8_t)start, (uint8_t)length,
                                              0, 1, h->block[block].generation};
    h->heap.old[h->heap.old_used] =
        (struct or_cell){.value = (int32_t)(1000 + block), .age = 1};
    h->heap.roots[root] = (uint8_t)(9 + h->heap.old_used++);
    rebuild(&h->heap, &h->cards);
    *out = (struct or_large_handle){(uint8_t)(block + 1),
                                    h->block[block].generation};
    return OR_OK;
}

enum or_status or_large_release(struct or_large_heap *h,
                                struct or_large_handle p)
{
    unsigned ref, i;
    if (!large_valid(h, p))
        return OR_INVALID;
    ref = descriptor(h, p.slot - 1);
    if (!ref)
        return OR_STATE;
    for (i = 0; i < 4; i++)
        if (h->heap.roots[i] == ref)
        {
            h->heap.roots[i] = 0;
            return OR_OK;
        }
    return OR_STATE;
}

enum or_status or_large_pin(struct or_large_heap *h, struct or_large_handle p,
                            struct or_large_lease *out, const uint32_t **data,
                            unsigned *length)
{
    unsigned i;
    struct or_large_block *b;
    if (!large_valid(h, p) || !out || !data || !length)
        return OR_INVALID;
    b = &h->block[p.slot - 1];
    if (b->pins == 255)
        return OR_FULL;
    for (i = 0; i < 4; i++)
        if (!h->ticket[i].active && h->ticket[i].generation <= 3)
            break;
    if (i == 4)
        return OR_FULL;
    h->ticket[i].object = p;
    h->ticket[i].active = 1;
    b->pins++;
    *out =
        (struct or_large_lease){h, (uint8_t)(i + 1), h->ticket[i].generation};
    *data = &h->words[b->start];
    *length = b->length;
    return OR_OK;
}

enum or_status or_large_unpin(struct or_large_lease p)
{
    struct or_large_ticket *t;
    if (!p.owner || p.slot < 1 || p.slot > 4)
        return OR_INVALID;
    t = &p.owner->ticket[p.slot - 1];
    if (!t->active || !p.generation || p.generation != t->generation ||
        !large_valid(p.owner, t->object))
        return OR_STATE;
    p.owner->block[t->object.slot - 1].pins--;
    t->active = 0;
    t->generation++;
    return OR_OK;
}

enum or_status or_large_collect(struct or_large_heap *h, unsigned *reclaimed)
{
    uint16_t retained = 0;
    uint8_t forward[16];
    unsigned i, j, n;
    enum or_status status;
    if (!h || !reclaimed)
        return OR_INVALID;
    for (i = 0; i < 4; i++)
        if (h->block[i].active && h->block[i].pins && ORGEN_MUTANT != 5)
        {
            unsigned ref = descriptor(h, i);
            if (!ref)
                return OR_STATE;
            retained |= B(ref - 1);
        }
    status = major(&h->heap, &h->cards, retained, &n, forward);
    if (status != OR_OK)
        return status;
    for (i = 0; i < 4; i++)
        if (h->block[i].active && !descriptor(h, i))
        {
            for (j = 0; j < h->block[i].length; j++)
                h->words[h->block[i].start + j] = 0;
            h->block[i].active = 0;
            h->block[i].pins = 0;
            h->block[i].generation++;
        }
    *reclaimed = n;
    return OR_OK;
}
