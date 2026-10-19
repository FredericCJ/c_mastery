#ifndef CMASTERY_MESSAGING_LIFETIME_H
#define CMASTERY_MESSAGING_LIFETIME_H
#include "messaging.h"
#include "owned_value.h"

typedef struct {
    void *context;
    void *(*allocate)(void *,size_t);
    void (*deallocate)(void *,void *);
} MflAllocator;
typedef struct MflShared MflShared;
MsgStatus mfl_make(MflAllocator,MfoValue *,MflShared **);
MsgStatus mfl_retain(MflShared *);
void mfl_drop(MflShared **);
const MfoValue *mfl_borrow(const MflShared *);

typedef struct MfgArray MfgArray;
typedef struct {
    pthread_mutex_t mutex;
    MflAllocator allocator;
    MfgArray *array;
    size_t top,bottom,capacity,maximum;
    unsigned active;
    bool live;
} MfgDeque;
typedef void (*MfgPause)(void *);
MsgStatus mfg_init(MfgDeque *,MflAllocator,size_t);
MsgStatus mfg_push(MfgDeque *,MfoValue *);
MsgStatus mfg_pop(MfgDeque *,MflShared **);
MsgStatus mfg_steal(MfgDeque *,MflShared **,MfgPause,void *);
MsgStatus mfg_destroy(MfgDeque *);

typedef struct {
    pthread_mutex_t mutex;
    MflShared *current;
    uint64_t sequence;
    bool live;
} MfpPointer;
MsgStatus mfp_init(MfpPointer *,MflShared **);
MsgStatus mfp_snapshot(MfpPointer *,MflShared **);
MsgStatus mfp_replace(MfpPointer *,MflShared **);
MsgStatus mfp_destroy(MfpPointer *);
#endif
