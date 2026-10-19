#ifndef CMASTERY_MESSAGING_TRANSACTIONS_H
#define CMASTERY_MESSAGING_TRANSACTIONS_H
#include "lifetime.h"
typedef enum {MFT_LOCKED,MFT_CLOCK,MFT_VALIDATED,MFT_PREPUBLISH,MFT_PAYLOAD,MFT_UNLOCKED,MFT_FINISHED} MftPhase;
typedef bool (*MftHook)(void *,MftPhase,unsigned);
MsgStatus mft_commit_phase(MsgTxn *,MftHook,void *);
typedef struct {
    pthread_mutex_t mutex;
    MsgStore core;
    MflAllocator allocator;
    MflShared *nodes[4];
    unsigned log[64],log_count,active;
} MftStore;
typedef struct {
    MftStore *owner;
    MsgTxn core;
    MflShared *reads[4],*writes[4];
    unsigned log[4],log_count;
    bool active;
} MftTxn;
typedef MsgStatus (*MftBody)(void *,MftTxn *);
MsgStatus mft_init(MftStore *,MflAllocator);
MsgStatus mft_begin(MftStore *,MftTxn *);
MsgStatus mft_read(MftTxn *,unsigned,const MfoValue **);
MsgStatus mft_write(MftTxn *,unsigned,MfoValue *);
MsgStatus mft_remove(MftTxn *,unsigned);
MsgStatus mft_log(MftTxn *,unsigned);
void mft_abort(MftTxn *);
MsgStatus mft_commit(MftTxn *,MftHook,void *);
MsgStatus mft_run(MftStore *,unsigned,MftBody,void *,unsigned *);
MsgStatus mft_destroy(MftStore *);
#endif
