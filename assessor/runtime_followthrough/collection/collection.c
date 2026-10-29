#include "collection.h"
#include <limits.h>
_Static_assert(UINT_MAX == UINT32_MAX, "Vg profile requires 32-bit unsigned");
#ifndef VG_MUTANT
#define VG_MUTANT 0
#endif
#define BIT(i) ((uint16_t)(1u << (i)))

static int valid(const struct vg_vg *g, struct cvm_value x)
{
    if ((unsigned)x.tag > CVM_REF)
    {
        return 0;
    }
    return x.tag != CVM_REF ||
           (x.integer >= 0 && (unsigned)x.integer < CVM_HEAP && x.generation &&
            g->vm.heap[x.integer].used &&
            g->vm.heap[x.integer].generation == x.generation);
}

static int ref(const struct vg_vg *g, struct vg_ref r)
{
    return g && g->epoch && r.owner == g &&
           (VG_MUTANT == 1 || r.epoch == g->epoch) && valid(g, r.value);
}

int vg_bind(struct vg_vg *g, struct cvm_value x, struct vg_ref *out)
{
    if (!g || !g->epoch || !out || !valid(g, x))
    {
        return VG_STALE;
    }
    *out = (struct vg_ref){g, g->epoch, x};
    return VG_OK;
}

int vg_reset(struct vg_vg *g, unsigned arena)
{
    if (!g || arena > 1)
    {
        return VG_BAD;
    }
    if (g->phase != VG_IDLE || g->epoch == UINT32_MAX)
    {
        return VG_STATE;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (g->clients[i])
        {
            return VG_STATE;
        }
    }
    g->epoch++;
    cvm_init(&g->vm, 0, UINT32_MAX, 0);
    g->vm.arena = (uint8_t)arena;
    orv_init(&g->registrations, &g->vm);
    for (unsigned i = 0; i < 16; i++)
    {
        g->clients[i] = 0;
        g->tickets[i] = 0;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        g->weak_present[i] = 0;
    }
    g->temporary_count = 0;
    g->inspected = g->scanned = g->commits = 0;
    return VG_OK;
}

static int root_value(struct vg_vg *g, struct cvm_value x, uint16_t *roots,
                      unsigned *inspected)
{
    (*inspected)++;
    if (!valid(g, x))
    {
        return VG_STALE;
    }
    if (x.tag == CVM_REF)
    {
        *roots |= BIT(x.integer);
    }
    return VG_OK;
}

static int roots(struct vg_vg *g, uint16_t *out, unsigned *inspected)
{
    uint16_t r = 0;
    if (g->vm.root_count > CVM_ROOTS || g->vm.sp > CVM_STACK ||
        g->vm.depth > CVM_FRAMES || g->temporary_count > 2)
    {
        return VG_BAD;
    }
    for (unsigned i = 0; i < g->vm.root_count; i++)
    {
        if (root_value(g, g->vm.roots[i], &r, inspected))
        {
            return VG_STALE;
        }
    }
    for (unsigned i = 0; i < g->vm.sp; i++)
    {
        if (root_value(g, g->vm.stack[i], &r, inspected))
        {
            return VG_STALE;
        }
    }
    for (unsigned i = 0; i < g->vm.depth; i++)
    {
        for (unsigned j = 0; j < CVM_LOCALS; j++)
        {
            if (g->vm.frames[i].initialized[j] > 1)
            {
                return VG_BAD;
            }
            if (g->vm.frames[i].initialized[j] &&
                root_value(g, g->vm.frames[i].locals[j], &r, inspected))
            {
                return VG_STALE;
            }
        }
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (g->registrations.roots[i] &&
            root_value(g, g->registrations.roots[i]->base, &r, inspected))
        {
            return VG_STALE;
        }
    }
    for (unsigned i = 0; i < g->temporary_count; i++)
    {
        if (root_value(g, g->temporaries[i], &r, inspected))
        {
            return VG_STALE;
        }
    }
    *out = r;
    return VG_OK;
}

