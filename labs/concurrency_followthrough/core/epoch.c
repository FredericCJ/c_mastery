#include "epoch.h"

void cf_epoch_init(struct cf_epoch *e, unsigned n)
{
    (void)e;
    (void)n;
}

int cf_epoch_register(struct cf_epoch *e, struct ag_lease *l)
{
    (void)e;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int cf_epoch_enter(struct cf_epoch *e, struct ag_lease l)
{
    (void)e;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int cf_epoch_leave(struct cf_epoch *e, struct ag_lease l)
{
    (void)e;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int cf_epoch_depart(struct cf_epoch *e, struct ag_lease l)
{
    (void)e;
    (void)l;
    return AG_UNIMPLEMENTED;
}

int cf_epoch_advance(struct cf_epoch *e)
{
    (void)e;
    return AG_UNIMPLEMENTED;
}

int cf_epoch_eligible(struct cf_epoch *e, unsigned r, unsigned *o)
{
    (void)e;
    (void)r;
    (void)o;
    return AG_UNIMPLEMENTED;
}
