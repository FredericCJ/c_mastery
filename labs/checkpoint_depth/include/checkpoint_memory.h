#ifndef CHECKPOINT_MEMORY_H
#define CHECKPOINT_MEMORY_H
#include <stddef.h>
enum { CPM_SLOTS=4, CPM_GENERATION_MAX=3 };
typedef enum { CPM_OK, CPM_FULL, CPM_BUSY, CPM_INVALID } CPMStatus;
typedef struct { size_t offset,length; unsigned generation; } CPMSpan;
typedef struct { unsigned char *bytes; size_t capacity,used,alignment; unsigned generation,pins; } CPMArena;
typedef struct { int value; } CPMRecord;
typedef struct { unsigned slot,generation; } CPMHandle;
typedef struct {
    CPMRecord objects[CPM_SLOTS];
    unsigned state[CPM_SLOTS],generation[CPM_SLOTS],pins[CPM_SLOTS];
    unsigned attempts,fail_at;
} CPMPool;
typedef struct { CPMHandle items[CPM_SLOTS]; size_t head,count; } CPMRing;
CPMStatus cpm_arena_init(CPMArena *,unsigned char *,size_t,size_t,size_t);
CPMStatus cpm_reserve(CPMArena *,size_t,CPMSpan *);
CPMStatus cpm_view(const CPMArena *,CPMSpan,const unsigned char **);
CPMStatus cpm_pin(CPMArena *,CPMSpan);
CPMStatus cpm_unpin(CPMArena *);
CPMStatus cpm_reset(CPMArena *);
void cpm_pool_init(CPMPool *);
void cpm_ring_init(CPMRing *);
void cpm_fail_at(CPMPool *,unsigned);
CPMStatus cpm_allocate(CPMPool *,int,CPMHandle *);
CPMStatus cpm_lookup(const CPMPool *,CPMHandle,const CPMRecord **);
CPMStatus cpm_pool_pin(CPMPool *,CPMHandle);
CPMStatus cpm_pool_unpin(CPMPool *,CPMHandle);
CPMStatus cpm_release(CPMPool *,CPMHandle);
CPMStatus cpm_prefix(CPMPool *,CPMRing *,const int *,size_t,size_t *);
CPMStatus cpm_atomic(CPMPool *,CPMRing *,const int *,size_t);
CPMStatus cpm_pop(CPMRing *,CPMHandle *);
#endif
