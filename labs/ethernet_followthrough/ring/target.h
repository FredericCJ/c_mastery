#ifndef ETHERNET_RING_TARGET_H
#define ETHERNET_RING_TARGET_H
#include "ring.h"

/* Privileged, exclusive debug-counter and RCC/GPIO/ETH ownership. No active
 * peripheral or DMA users. No board is configured by host validation. */
struct en_target
{
    unsigned opened;
    uint32_t old_demcr, old_dwt;
};

int en_target_open(struct en_target *);
int en_target_cold(struct en_target *);
struct en_bus en_target_bus(struct en_target *);
/* Close only after all consumers and device accesses are quiescent. */
int en_target_close(struct en_target *);
#endif
