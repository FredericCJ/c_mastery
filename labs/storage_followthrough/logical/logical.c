#include "logical.h"

int sl_format(store_io i)
{
    (void)i;
    return SL_UNIMPLEMENTED;
}

int sl_open(struct sl_counter *s, store_io i)
{
    (void)s;
    (void)i;
    return SL_UNIMPLEMENTED;
}

int sl_apply(struct sl_counter *s, uint64_t id, int64_t d, struct sl_receipt *r)
{
    (void)s;
    (void)id;
    (void)d;
    (void)r;
    return SL_UNIMPLEMENTED;
}

int sl_get(const struct sl_counter *s, struct sl_receipt *r)
{
    (void)s;
    (void)r;
    return SL_UNIMPLEMENTED;
}

int sl_checkpoint(struct sl_counter *s)
{
    (void)s;
    return SL_UNIMPLEMENTED;
}
