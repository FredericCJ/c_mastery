#include "checkpoint_channel.h"
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
void cpw_channel_init(struct cpw_channel *c)
{
    cpm_pool_init(&c->pool);
    cpm_ring_init(&c->queue);
    c->has_active = 0;
    c->byte = 0;
    c->active = (struct cpm_handle){0, 0};
    (void)cpw_begin(&c->transfer, 0, 0, 0);
}

enum cpm_status cpw_offer(struct cpw_channel *c, const int *values, size_t n)
{
    if (!c || (n && !values))
        return CPM_INVALID;
    for (size_t i = 0; i < n; ++i)
        if (values[i] < 0 || values[i] > 255)
            return CPM_INVALID;
    return cpm_atomic(&c->pool, &c->queue, values, n);
}

enum cpw_status cpw_channel_step(struct cpw_channel *c, cpw_write_fn writer,
                                 void *context)
{
    if (!c || !writer)
        return CPW_INVALID;
    if (!c->has_active)
    {
        if (!c->queue.count)
            return CPW_DONE;
        if (cpm_pop(&c->queue, &c->active) != CPM_OK)
            return CPW_INVALID;
        const struct cpm_record *record;
        if (cpm_lookup(&c->pool, c->active, &record) != CPM_OK)
            return CPW_INVALID;
        c->byte = (unsigned char)record->value;
        c->has_active = 1;
        (void)cpw_begin(&c->transfer, &c->byte, 1, 42);
    }
    enum cpw_status status = cpw_step(&c->transfer, writer, context);
    if (status == CPW_DONE)
    {
        if (cpm_release(&c->pool, c->active) != CPM_OK)
            return CPW_INVALID;
        c->has_active = 0;
        if (CP_MUTANT == 21)
        {
            int value = c->byte;
            (void)cpm_atomic(&c->pool, &c->queue, &value, 1);
        }
    }
    return status;
}
