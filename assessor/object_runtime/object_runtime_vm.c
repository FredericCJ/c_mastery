#include "object_runtime_vm.h"
#include <string.h>
#ifndef ORV_MUTANT
#define ORV_MUTANT 0
#endif
#define BIT(i) ((uint16_t)(1u << (i)))
#define MAX_SLOTS (CVM_ROOTS + CVM_STACK + CVM_FRAMES * CVM_LOCALS + 16u)

static int valid(const struct cvm_cvm *v, struct cvm_value x)
{
    if (x.tag < CVM_NIL || x.tag > CVM_REF)
        return 0;
    if (x.tag == CVM_REF)
        return x.integer >= 0 && (unsigned)x.integer < CVM_HEAP &&
               x.generation && v->heap[x.integer].used &&
               v->heap[x.integer].generation == x.generation;
    return 1;
}

static int derived(const struct cvm_cvm *v, const struct orv_derived *r)
{
    return r && valid(v, r->base) &&
           r->offset <= (r->base.tag == CVM_REF ? 2u : 0u);
}

static enum or_status ready(const struct or_vm *o)
{
    return !o || !o->vm ? OR_INVALID : !o->paused ? OR_STATE : OR_OK;
}

void orv_init(struct or_vm *o, struct cvm_cvm *v)
{
    memset(o, 0, sizeof *o);
    o->vm = v;
    o->paused = 1;
    algo_pool_init(&o->registrations);
}

enum or_status orv_pause(struct or_vm *o, int paused)
{
    if (!o || !o->vm || paused < 0 || paused > 1)
        return OR_INVALID;
    if (o->foreign)
        return OR_STATE;
    o->paused = (uint8_t)paused;
    return OR_OK;
}

enum or_status orv_register(struct or_vm *o, struct orv_derived *r,
                            uint64_t *out)
{
    unsigned i;
    uint64_t h;
    enum or_status s = ready(o);
    if (s)
        return s;
    if (!out || !derived(o->vm, r))
        return OR_INVALID;
    for (i = 0; i < 16; i++)
        if (o->roots[i] == r)
            return OR_STATE;
    for (i = 0; i < 16; i++)
        if (!o->roots[i])
            break;
    if (i == 16)
        return OR_FULL;
    if (algo_pool_take(&o->registrations, (int32_t)i, &h) != ALGO_OK)
        return OR_FULL;
    o->roots[i] = r;
    *out = h;
    return OR_OK;
}

enum or_status orv_unregister(struct or_vm *o, uint64_t h)
{
    int32_t i;
    enum or_status s = ready(o);
    if (s)
        return s;
    if (algo_pool_get(&o->registrations, h, &i) != ALGO_OK)
        return OR_MISS;
    if (i < 0 || i >= 16 || !o->roots[i])
        return OR_STATE;
    if (algo_pool_release(&o->registrations, h) != ALGO_OK)
        return OR_STATE;
    o->roots[i] = NULL;
    return OR_OK;
}

static struct cvm_value relocated(struct cvm_value x, const uint8_t f[16],
                                  const struct cvm_cvm *after)
{
    if (x.tag == CVM_REF)
    {
        x.integer = (int32_t)f[x.integer] - 1;
        x.generation = after->heap[x.integer].generation;
    }
    return x;
}

