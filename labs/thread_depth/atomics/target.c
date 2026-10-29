#include "atomics.h"

unsigned target_counter(struct ta_counter *p)
{
    return ta_counter_add(p, 1);
}

unsigned target_native(_Atomic(unsigned) *p)
{
    return atomic_fetch_add_explicit(p, 1, memory_order_relaxed);
}

unsigned long long target_wide(_Atomic(unsigned long long) *p)
{
    return atomic_fetch_add_explicit(p, 1, memory_order_relaxed);
}

struct ta_pair target_pair(_Atomic(struct ta_pair) *p)
{
    return atomic_load_explicit(p, memory_order_relaxed);
}

unsigned target_consume(_Atomic(struct ta_node *) *p)
{
    struct ta_node *n = atomic_load_explicit(p, memory_order_consume);
    return n->value;
}

unsigned target_acquire(_Atomic(struct ta_node *) *p)
{
    struct ta_node *n = atomic_load_explicit(p, memory_order_acquire);
    return n->value;
}

unsigned target_control(_Atomic(struct ta_node *) *p,
                        struct ta_node *independent)
{
    struct ta_node *n = atomic_load_explicit(p, memory_order_consume);
    return n ? independent->value : 0;
}

void target_release_fence(void)
{
    atomic_thread_fence(memory_order_release);
}

void target_signal_fence(void)
{
    atomic_signal_fence(memory_order_release);
}
