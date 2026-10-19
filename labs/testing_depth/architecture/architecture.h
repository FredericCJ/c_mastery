#ifndef TESTING_ARCHITECTURE_H
#define TESTING_ARCHITECTURE_H
#include <stddef.h>
#include <stdint.h>
/* Caller-owned typed objects, disjoint and live; serialized calls only.
   Read CONTRACTS.md before using these operations. No hidden allocation. */
enum { TA_MAX=8, TA_NONE=8 };
typedef enum { TA_OK, TA_FULL, TA_EMPTY, TA_INVALID, TA_AGAIN,
               TA_TIMEOUT, TA_IO, TA_DONE, TA_CANCELLED } TaStatus;
typedef enum { TA_RING, TA_COMPACT, TA_LINKED, TA_COUNTED } TaLayout;
typedef struct { unsigned char value; unsigned next; } TaNode;
typedef struct { unsigned char bytes[TA_MAX+1]; TaNode nodes[TA_MAX]; } TaStorage;
typedef struct {
 TaStorage *storage; TaLayout layout;
 unsigned capacity,extent,head,tail,count,free_head;
} TaQueue;
TaStatus ta_init(TaQueue *,TaStorage *,TaLayout,unsigned capacity,unsigned extent);
TaStatus ta_push(TaQueue *,unsigned char);
TaStatus ta_pop(TaQueue *,unsigned char *);
TaStatus ta_peek(const TaQueue *,unsigned char *);
size_t ta_count(const TaQueue *);
/* Batch: accepted prefix is committed; *accepted is defined on every result. */
TaStatus ta_batch(TaQueue *,const unsigned char *,size_t,size_t *accepted);
/* Separate representation-specific diagnostic, not the behavioral oracle. */
int ta_invariant(const TaQueue *);
/* Original packet: exactly {2,a,b}; staged decoder output on valid input. */
TaStatus ta_decode(const unsigned char *,size_t,unsigned char out[2]);
typedef enum { TA_WRITE_OK, TA_WRITE_WAIT, TA_WRITE_ERROR } TaWriteStatus;
typedef struct { TaWriteStatus status; size_t count; } TaWriteResult;
typedef TaWriteResult (*TaWrite)(void *,const unsigned char *,size_t);
typedef struct {
 TaQueue queue; TaStorage storage; unsigned char bytes[2];
 size_t offset; unsigned budget; TaStatus state; int uncertain;
} TaFlow;
/* Fresh destination; admitted begin copies packet and retains no input borrow. */
TaStatus ta_begin(TaFlow *,const unsigned char *,size_t,TaLayout,unsigned budget);
TaStatus ta_step(TaFlow *,TaWrite,void *);
TaStatus ta_cancel(TaFlow *);
#endif
