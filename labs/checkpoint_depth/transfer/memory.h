#ifndef CHECKPOINT_TRANSFER_MEMORY_H
#define CHECKPOINT_TRANSFER_MEMORY_H
#include <stddef.h>
#include <stdint.h>
#include "checkpoint_memory.h"
enum { CPX_LIMIT=4, CPX_NONE=4, CPX_GENERATION_MAX=3 };
typedef enum { CPX_OK,CPX_FULL,CPX_INVALID,CPX_BUSY,CPX_PENDING,CPX_FAILED,CPX_NOT_IMPLEMENTED } CPXStatus;
typedef struct { int value;size_t length;unsigned char bytes[16]; } CPXRecord;
typedef struct { unsigned slot,generation; } CPXHandle;
typedef struct { CPXHandle handle;unsigned next,used; } CPXNode;
typedef struct {
 void *context;
 void *(*allocate)(void *,size_t);
 void (*release)(void *,void *);
 int (*subscribe)(void *,uint64_t *);
 int (*unsubscribe)(void *,uint64_t);
} CPXBackend;
typedef struct {
 CPXBackend backend;CPXRecord *records;CPXNode *nodes;
 unsigned capacity,node_capacity,state[CPX_LIMIT],generation[CPX_LIMIT],pins[CPX_LIMIT];
 unsigned head,tail,count,node_attempts,node_fail_at,closing;
 uint64_t cookie;
} CPXStore;
CPXStatus cpx_init(CPXStore *,CPXBackend,unsigned,unsigned);
CPXStatus cpx_close(CPXStore *);
CPXStatus cpx_acknowledge(CPXStore *,uint64_t,int);
CPXStatus cpx_allocate(CPXStore *,const CPXRecord *,CPXHandle *);
CPXStatus cpx_lookup(CPXStore *,CPXHandle,const CPXRecord **);
CPXStatus cpx_pin(CPXStore *,CPXHandle);
CPXStatus cpx_unpin(CPXStore *,CPXHandle);
CPXStatus cpx_release(CPXStore *,CPXHandle);
CPXStatus cpx_relocate(CPXStore *);
CPXStatus cpx_grow(CPXStore *,unsigned);
CPXStatus cpx_push(CPXStore *,CPXHandle);
CPXStatus cpx_pop(CPXStore *,CPXHandle *);
CPXStatus cpx_batch(CPXStore *,const CPXRecord *,size_t,size_t *);
typedef struct { CPMArena *arena;uint64_t cookie;unsigned pending,cancelled; } CPXLease;
CPXStatus cpx_lease_begin(CPXLease *,CPMArena *,CPMSpan,uint64_t);
CPXStatus cpx_lease_cancel(CPXLease *);
CPXStatus cpx_lease_complete(CPXLease *,uint64_t,int);
#endif
