#ifndef CMASTERY_MESSAGING_STREAM_FOLLOW_H
#define CMASTERY_MESSAGING_STREAM_FOLLOW_H
#include "messaging.h"
#include "owned_value.h"
typedef enum {MFQ_WAIT,MFQ_DROP,MFQ_FAIL} MfqPolicy;
typedef struct {
    uint64_t accepted,delivered,discarded,rejected;
    unsigned queued,inflight,handles,waiting;
    bool accepting,finished,joined;
} MfqStats;
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    pthread_t worker;
    MsgHub channel;
    MsgStream core;
    MfoValue context;
    MsgNext next;
    MsgTerminal terminal;
    uint64_t requests;
    bool cancel_pending,invalid_pending,end_pending,idle;
    MsgStatus end_status;
    unsigned capacity;
    MfqStats stats;
} MfqStream;
typedef struct {MfqStream *owner;} MfqHandle;
MsgStatus mfq_init(MfqStream *,unsigned,MfoValue *,MsgNext,MsgTerminal,bool);
MsgStatus mfq_acquire(MfqStream *,MfqHandle *);
void mfq_release(MfqHandle *);
MsgStatus mfq_request(MfqHandle *,uint64_t);
MsgStatus mfq_submit(MfqHandle *,unsigned,MfqPolicy);
MsgStatus mfq_end(MfqHandle *,MsgStatus);
MsgStatus mfq_cancel(MfqHandle *);
MsgStatus mfq_close(MfqStream *);
MsgStatus mfq_join(MfqStream *);
MsgStatus mfq_idle(MfqStream *);
MsgStatus mfq_stats(MfqStream *,MfqStats *);
MsgStatus mfq_destroy(MfqStream *);
#endif