static int refresh(struct vg_vg *g)
{
    if (g->phase == VG_IDLE)
    {
        return VG_OK;
    }
    uint16_t r;
    unsigned ignored = 0;
    int s = roots(g, &r, &ignored);
    if (s)
    {
        return s;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (g->graph.live & BIT(i))
        {
            if (VG_MUTANT == 3)
            {
                g->graph.roots =
                    (uint16_t)((g->graph.roots & ~BIT(i)) | (r & BIT(i)));
            }
            else if (or_graph_root(&g->graph, i + 1, (r & BIT(i)) != 0) !=
                     OR_OK)
            {
                return VG_BAD;
            }
        }
    }
    g->phase = VG_MARK;
    return VG_OK;
}

int vg_acquire(struct vg_vg *g, unsigned client, struct vg_ref r,
               struct vg_lease *out)
{
    if (!g || !out || client < 1 || client > 2)
    {
        return VG_BAD;
    }
    if (!ref(g, r))
    {
        return VG_STALE;
    }
    unsigned i;
    for (i = 0; i < 16; i++)
    {
        if (!g->clients[i])
        {
            break;
        }
    }
    if (i == 16)
    {
        return VG_FULL;
    }
    struct orv_derived before = g->roots[i];
    g->roots[i] = (struct orv_derived){r.value, 0};
    uint64_t ticket;
    enum or_status s = orv_register(&g->registrations, &g->roots[i], &ticket);
    if (s != OR_OK)
    {
        g->roots[i] = before;
        return VG_FULL;
    }
    g->clients[i] = (uint8_t)client;
    g->tickets[i] = ticket;
    if (refresh(g) != VG_OK)
    {
        return VG_BAD; /* valid stopped representation makes this unreachable */
    }
    *out = (struct vg_lease){g, g->epoch, ticket, (uint8_t)client};
    return VG_OK;
}

int vg_release(struct vg_vg *g, unsigned client, struct vg_lease lease)
{
    if (!g || lease.owner != g || lease.epoch != g->epoch ||
        client != lease.client || client < 1 || client > 2)
    {
        return VG_STALE;
    }
    unsigned i;
    for (i = 0; i < 16; i++)
    {
        if (g->clients[i] == client && g->tickets[i] == lease.ticket)
        {
            break;
        }
    }
    if (i == 16 || orv_unregister(&g->registrations, lease.ticket) != OR_OK)
    {
        return VG_STALE;
    }
    g->clients[i] = 0;
    g->tickets[i] = 0;
    return refresh(g);
}

int vg_weak_set(struct vg_vg *g, unsigned index, struct vg_ref r)
{
    if (!g || index >= 4)
    {
        return VG_BAD;
    }
    if (!ref(g, r) || r.value.tag != CVM_REF)
    {
        return VG_STALE;
    }
    g->weak[index] = r.value;
    g->weak_present[index] = 1;
    return VG_OK;
}

int vg_weak_acquire(struct vg_vg *g, unsigned index, unsigned client,
                    struct vg_lease *lease, struct vg_ref *out)
{
    if (!g || !lease || !out || index >= 4)
    {
        return VG_BAD;
    }
    if (!g->weak_present[index] || !valid(g, g->weak[index]))
    {
        return VG_STALE;
    }
    struct vg_ref r = {g, g->epoch, g->weak[index]};
    struct vg_lease l;
    int s = vg_acquire(g, client, r, &l);
    if (s)
    {
        return s;
    }
    *lease = l;
    *out = r;
    return VG_OK;
}

int vg_begin(struct vg_vg *g)
{
    if (!g || !g->epoch)
    {
        return VG_BAD;
    }
    if (g->phase != VG_IDLE || g->vm.arena)
    {
        return VG_STATE;
    }
    struct or_graph graph = {0};
    unsigned examined = 0;
    int s = roots(g, &graph.roots, &examined);
    if (s)
    {
        return s;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (g->vm.heap[i].used)
        {
            if (g->vm.heap[i].used != 1 || !g->vm.heap[i].generation)
            {
                return VG_BAD;
            }
            graph.live |= BIT(i);
            struct cvm_value a[2] = {g->vm.heap[i].left, g->vm.heap[i].right};
            for (unsigned j = 0; j < 2; j++)
            {
                examined++;
                if (!valid(g, a[j]))
                {
                    return VG_STALE;
                }
                if (a[j].tag == CVM_REF)
                {
                    graph.edge[i][j] = (uint8_t)(a[j].integer + 1);
                }
            }
        }
    }
    if (or_mark_begin(&graph) != OR_OK)
    {
        return VG_BAD;
    }
    g->graph = graph;
    g->phase = VG_MARK;
    g->inspected = examined;
    g->scanned = 0;
    return VG_OK;
}

