#ifndef AMP_FOLLOWTHROUGH_H
#define AMP_FOLLOWTHROUGH_H
#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>
enum { AF_OK,AF_INVALID,AF_FULL,AF_EMPTY,AF_STALE,AF_BUSY,AF_EXHAUSTED,AF_CONFLICT,AF_UNIMPLEMENTED=99 };
enum { AF_CAPACITY=16,AF_PAYLOAD=32,AF_LAYOUT=0x41463220u };
/* Fixed prefix may be inspected before the payload layout is accepted.
 * Initial construction/reset requires explicit peer AND DMA quiescence.
 * One supervisor writes ready/layout/epoch; peer alone writes ack. */
typedef struct { _Atomic uint32_t ready,version,layout,epoch,ack,parked; } AfBoot;
typedef struct { uint32_t next_epoch,stage,start,timeout;unsigned active; } AfBootOwner;
int af_boot_begin(AfBootOwner *,uint32_t,uint32_t,uint32_t,unsigned,unsigned);
int af_boot_stage(AfBootOwner *,AfBoot *,unsigned,uint32_t,uint32_t);
int af_boot_ack(AfBoot *,uint32_t,uint32_t,uint32_t);
int af_boot_close(AfBootOwner *,AfBoot *,unsigned,unsigned);
int af_boot_status(const AfBootOwner *,const AfBoot *,uint32_t);
int af_elapsed(uint32_t,uint32_t,uint32_t,unsigned *);
typedef struct { uint32_t epoch,sequence,length;unsigned char bytes[AF_PAYLOAD]; } AfMessage;
typedef struct { _Alignas(64) _Atomic uint32_t head;_Alignas(64) _Atomic uint32_t tail;AfMessage slots[AF_CAPACITY]; } AfRing;
/* Exactly one producer and one consumer. No object copy/reset while active.
 * Caller input/output/control remain live, disjoint and exclusively owned. */
void af_ring_init(AfRing *,uint32_t);
int af_batch(AfRing *,uint32_t,const AfMessage *,unsigned);
int af_receive(AfRing *,uint32_t,void *,size_t,uint32_t *,size_t *);
uint32_t af_queued(const AfRing *);
int af_cache_span(uint32_t,uint32_t,uint32_t,uint32_t *,uint32_t *);
typedef struct { uint32_t session,last_id;int32_t total,last_delta,last_result; } AfReceipt;
/* Serialized latest-only receipt in survivor-owned memory; peer reset can
 * preserve it, full-machine reset does not imply persistent retention. */
int af_effect(AfReceipt *,uint32_t,uint32_t,int32_t,int32_t *);
typedef struct { uint32_t base,bytes;unsigned masters,domain; } AfBank;
int af_region(const AfBank *,size_t,uint32_t,uint32_t,unsigned,unsigned *);
typedef struct { unsigned clock_owner,dma_owner,telemetry_owner,compute_owner; } AfOwners;
int af_owner_claim(AfOwners *,unsigned,unsigned);
#endif
