/* Compile/analysis-only: never invoke two unordered race_worker calls.
 * If two threads access the same live int through this function, neither
 * happens before the other and accesses conflict, the program has a C data
 * race. A pipeline RAW dependence does not supply thread synchronization. */
#include <stdatomic.h>

void race_worker(int *counter)
{
    *counter = *counter + 1;
}

/* Atomicity repairs the conflicting accesses for this counter operation.
 * It does not publish unrelated data or provide a general ordering guarantee. */
void atomic_counter_worker(_Atomic unsigned *counter)
{
    atomic_fetch_add_explicit(counter, 1u, memory_order_relaxed);
}