int vg_step(struct vg_vg *g, unsigned budget, unsigned *scanned)
{
    if (!g || !scanned)
    {
        return VG_BAD;
    }
    if (g->phase == VG_IDLE)
    {
        return VG_STATE;
    }
    unsigned n;
    enum or_status s = or_mark_step(&g->graph, budget, &n);
    if (s != OR_OK && s != OR_DONE)
    {
        return VG_BAD;
    }
    g->scanned += n;
    *scanned = n;
    /* Keep barriers armed through the gap before commit, even when no gray node
     * remains. A root/store in that gap can create new work. */
    g->graph.active = 1;
    g->phase = s == OR_DONE ? VG_READY : VG_MARK;
    return s == OR_DONE ? VG_DONE : VG_OK;
}

int vg_store(struct vg_vg *g, struct vg_ref owner, unsigned field,
             struct vg_ref target)
{
    if (!g || field > 1)
    {
        return VG_BAD;
    }
    if (!ref(g, owner) || owner.value.tag != CVM_REF || !ref(g, target))
    {
        return VG_STALE;
    }
    if (g->phase != VG_IDLE)
    {
        unsigned slot = (unsigned)owner.value.integer,
                 t = target.value.tag == CVM_REF
                         ? (unsigned)target.value.integer + 1
                         : 0;
        if (VG_MUTANT == 2)
        {
            g->graph.edge[slot][field] = (uint8_t)t;
        }
        else if (or_graph_store(&g->graph, slot + 1, field, t) != OR_OK)
        {
            return VG_BAD;
        }
        g->phase = VG_MARK;
    }
    if (field)
    {
        g->vm.heap[owner.value.integer].right = target.value;
    }
    else
    {
        g->vm.heap[owner.value.integer].left = target.value;
    }
    return VG_OK;
}

int vg_field(struct vg_vg *g, struct vg_ref owner, unsigned field,
             struct vg_ref *out)
{
    if (!g || !out || field > 1)
    {
        return VG_BAD;
    }
    if (!ref(g, owner))
    {
        return VG_STALE;
    }
    struct cvm_value r;
    if (cvm_field(&g->vm, owner.value, (int)field, &r) != CVM_OK)
    {
        return VG_STALE;
    }
    return vg_bind(g, r, out);
}

int vg_commit(struct vg_vg *g, uint16_t *out)
{
    if (!g || !out)
    {
        return VG_BAD;
    }
    if (g->phase != VG_READY || g->commits == UINT32_MAX)
    {
        return VG_STATE;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (g->graph.color[i] == 1)
        {
            return VG_STATE;
        }
    }
    struct or_weak weak = {0};
    weak.graph = g->graph;
    weak.graph.active = 0;
    weak.graph.roots = 0;
    /* Strong closure has already been established by the incremental marker.
     * Weak processing receives that set, not the strong edges again: recomputing
     * closure here would silently hide an omitted incremental write barrier. */
    for (unsigned i = 0; i < 16; i++)
    {
        if (g->graph.color[i] == 2)
        {
            weak.graph.roots |= BIT(i);
        }
        weak.graph.edge[i][0] = weak.graph.edge[i][1] = 0;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (g->weak_present[i])
        {
            if (!valid(g, g->weak[i]))
            {
                return VG_STALE;
            }
            weak.weak[i] = (uint8_t)(g->weak[i].integer + 1);
            if (VG_MUTANT == 4)
            {
                weak.graph.roots |= BIT(g->weak[i].integer);
            }
        }
    }
    uint16_t dead;
    if (or_weak_collect(&weak, &dead) != OR_OK)
    {
        return VG_BAD;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (!weak.weak[i])
        {
            g->weak_present[i] = 0;
        }
    }
    for (unsigned i = 0; i < 16; i++)
    {
        g->vm.heap[i].marked = g->graph.color[i] == 2;
        if (dead & BIT(i))
        {
            g->vm.heap[i].used = 0;
        }
    }
    g->graph.active = 0;
    g->phase = VG_IDLE;
    g->commits++;
    *out = dead;
    return VG_OK;
}

