#ifndef EMBEDDED_DEBUG_RECOVERY_H
#define EMBEDDED_DEBUG_RECOVERY_H
#include <stddef.h>
#include <stdint.h>
#include "lifecycle_extension.h"
enum { DU_OK,DU_INVALID,DU_ABSENT,DU_CONFLICT,DU_STOP,DU_INTERRUPTED,DU_UNIMPLEMENTED };
struct du_image {uint32_t version,extent,base,entry,stack,clock,generation;};
struct du_slot {unsigned char bytes[64];};
/* All inputs/outputs live and disjoint; serialized exclusive ownership. Payload
   10..24 includes LE32 MSP/entry in first8 bytes. Metadata is integrity only. */
uint32_t du_crc(const unsigned char *,size_t);
int du_encode(const struct du_image *,const unsigned char *,size_t,struct du_slot *);
int du_decode(const struct du_slot *,struct du_image *);
int du_select(const struct du_slot *,size_t,uint32_t floor,uint32_t maximum,uint32_t clock,unsigned *);
/* Distinct erase sectors required; active slot retained. Simulated erase is
   atomic, then ordered 32-bit programs with arbitrary bit subset in next unit.
   complete=0..16, mask selects programmed bits of next unit. */
int du_install(struct du_slot slots[2],unsigned active,const unsigned sector[2],const struct du_slot *,unsigned complete,uint32_t mask);
struct du_machine {uint32_t clock;unsigned privileged_thread,active_exception,dma_owned,peripheral_owned;};
struct du_plan {uint32_t vector,stack,entry;};
/* Image must be a current trusted validated descriptor, not a forge-proof C
   capability. Clock mismatch rejects; actual transition requires a new proof. */
int du_prepare(const struct du_image *,const struct du_machine *,struct du_plan *);
struct du_retry {unsigned char records[2][24];};
int du_retry_initialize(struct du_retry *); /* Explicit trusted fresh provisioning only. */
int du_retry_state(const struct du_retry *,uint32_t limit,uint32_t *consumed);
/* Durable attempt consumption precedes permission; cut0..24 writes that prefix
   into the inactive erased independent record. A failed call grants nothing. */
int du_retry_take(struct du_retry *,uint32_t limit,unsigned cut,uint32_t *attempt);
struct du_recovery {struct lce_fault record;uint32_t first,secondary;unsigned failed,contained;uint32_t detection_step,containment_step,output,emitted;};
/* Trusted fresh zero object. Only logging callback may reenter du_fault once;
   nested call stores fallback only and never recurses into callback. */
int du_effect(struct du_recovery *,unsigned value);
int du_fault(struct du_recovery *,uint32_t reason,uint32_t step,int (*log)(void *,uint32_t),void *);
int du_contain(struct du_recovery *,uint32_t step,int (*safe_output)(void *),void *);
/* Status/count pair from a trusted callback, validated before any dereference
   or count-derived address. Status0 progresses;1 timeout;2 device failure. */
int du_recovery_input(struct du_recovery *,const unsigned char *,size_t offered,size_t reported,unsigned status,uint32_t step);
#endif
