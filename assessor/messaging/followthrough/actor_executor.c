#include "actor_executor.h"
#ifndef MFE_MUTANT
#define MFE_MUTANT 0
#endif
static void envelope_drop(void *p) {
    struct mfe_envelope *e=p;struct mfe_executor *x=e->owner;
    pthread_mutex_lock(&x->pool);struct mfo_value value=e->value;e->value=(struct mfo_value){0};e->used=false;
    pthread_mutex_unlock(&x->pool);mfo_drop(&value);
}
enum msg_status mfe_init(struct mfe_executor *x,size_t a,size_t b,mfa_handler_fn handler,void *contexts[2],mfa_dead_letter_fn dead,void *dc) {
    if (!x || !handler || !contexts || !dead || a>4 || b>4) return MSG_INVALID;
    if (pthread_mutex_init(&x->pool,NULL)) return MSG_FAILED;
    enum msg_status s=mfc_init(&x->input,a,b);
    if (s!=MSG_OK) {pthread_mutex_destroy(&x->pool);return s;}
    for(unsigned k=0;k<2;k++) {
        (void)mfa_init(&x->actor[k],handler,contexts[k],dead,dc);x->published[k]=x->actor[k].actor.generation;
    }
    for(unsigned k=0;k<8;k++) x->envelopes[k]=(struct mfe_envelope){.owner=x};
    return MSG_OK;
}
enum msg_status mfe_send(struct mfe_executor *x,unsigned destination,uint64_t generation,struct mfo_value *value,bool wait) {
    if (!x || destination>1 || !value || (value->data && !value->drop)) return MSG_INVALID;
    pthread_mutex_lock(&x->pool);unsigned slot=0;
    while(slot<8 && x->envelopes[slot].used) ++slot;
    if(slot==8) {pthread_mutex_unlock(&x->pool);return MSG_FULL;}
    struct mfe_envelope *e=&x->envelopes[slot];
    *e=(struct mfe_envelope){.owner=x,.value=*value,.generation=generation,.destination=destination,.used=true};
    pthread_mutex_unlock(&x->pool);
    struct mfo_value envelope={e,envelope_drop};enum msg_status result=mfc_send(&x->input,destination,&envelope,wait,NULL,NULL);
    if (result==MSG_OK) {
        /* A receiver may already have freed/reused e. Do not inspect it. */
        *value=(struct mfo_value){0};
    } else {
        /* No publication occurred, so this call still exclusively owns e. */
        pthread_mutex_lock(&x->pool);e->value=(struct mfo_value){0};e->used=false;pthread_mutex_unlock(&x->pool);
    }
    return result;
}
enum msg_status mfe_pump(struct mfe_executor *x,bool wait,struct mfe_event *out) {
    if (!x || !out) return MSG_INVALID;
    unsigned which;struct mfo_value envelope={0};enum msg_status status=mfc_select(&x->input,3,wait,&which,&envelope);
    if (status!=MSG_OK) return status;
    struct mfe_envelope *e=envelope.data;
    pthread_mutex_lock(&x->pool);struct mfo_value value=e->value;uint64_t generation=e->generation;
    unsigned destination=e->destination;e->value=(struct mfo_value){0};pthread_mutex_unlock(&x->pool);
    if (MFE_MUTANT!=101) mfo_drop(&envelope);
    struct mfa_actor *actor=&x->actor[destination];
    if (MFE_MUTANT==102) generation=actor->actor.generation;
    status=mfa_send(actor,generation,&value);
    if (status==MSG_OK) status=mfa_step(actor);
    else if (status!=MSG_STALE) {
        /* The channel accepted this owner. Failed actor admission is now a
           dead-letter transfer, not a return to its former producer. */
        actor->dead(actor->dead_ctx,generation,value);value=(struct mfo_value){0};
    }
    *out=(struct mfe_event){destination,generation,status};(void)which;return MSG_OK;
}
enum msg_status mfe_generation(struct mfe_executor *x,unsigned which,uint64_t *generation) {
    if (!x || which>1 || !generation) return MSG_INVALID;
    pthread_mutex_lock(&x->pool);*generation=x->published[which];pthread_mutex_unlock(&x->pool);return MSG_OK;
}
enum msg_status mfe_restart(struct mfe_executor *x,unsigned which) {
    if (!x || which>1) return MSG_INVALID;
    enum msg_status result=mfa_restart(&x->actor[which]);
    if(result==MSG_OK) {
        pthread_mutex_lock(&x->pool);x->published[which]=x->actor[which].actor.generation;pthread_mutex_unlock(&x->pool);
    }
    return result;
}
enum msg_status mfe_close(struct mfe_executor *x,unsigned which) {return x?mfc_close(&x->input,which):MSG_INVALID;}
void mfe_cancel(struct mfe_executor *x) {mfc_cancel(&x->input);}
enum msg_status mfe_destroy(struct mfe_executor *x) {
    if (!x) return MSG_INVALID;
    if (x->actor[0].actor.stepping || x->actor[1].actor.stepping) return MSG_BUSY;
    enum msg_status result=mfc_destroy(&x->input);if(result!=MSG_OK) return result;
    (void)mfa_destroy(&x->actor[0]);(void)mfa_destroy(&x->actor[1]);
    pthread_mutex_destroy(&x->pool);return MSG_OK;
}
