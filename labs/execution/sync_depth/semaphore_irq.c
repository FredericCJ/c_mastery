#include "semaphore_irq.h"

uint32_t cds_irq_save(void)
{
    return 0;
}

void cds_irq_restore(uint32_t saved)
{
    (void)saved;
}

int cds_irq_post(struct cd_sem *s, struct cd_post *out)
{
    (void)s;
    (void)out;
    return CMX_UNIMPLEMENTED;
}

int cds_high_offer(struct cd_deferred *d)
{
    (void)d;
    return CMX_UNIMPLEMENTED;
}
