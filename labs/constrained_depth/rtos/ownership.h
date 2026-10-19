#ifndef CMASTERY_CONSTRAINED_RTOS_OWNERSHIP_H
#define CMASTERY_CONSTRAINED_RTOS_OWNERSHIP_H
#include <stddef.h>
#include <stdint.h>
enum { CR_OK,CR_INVALID,CR_FULL,CR_STALE,CR_BUSY,CR_RANGE,CR_UNKNOWN };
enum { CR_FREE,CR_OWNED,CR_OFFERED,CR_CONSUMER,CR_RETIRED };
typedef struct {uint64_t world,generation;unsigned slot;} CrToken;
typedef struct {unsigned char bytes[16];size_t length;uint64_t generation;unsigned state;} CrPayload;
typedef struct {CrPayload slots[3];uint64_t world;unsigned closed;} CrPool;
/* Serialize each metadata transition; public objects must not be forged.
 * Init requires whole-world quiescence and a never-reused positive world.
 * Slots0/1 ordinary, slot2 fault-only; payload is copied before OWNED publication.
 * OFFERED covers a possibly blocked queue send. A consumer can claim it before
 * the sender returns. After successful send the sender must not touch it again.
 * Failed send has published no item and authorizes abort then owner release. */
int cr_pool_init(CrPool *,uint64_t);
int cr_allocate(CrPool *,int,const unsigned char *,size_t,CrToken *);
int cr_offer(CrPool *,CrToken);
int cr_abort_offer(CrPool *,CrToken);
int cr_claim(CrPool *,CrToken);
int cr_read(const CrPool *,CrToken,unsigned char *,size_t,size_t *);
int cr_release(CrPool *,CrToken);
int cr_close(CrPool *);
int cr_drained(const CrPool *);
/* Inputs are actual byte reservations. Unknown bound is not zero. */
int cr_budget(const uint64_t *,size_t,int,uint64_t,uint64_t *);
#endif