enum or_status orv_collect(struct or_vm *o, unsigned budget, unsigned cut,
                           struct orv_stats *out)
{
    struct cvm_cvm t;
    struct or_graph graph = {0};
    struct cvm_value *loc[MAX_SLOTS];
    struct orv_derived external[16];
    uint8_t offsets[MAX_SLOTS] = {0}, f[16] = {0}, length[16];
    struct orv_stats stats = {0};
    unsigned i, j, n = 0, next = 0, scanned;
    enum or_status s = ready(o);
    if (s)
        return s;
    if (!out || cut > 4)
        return OR_INVALID;
    if (o->vm->arena)
        return OR_STATE;
    if (o->vm->sp > CVM_STACK || o->vm->depth > CVM_FRAMES ||
        o->vm->root_count > CVM_ROOTS)
        return OR_INVALID;
    if (o->vm->collections == UINT32_MAX)
        return OR_FULL;
    t = *o->vm;
    for (i = 0; i < t.root_count; i++)
        loc[n++] = &t.roots[i];
    for (i = 0; i < t.sp; i++)
        loc[n++] = &t.stack[i];
    for (i = 0; i < t.depth; i++)
        for (j = 0; j < CVM_LOCALS; j++)
        {
            if (t.frames[i].initialized[j] > 1)
                return OR_INVALID;
            if (t.frames[i].initialized[j])
            {
#if ORV_MUTANT != 1
                loc[n++] = &t.frames[i].locals[j];
#endif
            }
        }
    for (i = 0; i < 16; i++)
        if (o->roots[i])
        {
            if (!derived(o->vm, o->roots[i]))
                return OR_INVALID;
            external[i] = *o->roots[i];
            offsets[n] = external[i].offset;
            loc[n++] = &external[i].base;
        }
    for (i = 0; i < n; i++)
    {
        if (!valid(o->vm, *loc[i]))
            return OR_INVALID;
        if (loc[i]->tag == CVM_REF)
        {
            graph.roots |= BIT(loc[i]->integer);
            stats.root_copies++;
        }
    }
    for (i = 0; i < CVM_HEAP; i++)
    {
        if (t.heap[i].used > 1)
            return OR_INVALID;
        length[i] = 2;
        if (t.heap[i].used)
        {
            struct cvm_value fields[2] = {t.heap[i].left, t.heap[i].right};
            if (!t.heap[i].generation)
                return OR_INVALID;
            graph.live |= BIT(i);
            for (j = 0; j < 2; j++)
            {
                if (!valid(o->vm, fields[j]))
                    return OR_INVALID;
                if (fields[j].tag == CVM_REF)
                    graph.edge[i][j] = (uint8_t)(fields[j].integer + 1);
                stats.work++;
            }
        }
    }
    stats.work += n + CVM_HEAP;
    if (cut == 1)
        return OR_FULL;
    s = or_mark_begin(&graph);
    if (s)
        return s;
    do
    {
        s = or_mark_step(&graph, 16, &scanned);
        stats.work += scanned;
    } while (s == OR_OK);
    if (s != OR_DONE)
        return s;
    if (stats.work > budget || cut == 2)
        return OR_FULL;
    for (i = 0; i < 16; i++)
        if (graph.color[i] == 2)
        {
            while (next < i && o->vm->heap[next].generation == UINT32_MAX)
                next++;
            f[i] = (uint8_t)(next + 1);
            stats.live++;
            stats.moved += (next != i);
            next++;
        }
    for (i = 0; i < 16; i++)
    {
        t.heap[i].used = 0;
        t.heap[i].marked = 0;
    }
    for (i = 0; i < 16; i++)
        if (f[i])
        {
            unsigned destination = f[i] - 1;
            t.heap[destination] = o->vm->heap[i];
            t.heap[destination].marked = 1;
#if ORV_MUTANT == 2
            t.heap[destination].generation = o->vm->heap[i].generation;
#else
            t.heap[destination].generation =
                destination == i ? o->vm->heap[i].generation
                                 : o->vm->heap[destination].generation + 1;
#endif
        }
    for (i = 0; i < 16; i++)
        if (t.heap[i].used)
        {
            t.heap[i].left = relocated(t.heap[i].left, f, &t);
            t.heap[i].right = relocated(t.heap[i].right, f, &t);
        }
    for (i = 0; i < n; i += 8)
    {
        struct or_frame frame = {0};
        unsigned count = n - i < 8 ? n - i : 8;
        frame.at_safepoint = 1;
        for (j = 0; j < count; j++)
        {
            struct cvm_value x = *loc[i + j];
            frame.initialized |= (uint8_t)(1u << j);
            frame.slot[j].scalar = (uint32_t)x.integer;
            if (x.tag == CVM_REF)
            {
                frame.root_map |= (uint8_t)(1u << j);
                frame.slot[j].tag = 1;
                frame.slot[j].base = (uint8_t)(x.integer + 1);
                frame.slot[j].offset = offsets[i + j];
            }
        }
        s = or_safepoint(&frame, f, length);
        if (s)
            return s;
        for (j = 0; j < count; j++)
            if (frame.root_map & (1u << j))
            {
                loc[i + j]->integer = (int32_t)frame.slot[j].base - 1;
                loc[i + j]->generation = t.heap[loc[i + j]->integer].generation;
            }
        stats.frames++;
    }
    if (cut == 3)
        return OR_FULL;
    stats.slots = n;
    memcpy(stats.forwarding, f, sizeof f);
    t.collections++;
    if (cut == 4)
        return OR_FULL;
    *o->vm = t;
    for (i = 0; i < 16; i++)
        if (o->roots[i])
            *o->roots[i] = external[i];
    *out = stats;
    return OR_OK;
}

