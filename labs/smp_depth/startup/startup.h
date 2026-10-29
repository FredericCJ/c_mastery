#ifndef CMASTERY_SMP_DEPTH_STARTUP_H
#define CMASTERY_SMP_DEPTH_STARTUP_H
#include <stdint.h>
enum ss_status {SS_OK,SS_INVALID,SS_BUSY,SS_STALE,SS_DUPLICATE,SS_FULL,SS_LATE,SS_NOT_QUIESCENT};
enum ss_phase {SS_EMPTY,SS_PENDING,SS_TIMED_OUT,SS_READY,SS_LATE_READY,SS_QUIESCENT};
enum {SS_VERIFIED=15,SS_SLOTS=3};
struct ss_ticket {unsigned slot;uint32_t generation;};
struct ss_slot {uint32_t generation;unsigned phase,index,apic;uint64_t page,stack_low,stack_high;};
struct ss_world {unsigned parallel;struct ss_slot slot[SS_SLOTS];};
struct ss_evidence {unsigned apic,verified;uint64_t entry_stack;};
/* Externally serialized control API; generation exhaustion never wraps.
 * Initial world must be zeroed, with parallel0/1. Failure preserves outputs. */
int ss_begin(struct ss_world *,unsigned slot,unsigned index,unsigned apic,uint64_t page,uint64_t stack_low,uint64_t stack_high,struct ss_ticket *);
int ss_timeout(struct ss_world *,struct ss_ticket);
int ss_ack(struct ss_world *,struct ss_ticket,struct ss_evidence);
/* Port attestation: caller has separately established no execution/access to
 * this attempt's page/mailbox/stack can remain. A timeout is NOT attestation. */
int ss_quiescent(struct ss_world *,struct ss_ticket,int port_attested);
int ss_release(struct ss_world *,struct ss_ticket);
#endif
