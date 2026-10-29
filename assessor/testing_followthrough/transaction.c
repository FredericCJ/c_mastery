#include "transaction.h"
#ifndef TX_MUTANT
#define TX_MUTANT 0
#endif
void tx_destroy(struct cv_tx *t, struct tx_owner *out)
{
    while (out->count)
        t->release(t->context, out->ids[--out->count]);
}

struct tx_result tx_run(struct cv_tx *t, unsigned fourth,
                        const unsigned char *p, size_t n, struct tx_owner *out)
{
    struct tx_owner staged = {{0}, 0};
    struct tx_result r = {TX_OK, 0};
    const unsigned ids[4] = {10, 15, 20, 30};
    if (!t || !out || !t->acquire || !t->release || !t->write ||
        !t->cancelled || fourth > 1 || n > 16 || (!p && n))
        return (struct tx_result){TX_INVALID, 0};
    if (t->busy)
        return (struct tx_result){TX_BUSY, 0};
    t->busy = 1;
    for (size_t i = 0; i < 4; i++)
    {
        if (!fourth && i == 1)
            continue;
        if (!t->acquire(t->context, ids[i]))
        {
            r.status = TX_ACQUIRE;
            goto failed;
        }
        staged.ids[staged.count++] = ids[i];
    }
    while (r.committed < n)
    {
        if (t->cancelled(t->context))
        {
            r.status = TX_CANCEL;
            goto failed;
        }
        size_t remaining = n - r.committed;
        long progress = t->write(t->context, p + r.committed, remaining);
        if (progress == -1)
        {
            r.status = TX_TIMEOUT;
            goto failed;
        }
        if (progress == 0)
        {
            r.status = TX_ZERO;
            goto failed;
        }
        if (progress < 0 || (unsigned long)progress > remaining)
        {
            r.status = TX_INVALID;
            goto failed;
        }
        r.committed += TX_MUTANT == 2 ? remaining : (size_t)progress;
    }
    *out = staged;
    t->busy = 0;
    return r;
failed:
    if (TX_MUTANT == 3)
        *out = staged;
    if (TX_MUTANT == 1 && staged.count)
        staged.count--;
    tx_destroy(t, &staged);
    t->busy = 0;
    return r;
}
