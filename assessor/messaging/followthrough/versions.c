#include "versions.h"
#ifndef MFV_MUTANT
#define MFV_MUTANT 0
#endif
static size_t detach(struct mfv_store*s,size_t replaced,struct mfl_shared*garbage[8]){
    size_t n=0;for(size_t i=0;i<8;i++)if(s->payloads[i] && (i==replaced || !s->core.versions[i].live)){
        garbage[n++]=s->payloads[i];s->payloads[i]=NULL;
    }
    return n;
}
static void release_all(struct mfl_shared**p,size_t n){for(size_t i=0;i<n;i++)mfl_drop(&p[i]);}
enum msg_status mfv_init(struct mfv_store*s,struct mfl_allocator a,struct mfo_value*initial){
    if(!s||!initial||(!initial->drop&&initial->data)||!a.allocate||!a.deallocate)return MSG_INVALID;
    if(pthread_mutex_init(&s->mutex,NULL))return MSG_FAILED;
    enum msg_status status=msg_versions_init(&s->core);
    if(status!=MSG_OK){pthread_mutex_destroy(&s->mutex);return status;}
    for(size_t i=0;i<8;i++)s->payloads[i]=NULL;
    status=mfl_make(a,initial,&s->payloads[0]);
    if(status!=MSG_OK){msg_versions_destroy(&s->core);pthread_mutex_destroy(&s->mutex);return status;}
    s->allocator=a;return MSG_OK;
}
enum msg_status mfv_snapshot(struct mfv_store*s,struct msg_snapshot*out){
    if(!s||!out)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);enum msg_status status=msg_snapshot(&s->core,out);pthread_mutex_unlock(&s->mutex);return status;
}
enum msg_status mfv_read(struct mfv_store*s,struct msg_snapshot pin,struct mfl_shared**out){
    if(!s||!out)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);unsigned ignored[2];enum msg_status status=msg_snapshot_read(&s->core,pin,ignored);
    if(status==MSG_OK){size_t best=8;uint64_t stamp=s->core.pins[pin.slot].stamp;
        for(size_t i=0;i<8;i++)if(s->core.versions[i].live && s->core.versions[i].stamp<=stamp && (best==8 || s->core.versions[i].stamp>s->core.versions[best].stamp))best=i;
        if(best==8 || !s->payloads[best])status=MSG_FAILED;
        else{status=MFV_MUTANT==62?MSG_OK:mfl_retain(s->payloads[best]);if(status==MSG_OK)*out=s->payloads[best];}
    }
    pthread_mutex_unlock(&s->mutex);return status;
}
enum msg_status mfv_drop_snapshot(struct mfv_store*s,struct msg_snapshot pin){
    if(!s)return MSG_INVALID;
    struct mfl_shared*garbage[8];pthread_mutex_lock(&s->mutex);enum msg_status status=msg_snapshot_drop(&s->core,pin);
    size_t n=status==MSG_OK?detach(s,8,garbage):0;pthread_mutex_unlock(&s->mutex);release_all(garbage,n);return status;
}
enum msg_status mfv_publish(struct mfv_store*s,mfv_clone_fn clone,void*ctx,const void*source){
    if(!s||!clone||!source)return MSG_INVALID;
    struct mfo_value value={0};enum msg_status status=clone(ctx,source,&value);if(status!=MSG_OK)return status;
    struct mfl_shared*candidate=NULL;status=mfl_make(s->allocator,&value,&candidate);
    if(status!=MSG_OK){mfo_drop(&value);return status;}
    struct mfl_shared*garbage[8];size_t n=0;pthread_mutex_lock(&s->mutex);
    status=msg_publish(&s->core,0,0);
    if(status==MSG_OK){size_t current=8;for(size_t i=0;i<8;i++)if(s->core.versions[i].live && s->core.versions[i].stamp==s->core.clock)current=i;
        /* A slot reclaimed by this publication may already own a former payload. */
        n=detach(s,MFV_MUTANT==61?8:current,garbage);s->payloads[current]=candidate;candidate=NULL;
    }
    pthread_mutex_unlock(&s->mutex);release_all(garbage,n);mfl_drop(&candidate);return status;
}
enum msg_status mfv_destroy(struct mfv_store*s){
    if(!s)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);for(size_t i=0;i<4;i++)if(s->core.pins[i].active){pthread_mutex_unlock(&s->mutex);return MSG_BUSY;}
    struct mfl_shared*garbage[8];size_t n=0;for(size_t i=0;i<8;i++)if(s->payloads[i]){garbage[n++]=s->payloads[i];s->payloads[i]=NULL;}
    msg_versions_destroy(&s->core);pthread_mutex_unlock(&s->mutex);pthread_mutex_destroy(&s->mutex);release_all(garbage,n);return MSG_OK;
}
enum msg_status mfk_init(struct mfk_store*s,unsigned a,unsigned b){
    if(!s)return MSG_INVALID;
    if(pthread_mutex_init(&s->mutex,NULL))return MSG_FAILED;
    msg_store_init(&s->core);atomic_store(&s->core.cells[0].value,a);atomic_store(&s->core.cells[1].value,b);return MSG_OK;
}
enum msg_status mfk_begin(struct mfk_store*s,struct mfk_txn*out){
    if(!s||!out)return MSG_INVALID;
    struct mfk_txn candidate={.owner=s};pthread_mutex_lock(&s->mutex);msg_begin(&s->core,&candidate.core);
    enum msg_status status=msg_read(&candidate.core,0,&candidate.snapshot[0]);if(status==MSG_OK)status=msg_read(&candidate.core,1,&candidate.snapshot[1]);
    candidate.core.reads=0;pthread_mutex_unlock(&s->mutex);if(status==MSG_OK)*out=candidate;return status;
}
enum msg_status mfk_read(struct mfk_txn*t,unsigned key,unsigned*out){
    if(!t||!out||key>1||!t->core.active)return MSG_INVALID;
    t->observed|=1u<<key;*out=(t->core.writes&(1u<<key))?t->core.values[key]:t->snapshot[key];return MSG_OK;
}
enum msg_status mfk_nonzero(struct mfk_txn*t,unsigned*out){
    if(!t||!out||!t->core.active)return MSG_INVALID;
    unsigned a,b;
    enum msg_status status=mfk_read(t,0,&a);if(status==MSG_OK)status=mfk_read(t,1,&b);
    if(status==MSG_OK)*out=(a!=0)+(b!=0);
    return status;
}
enum msg_status mfk_write(struct mfk_txn*t,unsigned key,unsigned value){if(!t||key>1)return MSG_INVALID;return msg_write(&t->core,key,value);}
enum msg_status mfk_commit(struct mfk_txn*t,bool validate_predicates){
    if(!t||!t->core.active)return MSG_INVALID;
    t->core.reads=(MFV_MUTANT==64?0:t->core.writes)|((validate_predicates && MFV_MUTANT!=63)?t->observed:0);
    pthread_mutex_lock(&t->owner->mutex);enum msg_status status=msg_commit(&t->core);pthread_mutex_unlock(&t->owner->mutex);return status;
}
void mfk_destroy(struct mfk_store*s){pthread_mutex_destroy(&s->mutex);}
