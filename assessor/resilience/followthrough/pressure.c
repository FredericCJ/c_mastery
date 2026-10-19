#include "pressure.h"
#include <limits.h>

static bool value_ok(const RzpValue *v) {return v && v->owned && v->dispose;}
void rzp_value_drop(RzpValue *v) {
    if(v && v->owned) {void *owned=v->owned;void (*dispose)(void *)=v->dispose;*v=(RzpValue){0};dispose(owned);}
}
void rzp_owned_init(RzpOwnedQueue *q) {if(q)*q=(RzpOwnedQueue){0};}
int rzp_owned_offer(RzpOwnedQueue *q,const RZScope *scope,uint64_t now,RZPolicy policy,RzpValue *value) {
    if(!q || !scope || !value_ok(value) || policy<RZ_BLOCK || policy>RZ_DEGRADE)return RZ_INVALID;
    size_t head=q->core.head,count=q->core.count,last=(head+3)%4;
    RZOffer offer={scope,value->value,true};int status=rz_offer(&q->core,&offer,policy,now);
    if(status==RZ_PENDING)return status;
    if(status==RZ_OK) {
        size_t index=(head+count)%4;
        if(count==4) {
#if RZF_MUTANT != 53
            rzp_value_drop(&q->values[index]);
#endif
        }
        q->values[index]=*value;*value=(RzpValue){0};
    } else if(status==RZ_DEGRADED && value->value>q->values[last].value) {
        rzp_value_drop(&q->values[last]);q->values[last]=*value;*value=(RzpValue){0};
    } else rzp_value_drop(value);
    return status;
}
int rzp_owned_pop(RzpOwnedQueue *q,RzpValue *out) {
    if(!q || !out)return RZ_INVALID;
    size_t index=q->core.head;unsigned unused;int status=rz_pop(&q->core,&unused);
    if(status==RZ_OK){*out=q->values[index];q->values[index]=(RzpValue){0};}
    return status;
}
void rzp_owned_close(RzpOwnedQueue *q) {if(q)q->core.closed=true;}

enum {FREE,QUEUED,TRANSPORT,LEASED,RETIRED};
static void finish(RzpProducer *p,int result) {
    rzp_value_drop(&p->value);p->pending=false;p->result=result;
    (void)rzf_borrow_ack(&p->scope);
}
static void progress(RzpPump *p) {
    unsigned start=p->next;
    for(unsigned j=0;j<2;++j) {
        unsigned k=(start+j)%2;RzpProducer *producer=&p->producer[k];
        if(!producer->active || !producer->pending)continue;
        int status=rz_check(&producer->scope.scope->core,p->last);
        if(status!=RZ_OK){finish(producer,status);continue;}
        if(p->closed){finish(producer,RZ_CLOSED);continue;}
        if(p->consumed==UINT64_MAX){finish(producer,RZ_FULL);continue;}
        if(!p->credits)continue;
        unsigned slot=0;while(slot<4 && p->slots[slot].state!=FREE)++slot;
        if(slot==4)continue;
        RzpSlot *s=&p->slots[slot];
        RZOffer offer={&producer->scope.scope->core,slot,true};
        status=rz_offer(&p->queue,&offer,RZ_BLOCK,p->last);
        if(status!=RZ_OK)continue;
        ++s->generation;s->state=QUEUED;s->value=producer->value;producer->value=(RzpValue){0};
        --p->credits;++p->consumed;p->next=(k+1)%2;finish(producer,RZ_OK);
    }
    pthread_cond_broadcast(&p->changed);
}
static bool ticket_ok(const RzpPump *p,RzpTicket t) {
    return t.owner==p && t.index<2 && p->producer[t.index].active && p->producer[t.index].generation==t.generation;
}
int rzp_init(RzpPump *p,size_t capacity) {
    if(!p || !capacity || capacity>4)return RZ_INVALID;
    *p=(RzpPump){0};
    if(pthread_mutex_init(&p->mutex,NULL))return RZ_CLOSED;
    if(pthread_cond_init(&p->changed,NULL)){pthread_mutex_destroy(&p->mutex);return RZ_CLOSED;}
    if(msg_hub_init(&p->hub,capacity,1)!=MSG_OK) {
        pthread_cond_destroy(&p->changed);pthread_mutex_destroy(&p->mutex);return RZ_CLOSED;
    }
    p->credits=4;p->initialized=true;return RZ_OK;
}
int rzp_begin(RzpPump *p,unsigned producer,RzfScope *scope,uint64_t now,RzpValue *value,RzpTicket *out) {
    if(!p || !p->initialized || producer>1 || !scope || !value_ok(value) || !out)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);int status=RZ_OK;RzpProducer *slot=&p->producer[producer];
    if(now<p->last)status=RZ_RANGE;
    else if(p->closed)status=RZ_CLOSED;
    else if(slot->active)status=RZ_BUSY;
    else if(slot->generation==UINT_MAX)status=RZ_FULL;
#if RZF_MUTANT == 51
    else if(!p->credits)status=RZ_PENDING;
