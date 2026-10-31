/*
 * Reference accessors for the consume/index companion (see CONTRACTS.md).
 *
 * Each accessor loads the publication pointer once. They then differ in how
 * the result reaches the array index:
 *   ci_pointer         reads a field of the loaded node (consume load);
 *   ci_value_index     indexes with a field of the loaded node (dependency
 *                      chain from a consume load);
 *   ci_branch_index    only branches on the pointer (control dependency from
 *                      a consume load);
 *   ci_killed_index    indexes through kill_dependency, ending the chain
 *                      (consume load);
 *   ci_acquire_branch  branches on the pointer after an acquire load.
 * The load orders and the operand flow are the subject of the lesson and the
 * object-code audit, so they are kept exactly as written.
 *
 * CI_MUTATION selects a deliberately broken variant (1..3) for the
 * rejection campaign; 0 is the reference.
 */
#include "consume_index.h"
#ifndef CI_MUTATION
#define CI_MUTATION 0
#endif

/* The graph model has four vertices and one edge per vertex pair. */
enum
{
    GRAPH_VERTICES = 4,
    EDGE_KINDS = 3,
    EDGE_ABSENT = 0,
    EDGE_VALUE = 1,
    EDGE_CONTROL = 2
};

unsigned ci_pointer(_Atomic(struct ci_node *) *publication)
{
    struct ci_node *n = atomic_load_explicit(publication, memory_order_consume);
    return n ? n->value : 0;
}

unsigned ci_value_index(_Atomic(struct ci_node *) *publication,
                        const unsigned values[4])
{
    struct ci_node *n = atomic_load_explicit(publication, memory_order_consume);
    if (!n)
    {
        return 0;
    }
#if CI_MUTATION == 1
    return values[1];
#else
    unsigned index = n->index;
    return values[index];
#endif
}

unsigned ci_branch_index(_Atomic(struct ci_node *) *publication,
                         const unsigned values[4])
{
    struct ci_node *n = atomic_load_explicit(publication, memory_order_consume);
    unsigned index;
    if (n)
    {
        index = 1;
    }
    else
    {
        index = 0;
    }
#if CI_MUTATION == 2
    index = n ? 0 : 1;
#endif
    return values[index];
}

unsigned ci_killed_index(_Atomic(struct ci_node *) *publication,
                         const unsigned values[4])
{
    struct ci_node *n = atomic_load_explicit(publication, memory_order_consume);
    if (!n)
    {
        return 0;
    }
    unsigned index = kill_dependency(n->index);
    return values[index];
}

unsigned ci_acquire_branch(_Atomic(struct ci_node *) *publication,
                           const unsigned values[4])
{
    struct ci_node *n = atomic_load_explicit(publication, memory_order_acquire);
    unsigned index;
    if (n)
    {
        index = 1;
    }
    else
    {
        index = 0;
    }
    return values[index];
}

/*
 * Decodes `code` (0 .. 3^6 - 1, one base-three digit per edge, in the order
 * 01,02,03,12,13,23) and returns the set of vertices reachable from vertex 0
 * through value edges only, as a bitmask. Control edges never carry the
 * dependency. Edges only run from a lower to a higher vertex, so one pass in
 * vertex order sees every path.
 */
unsigned ci_dependency_graph(unsigned code)
{
    unsigned reachable = 1;
    for (unsigned from = 0; from < GRAPH_VERTICES - 1; ++from)
    {
        for (unsigned to = from + 1; to < GRAPH_VERTICES; ++to)
        {
            unsigned kind = code % EDGE_KINDS;
            code /= EDGE_KINDS;
            int from_reachable = (reachable & (1u << from)) != 0;
#if CI_MUTATION == 3
            int carries_dependency = kind != EDGE_ABSENT;
#else
            int carries_dependency = kind == EDGE_VALUE;
#endif
            if (carries_dependency && from_reachable)
            {
                reachable |= 1u << to;
            }
        }
    }
    return reachable;
}
