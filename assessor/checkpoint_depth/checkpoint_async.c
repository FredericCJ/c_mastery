#define _POSIX_C_SOURCE 200809L
#include "checkpoint_async.h"
#include <errno.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
/* Caller supplies live initialized objects; no pthread cancellation is permitted. */
static void transition(CPAQueue *q,CPALifecycle state){q->state=state;q->transitions|=1u<<(unsigned)state;}
static void settled(CPAQueue *q){
    if(q->state!=CPA_OPEN && !q->count && !q->owned)transition(q,CPA_STOPPED);
}
static void release(CPAQueue *q,unsigned index){
    CPASlot *s=&q->slots[index];
    if(s->generation==q->generation_limit)s->state=3;
    else{++s->generation;s->state=0;}
    ++q->releases;
}
static unsigned free_slot(const CPAQueue *q){for(unsigned i=0;i<CPA_SLOTS;++i)if(q->slots[i].state==0)return i;return CPA_SLOTS;}
static CPAStatus leave(CPAQueue *q,CPAStatus status){
    int notify=pthread_cond_broadcast(&q->changed),unlock=pthread_mutex_unlock(&q->mutex);
    return notify||unlock?CPA_ERROR:status;
}
static int wait_changed(CPAQueue *q,const struct timespec *deadline){
    ++q->waiters;
    int result=deadline?pthread_cond_timedwait(&q->changed,&q->mutex,deadline):pthread_cond_wait(&q->changed,&q->mutex);
    --q->waiters;++q->wakes;return result;
}
CPAStatus cpa_init(CPAQueue *q,unsigned limit){
    if(!q || !limit)return CPA_ERROR;
    if(pthread_mutex_init(&q->mutex,0))return CPA_ERROR;
    pthread_condattr_t attr;
    if(pthread_condattr_init(&attr)){(void)pthread_mutex_destroy(&q->mutex);return CPA_ERROR;}
    int error=pthread_condattr_setclock(&attr,CLOCK_MONOTONIC);
    if(!error)error=pthread_cond_init(&q->changed,&attr);
    (void)pthread_condattr_destroy(&attr);
    if(error){(void)pthread_mutex_destroy(&q->mutex);return CPA_ERROR;}
    for(unsigned i=0;i<CPA_SLOTS;++i)q->slots[i]=(CPASlot){{0,0,{0}},1,0};
    for(unsigned i=0;i<CPA_QUEUE;++i)q->queue[i]=0;
    q->head=q->count=q->owned=q->waiters=q->wakes=0;q->accepted=q->completed=q->discarded=q->rejected=q->releases=q->report_failures=0;
    q->generation_limit=limit;q->transitions=0;transition(q,CPA_OPEN);return CPA_OK;
}
CPAStatus cpa_offer(CPAQueue *q,const CPMessage *message,int blocking){
    if(!q || !message || message->length>CP_PAYLOAD || pthread_mutex_lock(&q->mutex))return CPA_ERROR;
    unsigned slot=free_slot(q);
    while(q->state==CPA_OPEN && (q->count==CPA_QUEUE || slot==CPA_SLOTS)){
        if(!blocking){++q->rejected;return leave(q,CPA_FULL);}
        /* A fully retired pool cannot regain capacity by waiting. */
        if(slot==CPA_SLOTS && !q->owned && !q->count){++q->rejected;return leave(q,CPA_FULL);}
        if(wait_changed(q,0))return leave(q,CPA_ERROR);
        slot=free_slot(q);
    }
    if(q->state!=CPA_OPEN){++q->rejected;return leave(q,CP_MUTANT==28?CPA_FULL:CPA_CLOSED);}
    q->slots[slot].message=*message;q->slots[slot].state=1;
    q->queue[(q->head+q->count)%CPA_QUEUE]=slot;++q->count;++q->accepted;
    if(CP_MUTANT==23)q->slots[slot].message.payload[0]=0;
    return leave(q,CPA_OK);
}
CPAStatus cpa_take(CPAQueue *q,CPAHandle *handle,CPMessage *message,int blocking){
    if(!q || !handle || !message || pthread_mutex_lock(&q->mutex))return CPA_ERROR;
    while(!q->count && q->state==CPA_OPEN){
        if(!blocking)return leave(q,CPA_PENDING);
        if(wait_changed(q,0))return leave(q,CPA_ERROR);
        if(CP_MUTANT==27 && !q->count)return leave(q,CPA_PENDING);
    }
    if(!q->count)return leave(q,CPA_CLOSED);
    unsigned slot=q->queue[q->head];q->head=(q->head+1)%CPA_QUEUE;--q->count;++q->owned;
    q->slots[slot].state=2;*handle=(CPAHandle){slot,q->slots[slot].generation};*message=q->slots[slot].message;
    return leave(q,CPA_OK);
}
CPAStatus cpa_finish(CPAQueue *q,CPAHandle h,int reporter_failed){
    if(!q || pthread_mutex_lock(&q->mutex))return CPA_ERROR;
    if(h.slot>=CPA_SLOTS || q->slots[h.slot].state!=2 || (CP_MUTANT!=24 && q->slots[h.slot].generation!=h.generation))return leave(q,CPA_STALE);
    --q->owned;++q->completed;if(reporter_failed)++q->report_failures;
    release(q,h.slot);settled(q);return leave(q,CPA_OK);
}
CPAStatus cpa_close(CPAQueue *q,int aborting){
    if(!q || pthread_mutex_lock(&q->mutex))return CPA_ERROR;
    if(q->state==CPA_STOPPED)return leave(q,CPA_CLOSED);
    if(q->state==CPA_OPEN)transition(q,CPA_CLOSING);
    if(aborting){
        transition(q,CPA_ABORTING);
        while(q->count){unsigned slot=q->queue[q->head];q->head=(q->head+1)%CPA_QUEUE;--q->count;++q->discarded;release(q,slot);}
        if(CP_MUTANT==25)for(unsigned i=0;i<CPA_SLOTS;++i)if(q->slots[i].state==2){--q->owned;++q->discarded;release(q,i);}
    }else if(q->state!=CPA_ABORTING)transition(q,CPA_DRAINING);
    settled(q);return leave(q,q->state==CPA_STOPPED?CPA_CLOSED:CPA_PENDING);
}
CPAStatus cpa_wait_closed(CPAQueue *q,const struct timespec *deadline){
    if(!q || !deadline || pthread_mutex_lock(&q->mutex))return CPA_ERROR;
    while(q->state!=CPA_STOPPED){
        int error=wait_changed(q,deadline);
        if(error==ETIMEDOUT){if(CP_MUTANT==26)transition(q,CPA_STOPPED);return leave(q,CP_MUTANT==26?CPA_CLOSED:CPA_PENDING);}
        if(error)return leave(q,CPA_ERROR);
    }
    return leave(q,CPA_CLOSED);
}
CPAStatus cpa_destroy(CPAQueue *q){
    /* External quiescence: no thread can enter after this ownership decision. */
    if(!q || pthread_mutex_lock(&q->mutex))return CPA_ERROR;
    int ready=q->state==CPA_STOPPED && !q->waiters && !q->owned && !q->count;
    if(pthread_mutex_unlock(&q->mutex))return CPA_ERROR;
    if(!ready)return CPA_PENDING;
    if(pthread_cond_destroy(&q->changed) || pthread_mutex_destroy(&q->mutex))return CPA_ERROR;
    return CPA_OK;
}
