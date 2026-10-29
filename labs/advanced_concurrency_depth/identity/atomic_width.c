#include "identity.h"

int ai_pair_cas(_Atomic(struct ai_pair) *p, struct ai_pair *expected,
                struct ai_pair desired)
{
    return atomic_compare_exchange_strong(p, expected, desired);
}

int ai_pair_lock_free(_Atomic(struct ai_pair) *p)
{
    return atomic_is_lock_free(p);
}