#endif
    else if((status=rzf_borrow_begin(&slot->scope,scope))==RZ_OK) {
        /* Register the bounded owner before checking the readiness predicate. */
        slot->active=slot->pending=true;++slot->generation;
        slot->value=*value;*value=(RzpValue){0};p->last=now;
        *out=(RzpTicket){p,producer,slot->generation};progress(p);
        status=slot->pending ? RZ_PENDING : slot->result;
    }
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_poll(RzpPump *p,RzpTicket ticket,bool wait,int *out) {
    if(!p || !p->initialized || !out)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);int status=RZ_STALE;
    if(ticket_ok(p,ticket)) {
        RzpProducer *slot=&p->producer[ticket.index];
        if(slot->polling)status=RZ_BUSY;
        else {
            slot->polling=true;
            while(slot->pending && wait)pthread_cond_wait(&p->changed,&p->mutex);
            if(slot->pending)status=RZ_PENDING;
            else {*out=slot->result;slot->active=false;status=RZ_OK;}
            slot->polling=false;
        }
    }
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_cancel(RzpPump *p,RzpTicket ticket) {
    if(!p || !p->initialized)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);int status=RZ_STALE;
    if(ticket_ok(p,ticket)) {
        RzpProducer *slot=&p->producer[ticket.index];status=RZ_DONE;
        if(slot->pending){finish(slot,RZ_CANCELLED);pthread_cond_broadcast(&p->changed);status=RZ_OK;}
    }
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_tick(RzpPump *p,uint64_t now) {
    if(!p || !p->initialized)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);int status=RZ_RANGE;
    if(now>=p->last){p->last=now;progress(p);status=RZ_OK;}
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_forward(RzpPump *p) {
    if(!p || !p->initialized)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);unsigned index=p->queue.count?p->queue.values[p->queue.head]:0;
    int status=rz_forward(&p->queue,&p->hub,0);
    if(status==RZ_OK)p->slots[index].state=TRANSPORT;
    if(p->closed && !p->queue.count)(void)msg_close(&p->hub,0);
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_receive(RzpPump *p,RzpValue *out,RzpCredit *credit) {
    if(!p || !p->initialized || !out || !credit)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);unsigned channel,index;int status=RZ_INVALID;
    MsgStatus result=msg_select(&p->hub,1,false,&channel,&index);
    if(result==MSG_OK) {
        RzpSlot *slot=&p->slots[index];slot->state=LEASED;*out=slot->value;
        *credit=(RzpCredit){p,index,slot->generation};status=RZ_OK;
#if RZF_MUTANT == 54
        ++p->credits;++p->returned;
#endif
    } else if(result==MSG_EMPTY)status=RZ_EMPTY;
    else if(result==MSG_CLOSED)status=RZ_CLOSED;
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_release(RzpPump *p,RzpCredit credit) {
    if(!p || !p->initialized)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);int status=RZ_STALE;
    if(credit.owner==p && credit.index<4) {
        RzpSlot *slot=&p->slots[credit.index];
#if RZF_MUTANT == 52
        if(slot->generation==credit.generation && slot->state==FREE){++p->credits;++p->returned;status=RZ_OK;}
#endif
        if(slot->generation==credit.generation && slot->state==LEASED) {
            rzp_value_drop(&slot->value);
            slot->state=slot->generation==UINT_MAX ? RETIRED : FREE;
            if(slot->state==FREE){++p->credits;++p->returned;}
            progress(p);status=RZ_OK;
        }
    }
    pthread_mutex_unlock(&p->mutex);return status;
}
int rzp_close(RzpPump *p) {
    if(!p || !p->initialized)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);p->closed=true;progress(p);
    if(!p->queue.count)(void)msg_close(&p->hub,0);
    pthread_mutex_unlock(&p->mutex);return RZ_OK;
}
int rzp_snapshot(RzpPump *p,RzpSnapshot *out) {
    if(!p || !p->initialized || !out)return RZ_INVALID;
    pthread_mutex_lock(&p->mutex);RzpSnapshot s={0};
    for(unsigned i=0;i<4;++i) {
        s.queued+=p->slots[i].state==QUEUED;s.transport+=p->slots[i].state==TRANSPORT;
        s.leased+=p->slots[i].state==LEASED;s.retired+=p->slots[i].state==RETIRED;
    }
    for(unsigned i=0;i<2;++i){s.pending+=p->producer[i].active && p->producer[i].pending;s.terminal+=p->producer[i].active && !p->producer[i].pending;}
    s.credits=p->credits;s.consumed=p->consumed;s.returned=p->returned;*out=s;
    pthread_mutex_unlock(&p->mutex);return RZ_OK;
}
int rzp_destroy(RzpPump *p) {
    if(!p || !p->initialized)return RZ_INVALID;
    /* Caller has joined all threads; no concurrent entry is permitted here. */
    if(!p->closed || p->queue.count)return RZ_BUSY;
    for(unsigned i=0;i<2;++i)if(p->producer[i].active)return RZ_BUSY;
    for(unsigned i=0;i<4;++i)if(p->slots[i].state!=FREE && p->slots[i].state!=RETIRED)return RZ_BUSY;
    msg_hub_destroy(&p->hub);pthread_cond_destroy(&p->changed);pthread_mutex_destroy(&p->mutex);
    p->initialized=false;return RZ_OK;
}
