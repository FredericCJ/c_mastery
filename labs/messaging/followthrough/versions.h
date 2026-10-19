#ifndef CMASTERY_MESSAGING_VERSIONS_H
#define CMASTERY_MESSAGING_VERSIONS_H
#include "lifetime.h"
typedef MsgStatus (*MfvClone)(void *,const void *,MfoValue *);
typedef struct {
    pthread_mutex_t mutex;
    MsgVersions core;
    MflShared *payloads[8];
    MflAllocator allocator;
} MfvStore;
MsgStatus mfv_init(MfvStore *,MflAllocator,MfoValue *);
MsgStatus mfv_snapshot(MfvStore *,MsgSnapshot *);
MsgStatus mfv_read(MfvStore *,MsgSnapshot,MflShared **);
MsgStatus mfv_drop_snapshot(MfvStore *,MsgSnapshot);
MsgStatus mfv_publish(MfvStore *,MfvClone,void *,const void *);
MsgStatus mfv_destroy(MfvStore *);

typedef struct {pthread_mutex_t mutex;MsgStore core;} MfkStore;
typedef struct {MfkStore *owner;MsgTxn core;unsigned snapshot[2],observed;} MfkTxn;
MsgStatus mfk_init(MfkStore *,unsigned,unsigned);
MsgStatus mfk_begin(MfkStore *,MfkTxn *);
MsgStatus mfk_read(MfkTxn *,unsigned,unsigned *);
MsgStatus mfk_nonzero(MfkTxn *,unsigned *);
MsgStatus mfk_write(MfkTxn *,unsigned,unsigned);
MsgStatus mfk_commit(MfkTxn *,bool);
void mfk_destroy(MfkStore *);
#endif
