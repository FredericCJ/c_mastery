#ifndef CMASTERY_SEMAPHORE_IRQ_H
#define CMASTERY_SEMAPHORE_IRQ_H
#include "semaphore.h"
#define CDS_KERNEL_THRESHOLD 0x80u
uint32_t cds_irq_save(void);
void cds_irq_restore(uint32_t);
int cds_irq_post(struct cd_sem *, struct cd_post *);
int cds_high_offer(struct cd_deferred *);
#endif
