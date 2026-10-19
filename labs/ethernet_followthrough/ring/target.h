#ifndef ETHERNET_RING_TARGET_H
#define ETHERNET_RING_TARGET_H
#include "ring.h"
/* Privileged, exclusive debug-counter and RCC/GPIO/ETH ownership. No active
 * peripheral or DMA users. No board is configured by host validation. */
typedef struct {unsigned opened;uint32_t old_demcr,old_dwt;} EnTarget;
int en_target_open(EnTarget *);
int en_target_cold(EnTarget *);
EnBus en_target_bus(EnTarget *);
/* Close only after all consumers and device accesses are quiescent. */
int en_target_close(EnTarget *);
#endif
