#include "hedge.h"

static bool blob_valid(const RzhBlob *b) {
    return b && ((b->data && b->size && b->dispose) || (!b->data && !b->size && !b->dispose));
}
static void dispose(RzhBlob *b) {
    if(b->data)b->dispose(b->context,b->data);
    *b=(RzhBlob){0};
}
static bool live(const RzhArbiter *a,RzhTicket t) {
    if(t.owner!=a || !a->active || t.attempt>1)return false;
#if RZF_MUTANT != 73
    if(t.generation!=a->generation)return false;
#endif
    return (a->core.started & (1u<<t.attempt)) && !(a->finalized & (1u<<t.attempt));
}
static uint64_t now(RzhArbiter *a) {
    /* The injected clock is monotonic and all observations are serialized here. */
    a->last=a->clock(a->context);return a->last;
}
static bool quiet(const RzhArbiter *a) {
    if(!a->active)return true;
#if RZF_MUTANT == 72
    return rz_hedge_quiescent(&a->core);
#else
    return rz_hedge_quiescent(&a->core) && !a->callbacks && rzb_quiescent(&a->gate)==RZ_OK;
#endif
}
static void request_cancel(RzhArbiter *a) {
    unsigned outstanding=a->core.started & ~a->core.completed;
    a->core.cancel_requested|=outstanding;
    for(unsigned i=0;i<2;++i)if(outstanding & (1u<<i))(void)rzb_cancel(&a->gate,a->permits[i]);
}
int rzh_init(RzhArbiter *a,void *context,uint64_t (*clock)(void *),int (*decode)(void *,const unsigned char *,size_t,unsigned *)) {
    if(!a || !clock || !decode)return RZ_INVALID;
    *a=(RzhArbiter){.context=context,.clock=clock,.decode=decode};a->last=clock(context);
    if(pthread_mutex_init(&a->mutex,NULL))return RZ_FULL;
    (void)rzb_init(&a->gate,2,2,(RzbPolicy){8,8,1,1},a->last);
    (void)rz_receiver_init(&a->receiver,a->last,16);return RZ_OK;
}
int rzh_begin(RzhArbiter *a,RzfScope *scope,uint64_t delay,unsigned client,unsigned id,uint64_t fence,RzhTicket *out) {
    if(!a || !scope || !out || client>1 || !id || id>15 || !fence)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);
    int status=RZ_BUSY;
    if(!quiet(a) || (a->result_ready && !a->taken))goto done;
    if(a->generation==UINT64_MAX){status=RZ_FULL;goto done;}
    RZHedge staged;uint64_t time=now(a);
    status=rz_hedge_init(&staged,&scope->core,time,delay);if(status!=RZ_OK)goto done;
    RzfBorrow borrow={0};status=rzf_borrow_begin(&borrow,scope);if(status!=RZ_OK)goto done;
    RzbPermit permit;status=rzb_admit(&a->gate,0,1,scope,time,&permit);
    if(status!=RZ_OK){(void)rzf_borrow_ack(&borrow);goto done;}
    if(a->request.scope)(void)rzf_borrow_ack(&a->request);
    a->request=borrow;a->core=staged;a->permits[0]=permit;a->active=true;
    a->claimed=a->finalized=a->callbacks=0;a->abandoned=a->result_ready=a->taken=false;
    a->client=client;a->id=id;a->fence=fence;++a->generation;
    *out=(RzhTicket){a,a->generation,0};
