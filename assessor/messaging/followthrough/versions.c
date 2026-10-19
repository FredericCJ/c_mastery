#include "versions.h"
#ifndef MFV_MUTANT
#define MFV_MUTANT 0
#endif
static size_t detach(MfvStore*s,size_t replaced,MflShared*garbage[8]){
    size_t n=0;for(size_t i=0;i<8;i++)if(s->payloads[i] && (i==replaced || !s->core.versions[i].live)){
        garbage[n++]=s->payloads[i];s->payloads[i]=NULL;
    }
    return n;
}
static void release_all(MflShared**p,size_t n){for(size_t i=0;i<n;i++)mfl_drop(&p[i]);}
MsgStatus mfv_init(MfvStore*s,MflAllocator a,MfoValue*initial){
    if(!s||!initial||(!initial->drop&&initial->data)||!a.allocate||!a.deallocate)return MSG_INVALID;
    if(pthread_mutex_init(&s->mutex,NULL))return MSG_FAILED;
    MsgStatus status=msg_versions_init(&s->core);
    if(status!=MSG_OK){pthread_mutex_destroy(&s->mutex);return status;}
    for(size_t i=0;i<8;i++)s->payloads[i]=NULL;
    status=mfl_make(a,initial,&s->payloads[0]);
    if(status!=MSG_OK){msg_versions_destroy(&s->core);pthread_mutex_destroy(&s->mutex);return status;}
    s->allocator=a;return MSG_OK;
}
MsgStatus mfv_snapshot(MfvStore*s,MsgSnapshot*out){
    if(!s||!out)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);MsgStatus status=msg_snapshot(&s->core,out);pthread_mutex_unlock(&s->mutex);return status;
}
MsgStatus mfv_read(MfvStore*s,MsgSnapshot pin,MflShared**out){
    if(!s||!out)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);unsigned ignored[2];MsgStatus status=msg_snapshot_read(&s->core,pin,ignored);
    if(status==MSG_OK){size_t best=8;uint64_t stamp=s->core.pins[pin.slot].stamp;
        for(size_t i=0;i<8;i++)if(s->core.versions[i].live && s->core.versions[i].stamp<=stamp && (best==8 || s->core.versions[i].stamp>s->core.versions[best].stamp))best=i;
        if(best==8 || !s->payloads[best])status=MSG_FAILED;
        else{status=MFV_MUTANT==62?MSG_OK:mfl_retain(s->payloads[best]);if(status==MSG_OK)*out=s->payloads[best];}
    }
    pthread_mutex_unlock(&s->mutex);return status;
}
MsgStatus mfv_drop_snapshot(MfvStore*s,MsgSnapshot pin){
    if(!s)return MSG_INVALID;
    MflShared*garbage[8];pthread_mutex_lock(&s->mutex);MsgStatus status=msg_snapshot_drop(&s->core,pin);
    size_t n=status==MSG_OK?detach(s,8,garbage):0;pthread_mutex_unlock(&s->mutex);release_all(garbage,n);return status;
}
MsgStatus mfv_publish(MfvStore*s,MfvClone clone,void*ctx,const void*source){
    if(!s||!clone||!source)return MSG_INVALID;
    MfoValue value={0};MsgStatus status=clone(ctx,source,&value);if(status!=MSG_OK)return status;
    MflShared*candidate=NULL;status=mfl_make(s->allocator,&value,&candidate);
    if(status!=MSG_OK){mfo_drop(&value);return status;}
    MflShared*garbage[8];size_t n=0;pthread_mutex_lock(&s->mutex);
    status=msg_publish(&s->core,0,0);
    if(status==MSG_OK){size_t current=8;for(size_t i=0;i<8;i++)if(s->core.versions[i].live && s->core.versions[i].stamp==s->core.clock)current=i;
        /* A slot reclaimed by this publication may already own a former payload. */
        n=detach(s,MFV_MUTANT==61?8:current,garbage);s->payloads[current]=candidate;candidate=NULL;
    }
    pthread_mutex_unlock(&s->mutex);release_all(garbage,n);mfl_drop(&candidate);return status;
}
MsgStatus mfv_destroy(MfvStore*s){
    if(!s)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);for(size_t i=0;i<4;i++)if(s->core.pins[i].active){pthread_mutex_unlock(&s->mutex);return MSG_BUSY;}
    MflShared*garbage[8];size_t n=0;for(size_t i=0;i<8;i++)if(s->payloads[i]){garbage[n++]=s->payloads[i];s->payloads[i]=NULL;}
    msg_versions_destroy(&s->core);pthread_mutex_unlock(&s->mutex);pthread_mutex_destroy(&s->mutex);release_all(garbage,n);return MSG_OK;
}
MsgStatus mfk_init(MfkStore*s,unsigned a,unsigned b){
    if(!s)return MSG_INVALID;
    if(pthread_mutex_init(&s->mutex,NULL))return MSG_FAILED;
    msg_store_init(&s->core);atomic_store(&s->core.cells[0].value,a);atomic_store(&s->core.cells[1].value,b);return MSG_OK;
}
MsgStatus mfk_begin(MfkStore*s,MfkTxn*out){
    if(!s||!out)return MSG_INVALID;
    MfkTxn candidate={.owner=s};pthread_mutex_lock(&s->mutex);msg_begin(&s->core,&candidate.core);
    MsgStatus status=msg_read(&candidate.core,0,&candidate.snapshot[0]);if(status==MSG_OK)status=msg_read(&candidate.core,1,&candidate.snapshot[1]);
    candidate.core.reads=0;pthread_mutex_unlock(&s->mutex);if(status==MSG_OK)*out=candidate;return status;
}
MsgStatus mfk_read(MfkTxn*t,unsigned key,unsigned*out){
    if(!t||!out||key>1||!t->core.active)return MSG_INVALID;
    t->observed|=1u<<key;*out=(t->core.writes&(1u<<key))?t->core.values[key]:t->snapshot[key];return MSG_OK;
}
MsgStatus mfk_nonzero(MfkTxn*t,unsigned*out){
    if(!t||!out||!t->core.active)return MSG_INVALID;
    unsigned a,b;
    MsgStatus status=mfk_read(t,0,&a);if(status==MSG_OK)status=mfk_read(t,1,&b);
    if(status==MSG_OK)*out=(a!=0)+(b!=0);
    return status;
}
MsgStatus mfk_write(MfkTxn*t,unsigned key,unsigned value){if(!t||key>1)return MSG_INVALID;return msg_write(&t->core,key,value);}
MsgStatus mfk_commit(MfkTxn*t,bool validate_predicates){
    if(!t||!t->core.active)return MSG_INVALID;
    t->core.reads=(MFV_MUTANT==64?0:t->core.writes)|((validate_predicates && MFV_MUTANT!=63)?t->observed:0);
    pthread_mutex_lock(&t->owner->mutex);MsgStatus status=msg_commit(&t->core);pthread_mutex_unlock(&t->owner->mutex);return status;
}
void mfk_destroy(MfkStore*s){pthread_mutex_destroy(&s->mutex);}
