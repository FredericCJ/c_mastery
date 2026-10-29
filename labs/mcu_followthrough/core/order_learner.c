#include "order.h"

void mf_order_init(struct mf_order *q, uint32_t epoch, unsigned heap,
                   unsigned reverse)
{
    (void)q;
    (void)epoch;
    (void)heap;
    (void)reverse;
}

int mf_order_push(struct mf_order *q, struct mf_order_entry e)
{
    (void)q;
    (void)e;
    return 0;
}

int mf_order_pop(struct mf_order *q, struct mf_order_entry *e)
{
    (void)q;
    (void)e;
    return 0;
}
