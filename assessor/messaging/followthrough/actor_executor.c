#include "actor_executor.h"
#ifndef MFE_MUTANT
#define MFE_MUTANT 0
#endif
static void envelope_drop(void *p) {
    MfeEnvelope *e=p;MfeExecutor *x=e->owner;
    pthread_mutex_lock(&x->pool);MfoValue value=e->value;e->value=(MfoValue){0};e->used=false;
    pthread_mutex_unlock(&x->pool);mfo_drop(&value);
}
MsgStatus mfe_init(MfeExecutor *x,size_t a,size_t b,MfaHandler handler,void *contexts[2],MfaDeadLetter dead,void *dc) {
    if (!x || !handler || !contexts || !dead || a>4 || b>4) return MSG_INVALID;
    if (pthread_mutex_init(&x->pool,NULL)) return MSG_FAILED;
    MsgStatus s=mfc_init(&x->input,a,b);
    if (s!=MSG_OK) {pthread_mutex_destroy(&x->pool);return s;}
    for(unsigned k=0;k<2;k++) {
        (void)mfa_init(&x->actor[k],handler,contexts[k],dead,dc);x->published[k]=x->actor[k].actor.generation;
    }
    for(unsigned k=0;k<8;k++) x->envelopes[k]=(MfeEnvelope){.owner=x};
    return MSG_OK;
}
MsgStatus mfe_send(MfeExecutor *x,unsigned destination,uint64_t generation,MfoValue *value,bool wait) {
    if (!x || destination>1 || !value || (value->data && !value->drop)) return MSG_INVALID;
    pthread_mutex_lock(&x->pool);unsigned slot=0;
    while(slot<8 && x->envelopes[slot].used) ++slot;
    if(slot==8) {pthread_mutex_unlock(&x->pool);return MSG_FULL;}
    MfeEnvelope *e=&x->envelopes[slot];
    *e=(MfeEnvelope){.owner=x,.value=*value,.generation=generation,.destination=destination,.used=true};
    pthread_mutex_unlock(&x->pool);
    MfoValue envelope={e,envelope_drop};MsgStatus result=mfc_send(&x->input,destination,&envelope,wait,NULL,NULL);
    if (result==MSG_OK) {
        /* A receiver may already have freed/reused e. Do not inspect it. */
        *value=(MfoValue){0};
    } else {
        /* No publication occurred, so this call still exclusively owns e. */
        pthread_mutex_lock(&x->pool);e->value=(MfoValue){0};e->used=false;pthread_mutex_unlock(&x->pool);
    }
    return result;
}
MsgStatus mfe_pump(MfeExecutor *x,bool wait,MfeEvent *out) {
    if (!x || !out) return MSG_INVALID;
    unsigned which;MfoValue envelope={0};MsgStatus status=mfc_select(&x->input,3,wait,&which,&envelope);
    if (status!=MSG_OK) return status;
    MfeEnvelope *e=envelope.data;
    pthread_mutex_lock(&x->pool);MfoValue value=e->value;uint64_t generation=e->generation;
    unsigned destination=e->destination;e->value=(MfoValue){0};pthread_mutex_unlock(&x->pool);
    if (MFE_MUTANT!=101) mfo_drop(&envelope);
    MfaActor *actor=&x->actor[destination];
    if (MFE_MUTANT==102) generation=actor->actor.generation;
    status=mfa_send(actor,generation,&value);
    if (status==MSG_OK) status=mfa_step(actor);
    else if (status!=MSG_STALE) {
        /* The channel accepted this owner. Failed actor admission is now a
           dead-letter transfer, not a return to its former producer. */
        actor->dead(actor->dead_ctx,generation,value);value=(MfoValue){0};
    }
    *out=(MfeEvent){destination,generation,status};(void)which;return MSG_OK;
}
MsgStatus mfe_generation(MfeExecutor *x,unsigned which,uint64_t *generation) {
    if (!x || which>1 || !generation) return MSG_INVALID;
    pthread_mutex_lock(&x->pool);*generation=x->published[which];pthread_mutex_unlock(&x->pool);return MSG_OK;
}
MsgStatus mfe_restart(MfeExecutor *x,unsigned which) {
    if (!x || which>1) return MSG_INVALID;
    MsgStatus result=mfa_restart(&x->actor[which]);
    if(result==MSG_OK) {
        pthread_mutex_lock(&x->pool);x->published[which]=x->actor[which].actor.generation;pthread_mutex_unlock(&x->pool);
    }
    return result;
}
MsgStatus mfe_close(MfeExecutor *x,unsigned which) {return x?mfc_close(&x->input,which):MSG_INVALID;}
void mfe_cancel(MfeExecutor *x) {mfc_cancel(&x->input);}
MsgStatus mfe_destroy(MfeExecutor *x) {
    if (!x) return MSG_INVALID;
    if (x->actor[0].actor.stepping || x->actor[1].actor.stepping) return MSG_BUSY;
    MsgStatus result=mfc_destroy(&x->input);if(result!=MSG_OK) return result;
    (void)mfa_destroy(&x->actor[0]);(void)mfa_destroy(&x->actor[1]);
    pthread_mutex_destroy(&x->pool);return MSG_OK;
}
