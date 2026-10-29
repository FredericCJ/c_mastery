#include "epoch.h"

static int acquire(struct cf_epoch *e)
{
    return !atomic_flag_test_and_set_explicit(&e->gate, memory_order_acquire);
}

static void release(struct cf_epoch *e)
{
    atomic_flag_clear_explicit(&e->gate, memory_order_release);
}

void cf_epoch_init(struct cf_epoch *e, unsigned nested)
{
    atomic_flag_clear(&e->gate);
    ag_init(&e->domain, nested);
}

int cf_epoch_register(struct cf_epoch *e, struct ag_lease *l)
{
    if (!e || !l)
        return AG_INVALID;
    if (!acquire(e))
        return AG_BUSY;
    int r = ag_register(&e->domain, l);
    release(e);
    return r;
}

int cf_epoch_enter(struct cf_epoch *e, struct ag_lease l)
{
    if (!e)
        return AG_INVALID;
    if (!acquire(e))
        return AG_BUSY;
    int r = ag_enter(&e->domain, l);
    release(e);
    return r;
}

int cf_epoch_leave(struct cf_epoch *e, struct ag_lease l)
{
    if (!e)
        return AG_INVALID;
    if (!acquire(e))
        return AG_BUSY;
    int r = ag_leave(&e->domain, l);
    release(e);
    return r;
}

int cf_epoch_depart(struct cf_epoch *e, struct ag_lease l)
{
    if (!e)
        return AG_INVALID;
    if (!acquire(e))
        return AG_BUSY;
    int r = ag_depart(&e->domain, l);
    release(e);
    return r;
}

int cf_epoch_advance(struct cf_epoch *e)
{
    if (!e)
        return AG_INVALID;
    if (!acquire(e))
        return AG_BUSY;
    int r = ag_advance(&e->domain);
    release(e);
    return r;
}

int cf_epoch_eligible(struct cf_epoch *e, unsigned retired, unsigned *eligible)
{
    if (!e || !eligible)
        return AG_INVALID;
    if (!acquire(e))
        return AG_BUSY;
    *eligible = (unsigned)ag_reclaimable(&e->domain, retired);
    release(e);
    return AG_OK;
}