int vg_pair(struct vg_vg *g, struct vg_ref a, struct vg_ref b,
            struct vg_ref *out)
{
    if (!g || !out)
    {
        return VG_BAD;
    }
    if (g->phase != VG_IDLE)
    {
        return VG_STATE;
    }
    if (!ref(g, a) || !ref(g, b))
    {
        return VG_STALE;
    }
    if (g->vm.allocations == UINT32_MAX)
    {
        return VG_FULL;
    }
    unsigned i;
    for (i = 0; i < 16; i++)
    {
        if (!g->vm.heap[i].used && g->vm.heap[i].generation != UINT32_MAX)
        {
            break;
        }
    }
    if (i == 16)
    {
        if (g->vm.arena)
        {
            return VG_FULL;
        }
        g->temporaries[0] = a.value;
        g->temporaries[1] = b.value;
        g->temporary_count = VG_MUTANT == 5 ? 0 : 2;
        int s = vg_begin(g);
        unsigned scanned;
        uint16_t dead;
        if (s == VG_OK)
        {
            s = vg_step(g, 16, &scanned);
            if (s == VG_DONE)
            {
                s = vg_commit(g, &dead);
            }
        }
        g->temporary_count = 0;
        if (s != VG_OK)
        {
            return s;
        }
        for (i = 0; i < 16; i++)
        {
            if (!g->vm.heap[i].used && g->vm.heap[i].generation != UINT32_MAX)
            {
                break;
            }
        }
        if (i == 16)
        {
            return VG_FULL;
        }
    }
    struct cvm_value r;
    if (cvm_pair(&g->vm, a.value, b.value, &r) != CVM_OK)
    {
        return VG_STALE;
    }
    return vg_bind(g, r, out);
}

int vg_transfer(struct vg_vg *source, struct vg_vg *dest, struct vg_ref root,
                unsigned cut, struct vg_ref *out)
{
    if (!source || !dest || source == dest || !out)
    {
        return VG_BAD;
    }
    if (source->phase != VG_IDLE || dest->phase != VG_IDLE ||
        !source->vm.arena || !dest->vm.arena)
    {
        return VG_STATE;
    }
    if (!ref(source, root) || !dest->epoch)
    {
        return VG_STALE;
    }
    uint8_t queue[16], used[16] = {0};
    unsigned count = 0, scan = 0, step = 0;
    if (root.value.tag == CVM_REF)
    {
        queue[count++] = (uint8_t)root.value.integer;
        used[root.value.integer] = 1;
    }
    while (scan < count)
    {
        unsigned slot = queue[scan++];
        struct cvm_value a[2] = {source->vm.heap[slot].left,
                                 source->vm.heap[slot].right};
        for (unsigned j = 0; j < 2; j++)
        {
            if (!valid(source, a[j]))
            {
                return VG_STALE;
            }
            if (a[j].tag == CVM_REF && !used[a[j].integer])
            {
                used[a[j].integer] = 1;
                queue[count++] = (uint8_t)a[j].integer;
            }
        }
    }
    if (cut > 3 * count + 1)
    {
        return VG_BAD;
    }
    struct cvm_cvm staged = dest->vm;
    struct cvm_value map[16];
    for (unsigned i = 0; i < count; i++)
    {
        if (staged.allocations == UINT32_MAX ||
            cvm_pair(&staged, cvm_int(0), cvm_int(0), &map[queue[i]]) != CVM_OK)
        {
            return VG_FULL;
        }
        if (cut == ++step)
        {
            return VG_FULL;
        }
    }
    for (unsigned i = 0; i < count; i++)
    {
        unsigned old = queue[i], now = (unsigned)map[old].integer;
        struct cvm_value a[2] = {source->vm.heap[old].left,
                                 source->vm.heap[old].right};
        for (unsigned j = 0; j < 2; j++)
        {
            struct cvm_value value = a[j];
            if (value.tag == CVM_REF)
            {
                value = map[value.integer];
            }
            if (j)
            {
                staged.heap[now].right = value;
            }
            else
            {
                staged.heap[now].left = value;
            }
            if (cut == ++step)
            {
                return VG_FULL;
            }
        }
    }
    if (VG_MUTANT == 6)
    {
        dest->vm = staged;
    }
    if (cut == ++step)
    {
        return VG_FULL;
    }
    dest->vm = staged;
    *out = (struct vg_ref){dest, dest->epoch,
                           root.value.tag == CVM_REF ? map[root.value.integer]
                                                     : root.value};
    return VG_OK;
}