enum or_status orv_pair(struct or_vm *o, struct orv_derived *a,
                        struct orv_derived *b, int force, unsigned cut,
                        struct cvm_value *out)
{
    struct orv_stats stats;
    enum or_status s = ready(o);
    unsigned i;
    struct cvm_value result;
    if (s)
        return s;
    if (!out || !derived(o->vm, a) || !derived(o->vm, b) || a->offset ||
        b->offset || force < 0 || force > 1 || cut > 4)
        return OR_INVALID;
    for (i = 0; i < 16; i++)
        if (!o->vm->heap[i].used && o->vm->heap[i].generation != UINT32_MAX)
            break;
    if (force || i == 16)
    {
        int found_a = 0, found_b = 0;
        for (i = 0; i < 16; i++)
        {
            found_a |= o->roots[i] == a;
            found_b |= o->roots[i] == b;
        }
        if (!found_a || !found_b)
            return OR_STATE;
        s = orv_collect(o, UINT32_MAX, cut, &stats);
        if (s)
            return s;
    }
    for (i = 0; i < 16; i++)
        if (!o->vm->heap[i].used && o->vm->heap[i].generation != UINT32_MAX)
            break;
    if (i == 16 || o->vm->allocations == UINT32_MAX)
        return OR_FULL;
    if (cvm_pair(o->vm, a->base, b->base, &result) != CVM_OK)
        return OR_STATE;
    *out = result;
    return OR_OK;
}

enum or_status orv_call(struct or_vm *o, struct cvm_value a, struct cvm_value b,
                        orv_foreign_fn callback, void *context,
                        struct cvm_value *out)
{
    struct orv_derived left = {a, 0}, right = {b, 0};
    uint64_t first, second;
    struct cvm_value result;
    enum or_status s = ready(o);
    if (s)
        return s;
    if (!callback || !out || !valid(o->vm, a) || !valid(o->vm, b))
        return OR_INVALID;
    if (o->foreign)
        return OR_STATE;
    s = orv_register(o, &left, &first);
    if (s)
        return s;
    s = orv_register(o, &right, &second);
    if (s)
    {
        (void)orv_unregister(o, first);
        return s;
    }
    o->foreign = 1;
    s = callback(o, &left, &right, context, &result);
    o->foreign = 0;
#if ORV_MUTANT != 3
    (void)orv_unregister(o, second);
    (void)orv_unregister(o, first);
#endif
    if (s == OR_OK)
    {
        if (!valid(o->vm, result))
            return OR_INVALID;
#if ORV_MUTANT != 4
        *out = result;
#endif
    }
    return s;
}
