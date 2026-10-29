#include "checkpoint_channel.h"

void cpw_channel_init(struct cpw_channel *c)
{
    (void)c;
}

enum cpm_status cpw_offer(struct cpw_channel *c, const int *v, size_t n)
{
    (void)c;
    (void)v;
    (void)n;
    return CPM_INVALID;
}

enum cpw_status cpw_channel_step(struct cpw_channel *c, cpw_write_fn w, void *p)
{
    (void)c;
    (void)w;
    (void)p;
    return CPW_INVALID;
}
