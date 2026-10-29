/* Deliberate negative target fixture: inspect imports; do not link libatomic
 * to make an unbounded/unsupported ISR dependency disappear. */
#include <stdatomic.h>
#include <stdint.h>
static _Atomic uint64_t head, tail;
static uint32_t slots[4];

int xi_wide_queue_push(uint32_t value)
{
    uint64_t h = atomic_load_explicit(&head, memory_order_relaxed);
    uint64_t t = atomic_load_explicit(&tail, memory_order_acquire);
    if (((h + 1) & 3) == t)
    {
        return 0;
    }
    slots[h] = value;
    atomic_store_explicit(&head, (h + 1) & 3, memory_order_release);
    return 1;
}

int xi_wide_queue_pop(uint32_t *value)
{
    uint64_t t = atomic_load_explicit(&tail, memory_order_relaxed);
    uint64_t h = atomic_load_explicit(&head, memory_order_acquire);
    if (t == h)
    {
        return 0;
    }
    *value = slots[t];
    atomic_store_explicit(&tail, (t + 1) & 3, memory_order_release);
    return 1;
}
