#include "consume_index.h"
#ifndef CI_MUTATION
#define CI_MUTATION 0
#endif
unsigned ci_pointer(_Atomic(CINode *) *publication) {
    CINode *n = atomic_load_explicit(publication, memory_order_consume);
    return n ? n->value : 0;
}
unsigned ci_value_index(_Atomic(CINode *) *publication, const unsigned values[4]) {
    CINode *n = atomic_load_explicit(publication, memory_order_consume);
    if (!n) return 0;
#if CI_MUTATION == 1
    return values[1];
#else
    unsigned index = n->index;
    return values[index];
#endif
}
unsigned ci_branch_index(_Atomic(CINode *) *publication, const unsigned values[4]) {
    CINode *n = atomic_load_explicit(publication, memory_order_consume);
    unsigned index;
    if (n) index = 1; else index = 0;
#if CI_MUTATION == 2
    index = n ? 0 : 1;
#endif
    return values[index];
}
unsigned ci_killed_index(_Atomic(CINode *) *publication, const unsigned values[4]) {
    CINode *n = atomic_load_explicit(publication, memory_order_consume);
    if (!n) return 0;
    unsigned index = kill_dependency(n->index);
    return values[index];
}
unsigned ci_acquire_branch(_Atomic(CINode *) *publication, const unsigned values[4]) {
    CINode *n = atomic_load_explicit(publication, memory_order_acquire);
    unsigned index;
    if (n) index = 1; else index = 0;
    return values[index];
}
unsigned ci_dependency_graph(unsigned code) {
    unsigned reachable = 1;
    for (unsigned from = 0; from < 3; ++from) {
        for (unsigned to = from + 1; to < 4; ++to) {
            unsigned kind = code % 3;
            code /= 3;
#if CI_MUTATION == 3
            if (kind != 0 && (reachable & (1u << from)))
#else
            if (kind == 1 && (reachable & (1u << from)))
#endif
                reachable |= 1u << to;
        }
    }
    return reachable;
}