done:(void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_start(RzhArbiter *a,RzhTicket *out) {
    if(!a || !out)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);int status=RZ_STALE;
    if(!a->active)goto done;
    if(a->abandoned){status=RZ_CANCELLED;goto done;}
    uint64_t time=now(a);RZHedge staged=a->core;status=rz_hedge_start(&staged,time);
    if(status!=RZ_OK)goto done;
    RzbPermit permit;status=rzb_admit(&a->gate,0,1,a->request.scope,time,&permit);
    if(status==RZ_OK){a->permits[1]=permit;a->core=staged;*out=(RzhTicket){a,a->generation,1};}
done:(void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_effect(RzhArbiter *a,RzhTicket t,int delta,int *out,bool *duplicate) {
    if(!a || !out || !duplicate || delta< -8 || delta>8)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);int status=RZ_STALE;
    if(!live(a,t) || (a->claimed & (1u<<t.attempt)))goto done;
    uint64_t time=now(a);bool cancelled;
    status=rzb_cancelled(&a->gate,a->permits[t.attempt],time,&cancelled);
    if(status==RZ_OK) {
        if(cancelled || a->abandoned)status=RZ_CANCELLED;
        else status=rz_receive(&a->receiver,time,a->client,a->id,a->fence,delta,out,duplicate);
    }
done:(void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_complete(RzhArbiter *a,RzhTicket t,RzhBlob *input) {
    if(!a || !blob_valid(input))return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);
    if(!live(a,t)){(void)pthread_mutex_unlock(&a->mutex);return RZ_STALE;}
    unsigned bit=1u<<t.attempt;
    if(a->claimed & bit){(void)pthread_mutex_unlock(&a->mutex);return RZ_BUSY;}
    a->claimed|=bit;++a->callbacks;RzhBlob local=*input;*input=(RzhBlob){0};
    (void)pthread_mutex_unlock(&a->mutex);
    unsigned value=0;int decoded=a->decode(a->context,local.data,local.size,&value);
    (void)pthread_mutex_lock(&a->mutex);uint64_t time=now(a);
    bool acceptable=decoded==RZ_OK && !a->abandoned;
#if RZF_MUTANT == 71
    acceptable=!a->abandoned;
#endif
    int status=rz_hedge_complete(&a->core,t.attempt,time,acceptable,value);
    if(status==RZ_OK && a->core.winner==(int)t.attempt) {
        a->winner_blob=local;local=(RzhBlob){0};a->result_ready=true;
    }
    if(a->core.winner>=0 || status==RZ_CANCELLED || status==RZ_EXPIRED || a->abandoned)request_cancel(a);
    int outcome=rz_check(&a->request.scope->core,time)!=RZ_OK || a->abandoned ? -1 : decoded==RZ_OK;
    (void)pthread_mutex_unlock(&a->mutex);
    dispose(&local);
    (void)pthread_mutex_lock(&a->mutex);
    (void)rzb_complete(&a->gate,a->permits[t.attempt],now(a),outcome);
    a->finalized|=bit;--a->callbacks;
    (void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_cancelled(RzhArbiter *a,RzhTicket t,bool *out) {
    if(!a || !out)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);int status=RZ_STALE;
    if(live(a,t))status=rzb_cancelled(&a->gate,a->permits[t.attempt],now(a),out);
    (void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_take(RzhArbiter *a,RzhResult *out) {
    if(!a || !out)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);int status=RZ_PENDING;
    if(a->taken)status=RZ_DONE;
    else if(a->result_ready){*out=(RzhResult){a->winner_blob,a->core.value,(unsigned)a->core.winner};a->winner_blob=(RzhBlob){0};a->taken=true;status=RZ_OK;}
    else if(a->abandoned)status=RZ_CANCELLED;
    (void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_abandon(RzhArbiter *a) {
    if(!a)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);
    if(!a->active){(void)pthread_mutex_unlock(&a->mutex);return RZ_STALE;}
    if(a->abandoned){(void)pthread_mutex_unlock(&a->mutex);return RZ_OK;}
    a->abandoned=true;request_cancel(a);RzhBlob local=a->winner_blob;a->winner_blob=(RzhBlob){0};
    if(a->result_ready)a->taken=true;
    ++a->callbacks;(void)pthread_mutex_unlock(&a->mutex);dispose(&local);
    (void)pthread_mutex_lock(&a->mutex);--a->callbacks;(void)pthread_mutex_unlock(&a->mutex);return RZ_OK;
}
int rzh_snapshot(RzhArbiter *a,RzhSnapshot *out) {
    if(!a || !out)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);
    *out=(RzhSnapshot){a->generation,a->core.started,a->core.completed,a->claimed,a->finalized,
        a->core.cancel_requested,a->callbacks,a->gate.used[0],a->active?a->core.winner:-1,a->receiver.total,
        a->abandoned,a->result_ready,a->taken};
    (void)pthread_mutex_unlock(&a->mutex);return RZ_OK;
}
int rzh_quiescent(RzhArbiter *a) {
    if(!a)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);int status=quiet(a)?RZ_OK:RZ_BUSY;
    (void)pthread_mutex_unlock(&a->mutex);return status;
}
int rzh_destroy(RzhArbiter *a) {
    if(!a)return RZ_INVALID;
    (void)pthread_mutex_lock(&a->mutex);
    if(!quiet(a) || (a->result_ready && !a->taken)){(void)pthread_mutex_unlock(&a->mutex);return RZ_BUSY;}
    if(a->request.scope)(void)rzf_borrow_ack(&a->request);
    (void)pthread_mutex_unlock(&a->mutex);(void)pthread_mutex_destroy(&a->mutex);return RZ_OK;
}
