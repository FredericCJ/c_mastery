#ifndef CMASTERY_MESSAGING_ACTOR_EXECUTOR_H
#define CMASTERY_MESSAGING_ACTOR_EXECUTOR_H
#include "owned.h"
typedef struct MfeExecutor MfeExecutor;
typedef struct {
    MfeExecutor *owner;MfoValue value;uint64_t generation;unsigned destination;bool used;
} MfeEnvelope;
struct MfeExecutor {
    MfcHub input;MfaActor actor[2];pthread_mutex_t pool;
    MfeEnvelope envelopes[8];uint64_t published[2];
};
typedef struct {unsigned destination;uint64_t generation;MsgStatus delivery;} MfeEvent;
MsgStatus mfe_init(MfeExecutor *,size_t,size_t,MfaHandler,void *[2],MfaDeadLetter,void *);
MsgStatus mfe_send(MfeExecutor *,unsigned,uint64_t,MfoValue *,bool);
MsgStatus mfe_pump(MfeExecutor *,bool,MfeEvent *);
MsgStatus mfe_generation(MfeExecutor *,unsigned,uint64_t *);
MsgStatus mfe_restart(MfeExecutor *,unsigned);
MsgStatus mfe_close(MfeExecutor *,unsigned);
void mfe_cancel(MfeExecutor *);
MsgStatus mfe_destroy(MfeExecutor *);
#endif
