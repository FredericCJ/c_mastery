#ifndef AF_DOORBELL_H
#define AF_DOORBELL_H
#include "amp.h"
typedef struct { void *context;uint32_t (*read)(void *,uint32_t);void (*write)(void *,uint32_t,uint32_t);void (*fence)(void *); } AfRegisters;
typedef struct { unsigned core,outgoing,incoming; } AfDoor;
/* Caller serializes all local users of outgoing semaphore. Each core has
 * a distinct IRQ bank. A doorbell carries no count and confers no slot lease. */
int af_ring_door(AfRegisters,AfDoor);
int af_mask_door(AfRegisters,AfDoor,unsigned);
int af_clear_door(AfRegisters,AfDoor);
int af_service_door(AfRegisters,AfDoor,AfRing *,uint32_t,unsigned,unsigned *,unsigned *);
#endif
