#ifndef CMASTERY_SMP_DEPTH_STARTUP_H
#define CMASTERY_SMP_DEPTH_STARTUP_H
#include <stdint.h>
enum SsStatus {SS_OK,SS_INVALID,SS_BUSY,SS_STALE,SS_DUPLICATE,SS_FULL,SS_LATE,SS_NOT_QUIESCENT};
enum SsPhase {SS_EMPTY,SS_PENDING,SS_TIMED_OUT,SS_READY,SS_LATE_READY,SS_QUIESCENT};
enum {SS_VERIFIED=15,SS_SLOTS=3};
typedef struct {unsigned slot;uint32_t generation;} SsTicket;
typedef struct {uint32_t generation;unsigned phase,index,apic;uint64_t page,stack_low,stack_high;} SsSlot;
typedef struct {unsigned parallel;SsSlot slot[SS_SLOTS];} SsWorld;
typedef struct {unsigned apic,verified;uint64_t entry_stack;} SsEvidence;
/* Externally serialized control API; generation exhaustion never wraps.
 * Initial world must be zeroed, with parallel0/1. Failure preserves outputs. */
int ss_begin(SsWorld *,unsigned slot,unsigned index,unsigned apic,uint64_t page,uint64_t stack_low,uint64_t stack_high,SsTicket *);
int ss_timeout(SsWorld *,SsTicket);
int ss_ack(SsWorld *,SsTicket,SsEvidence);
/* Port attestation: caller has separately established no execution/access to
 * this attempt's page/mailbox/stack can remain. A timeout is NOT attestation. */
int ss_quiescent(SsWorld *,SsTicket,int port_attested);
int ss_release(SsWorld *,SsTicket);
#endif
