/*
 * Reference logger: one non-blocking attempt to own a one-slot log.
 *
 * Only ISO C atomics are used; there is no retry loop, so the critical region
 * is the bounded copy below. Whether the generated flag operation is wait-free
 * on a given target is a separate object-code question (see ANALYSIS.md).
 */
#include "logger.h"

int ef_log(struct ef_log *target, const unsigned char *data, size_t length)
{
    if (!target || (!data && length) || length > EF_LOG_BYTES)
    {
        return -1;
    }
    /* Acquire pairs with the release below, so a reader that wins the flag
     * next sees the bytes and the length this call published. */
    if (atomic_flag_test_and_set_explicit(&target->owned, memory_order_acquire))
    {
        return 0;
    }
    for (size_t i = 0; i < length; ++i)
    {
        target->bytes[i] = data[i];
    }
    target->length = length;
    atomic_flag_clear_explicit(&target->owned, memory_order_release);
    return 1;
}
