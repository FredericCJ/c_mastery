#include "transaction.h"

int bf_construct(unsigned s, struct b2_backend b, unsigned *p)
{
    (void)s;
    (void)b;
    (void)p;
    return B2_FAILED;
}

void bf_tx_init(struct bf_tx *t)
{
    *t = (struct bf_tx){0};
    t->stage = 4;
}

int bf_tx_begin(struct bf_tx *t, struct bf_tx_backend b, unsigned *p)
{
    (void)t;
    (void)b;
    (void)p;
    return B2_FAILED;
}

int bf_tx_close(struct bf_tx *t)
{
    (void)t;
    return B2_FAILED;
}
