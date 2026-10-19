#ifndef EMBEDDED_DEBUG_RECOVERY_H
#define EMBEDDED_DEBUG_RECOVERY_H
#include <stddef.h>
#include <stdint.h>
#include "lifecycle_extension.h"
enum { DU_OK,DU_INVALID,DU_ABSENT,DU_CONFLICT,DU_STOP,DU_INTERRUPTED,DU_UNIMPLEMENTED };
typedef struct {uint32_t version,extent,base,entry,stack,clock,generation;} DuImage;
typedef struct {unsigned char bytes[64];} DuSlot;
/* All inputs/outputs live and disjoint; serialized exclusive ownership. Payload
   10..24 includes LE32 MSP/entry in first8 bytes. Metadata is integrity only. */
uint32_t du_crc(const unsigned char *,size_t);
int du_encode(const DuImage *,const unsigned char *,size_t,DuSlot *);
int du_decode(const DuSlot *,DuImage *);
int du_select(const DuSlot *,size_t,uint32_t floor,uint32_t maximum,uint32_t clock,unsigned *);
/* Distinct erase sectors required; active slot retained. Simulated erase is
   atomic, then ordered 32-bit programs with arbitrary bit subset in next unit.
   complete=0..16, mask selects programmed bits of next unit. */
int du_install(DuSlot slots[2],unsigned active,const unsigned sector[2],const DuSlot *,unsigned complete,uint32_t mask);
typedef struct {uint32_t clock;unsigned privileged_thread,active_exception,dma_owned,peripheral_owned;} DuMachine;
typedef struct {uint32_t vector,stack,entry;} DuPlan;
/* Image must be a current trusted validated descriptor, not a forge-proof C
   capability. Clock mismatch rejects; actual transition requires a new proof. */
int du_prepare(const DuImage *,const DuMachine *,DuPlan *);
typedef struct {unsigned char records[2][24];} DuRetry;
int du_retry_initialize(DuRetry *); /* Explicit trusted fresh provisioning only. */
int du_retry_state(const DuRetry *,uint32_t limit,uint32_t *consumed);
/* Durable attempt consumption precedes permission; cut0..24 writes that prefix
   into the inactive erased independent record. A failed call grants nothing. */
int du_retry_take(DuRetry *,uint32_t limit,unsigned cut,uint32_t *attempt);
typedef struct {LCEFault record;uint32_t first,secondary;unsigned failed,contained;uint32_t detection_step,containment_step,output,emitted;} DuRecovery;
/* Trusted fresh zero object. Only logging callback may reenter du_fault once;
   nested call stores fallback only and never recurses into callback. */
int du_effect(DuRecovery *,unsigned value);
int du_fault(DuRecovery *,uint32_t reason,uint32_t step,int (*log)(void *,uint32_t),void *);
int du_contain(DuRecovery *,uint32_t step,int (*safe_output)(void *),void *);
/* Status/count pair from a trusted callback, validated before any dereference
   or count-derived address. Status0 progresses;1 timeout;2 device failure. */
int du_recovery_input(DuRecovery *,const unsigned char *,size_t offered,size_t reported,unsigned status,uint32_t step);
#endif
