#include "admission.h"

/*
 * Reference admission: the capacity argument may only tighten the physical
 * queue bound CP_QUEUE. A refused frame must stay ready in the parser, so the
 * full-queue branch returns before cp_deliver() can touch the parser state.
 * See admission.h for the contract.
 */
enum cp_status cpa_admit(struct cp_parser *p, struct cp_queue *q,
                         size_t capacity)
{
    if (p == NULL || q == NULL || capacity == 0 || capacity > CP_QUEUE ||
        q->count > capacity)
    {
        return CP_INVALID;
    }
    if (!p->ready)
    {
        return CP_NEED_MORE;
    }
    if (q->count == capacity)
    {
#ifdef CPA_MUTANT_DROP
        p->ready = 0;
#endif
        return CP_FULL;
    }
    return cp_deliver(p, q);
}
