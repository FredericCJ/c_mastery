#ifndef AF_DOORBELL_H
#define AF_DOORBELL_H
#include "amp.h"
struct af_registers { void *context;uint32_t (*read)(void *,uint32_t);void (*write)(void *,uint32_t,uint32_t);void (*fence)(void *); };
struct af_door { unsigned core,outgoing,incoming; };
/* Caller serializes all local users of outgoing semaphore. Each core has
 * a distinct IRQ bank. A doorbell carries no count and confers no slot lease. */
int af_ring_door(struct af_registers,struct af_door);
int af_mask_door(struct af_registers,struct af_door,unsigned);
int af_clear_door(struct af_registers,struct af_door);
int af_service_door(struct af_registers,struct af_door,struct af_ring *,uint32_t,unsigned,unsigned *,unsigned *);
#endif
