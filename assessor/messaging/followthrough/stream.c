#include "stream.h"
#include <limits.h>
#ifndef MFQ_MUTANT
#define MFQ_MUTANT 0
#endif
static void next(void*ctx,unsigned value){struct mfq_stream*s=ctx;s->next(s->context.data,value);}
static void terminal(void*ctx,enum msg_status status){struct mfq_stream*s=ctx;s->terminal(s->context.data,status);}
static void finish(struct mfq_stream*s){
    pthread_mutex_lock(&s->mutex);unsigned which,value;
    while(msg_select(&s->channel,1,false,&which,&value)==MSG_OK){--s->stats.queued;++s->stats.discarded;}
    s->stats.finished=true;pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);
}
static void*executor(void*ctx){struct mfq_stream*s=ctx;
    for(;;){pthread_mutex_lock(&s->mutex);
        if(s->cancel_pending){pthread_mutex_unlock(&s->mutex);msg_stream_cancel(&s->core);finish(s);return NULL;}
        if(s->invalid_pending){pthread_mutex_unlock(&s->mutex);(void)msg_request(&s->core,0);finish(s);return NULL;}
        if(s->end_pending){enum msg_status status=s->end_status;pthread_mutex_unlock(&s->mutex);(void)msg_stream_end(&s->core,status);finish(s);return NULL;}
        if(s->requests){uint64_t n=s->requests;s->requests=0;pthread_mutex_unlock(&s->mutex);(void)msg_request(&s->core,n);continue;}
        if(s->stats.queued && (s->core.demand || MFQ_MUTANT==81)){
            unsigned which,value;enum msg_status status=msg_select(&s->channel,1,false,&which,&value);
            if(status!=MSG_OK){s->end_pending=true;s->end_status=MSG_FAILED;s->stats.accepting=false;pthread_mutex_unlock(&s->mutex);continue;}
            --s->stats.queued;s->stats.inflight=1;pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);
            if(MFQ_MUTANT==81 && !s->core.demand)(void)msg_request(&s->core,1);
            status=msg_emit(&s->core,value);
            pthread_mutex_lock(&s->mutex);s->stats.inflight=0;if(status==MSG_OK)++s->stats.delivered;else ++s->stats.discarded;
            pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);continue;
        }
        s->idle=true;pthread_cond_broadcast(&s->changed);pthread_cond_wait(&s->changed,&s->mutex);pthread_mutex_unlock(&s->mutex);
    }
}
enum msg_status mfq_init(struct mfq_stream*s,unsigned capacity,struct mfo_value*ctx,msg_next_fn n,msg_terminal_fn t,bool fail_start){
    if(!s||!ctx||!ctx->data||!ctx->drop||!n||!t||capacity<1||capacity>4)return MSG_INVALID;
    if(pthread_mutex_init(&s->mutex,NULL))return MSG_FAILED;
    if(pthread_cond_init(&s->changed,NULL)){pthread_mutex_destroy(&s->mutex);return MSG_FAILED;}
    enum msg_status status=msg_hub_init(&s->channel,capacity,1);if(status!=MSG_OK){pthread_cond_destroy(&s->changed);pthread_mutex_destroy(&s->mutex);return status;}
    s->context=*ctx;s->next=n;s->terminal=t;s->capacity=capacity;s->requests=0;
    s->cancel_pending=s->invalid_pending=s->end_pending=s->idle=false;s->end_status=MSG_OK;s->stats=(struct mfq_stats){.accepting=true};
    msg_stream_init(&s->core,next,terminal,s);
    if(fail_start || pthread_create(&s->worker,NULL,executor,s)){msg_hub_destroy(&s->channel);pthread_cond_destroy(&s->changed);pthread_mutex_destroy(&s->mutex);return MSG_FAILED;}
    *ctx=(struct mfo_value){0};return MSG_OK;
}
enum msg_status mfq_acquire(struct mfq_stream*s,struct mfq_handle*out){
    if(!s||!out)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);enum msg_status status=MSG_OK;
    if(!s->stats.accepting)status=MSG_CLOSED;else if(s->stats.handles==UINT_MAX)status=MSG_OVERFLOW;
    else{++s->stats.handles;*out=(struct mfq_handle){s};}
    pthread_mutex_unlock(&s->mutex);return status;
}
void mfq_release(struct mfq_handle*h){if(!h||!h->owner)return;struct mfq_stream*s=h->owner;h->owner=NULL;
    pthread_mutex_lock(&s->mutex);--s->stats.handles;pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);
}
enum msg_status mfq_request(struct mfq_handle*h,uint64_t n){
    if(!h||!h->owner)return MSG_INVALID;
    struct mfq_stream*s=h->owner;pthread_mutex_lock(&s->mutex);if(!s->stats.accepting){pthread_mutex_unlock(&s->mutex);return MSG_CLOSED;}
    s->idle=false;
    if(!n){s->invalid_pending=true;s->stats.accepting=false;}else s->requests=UINT64_MAX-s->requests<n?UINT64_MAX:s->requests+n;
    pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);return MSG_OK;
}
enum msg_status mfq_submit(struct mfq_handle*h,unsigned value,enum mfq_policy policy){
    if(!h||!h->owner||(policy!=MFQ_WAIT&&policy!=MFQ_DROP&&policy!=MFQ_FAIL))return MSG_INVALID;
    struct mfq_stream*s=h->owner;pthread_mutex_lock(&s->mutex);
    while(s->stats.accepting && s->stats.queued==s->capacity){
        if(policy!=MFQ_WAIT){if(s->stats.rejected<UINT64_MAX)++s->stats.rejected;
            if(policy==MFQ_FAIL && MFQ_MUTANT!=84){s->end_pending=true;s->end_status=MSG_FAILED;s->stats.accepting=false;pthread_cond_broadcast(&s->changed);}
            pthread_mutex_unlock(&s->mutex);return policy==MFQ_FAIL?MSG_FAILED:MSG_FULL;
        }
        if(pthread_equal(pthread_self(),s->worker)){pthread_mutex_unlock(&s->mutex);return MSG_BUSY;}
        ++s->stats.waiting;pthread_cond_broadcast(&s->changed);pthread_cond_wait(&s->changed,&s->mutex);--s->stats.waiting;
        if(MFQ_MUTANT==83){pthread_mutex_unlock(&s->mutex);return MSG_FULL;}
    }
    if(!s->stats.accepting){pthread_mutex_unlock(&s->mutex);return MSG_CLOSED;}
    if(s->stats.accepted==UINT64_MAX){pthread_mutex_unlock(&s->mutex);return MSG_OVERFLOW;}
    enum msg_status status=msg_send(&s->channel,0,value,false);
    if(status==MSG_OK){s->idle=false;++s->stats.accepted;++s->stats.queued;pthread_cond_broadcast(&s->changed);}
    pthread_mutex_unlock(&s->mutex);return status;
}
enum msg_status mfq_end(struct mfq_handle*h,enum msg_status status){
    if(!h||!h->owner||(status!=MSG_OK&&status!=MSG_FAILED&&status!=MSG_INVALID))return MSG_INVALID;
    struct mfq_stream*s=h->owner;pthread_mutex_lock(&s->mutex);if(!s->stats.accepting){pthread_mutex_unlock(&s->mutex);return MSG_CLOSED;}
    s->idle=false;s->stats.accepting=false;s->end_pending=true;s->end_status=status;pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);return MSG_OK;
}
enum msg_status mfq_close(struct mfq_stream*s){
    if(!s)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);s->idle=false;s->stats.accepting=false;s->cancel_pending=true;pthread_cond_broadcast(&s->changed);pthread_mutex_unlock(&s->mutex);return MSG_OK;
}
enum msg_status mfq_cancel(struct mfq_handle*h){if(!h||!h->owner)return MSG_INVALID;return mfq_close(h->owner);}
enum msg_status mfq_join(struct mfq_stream*s){
    if(!s)return MSG_INVALID;
    if(pthread_equal(pthread_self(),s->worker))return MSG_BUSY;
    pthread_mutex_lock(&s->mutex);if(s->stats.accepting){pthread_mutex_unlock(&s->mutex);return MSG_BUSY;}
    bool joined=s->stats.joined;pthread_mutex_unlock(&s->mutex);if(joined)return MSG_OK;
    if(pthread_join(s->worker,NULL))return MSG_FAILED;
    pthread_mutex_lock(&s->mutex);s->stats.joined=true;pthread_mutex_unlock(&s->mutex);return MSG_OK;
}
enum msg_status mfq_idle(struct mfq_stream*s){
    if(!s)return MSG_INVALID;
    if(pthread_equal(pthread_self(),s->worker))return MSG_BUSY;
    pthread_mutex_lock(&s->mutex);while(!s->idle&&!s->stats.finished)pthread_cond_wait(&s->changed,&s->mutex);pthread_mutex_unlock(&s->mutex);return MSG_OK;
}
enum msg_status mfq_stats(struct mfq_stream*s,struct mfq_stats*out){if(!s||!out)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);*out=s->stats;pthread_mutex_unlock(&s->mutex);return MSG_OK;
}
enum msg_status mfq_destroy(struct mfq_stream*s){
    if(!s)return MSG_INVALID;
    pthread_mutex_lock(&s->mutex);if(!s->stats.joined || (s->stats.handles && MFQ_MUTANT!=82)){pthread_mutex_unlock(&s->mutex);return MSG_BUSY;}
    pthread_mutex_unlock(&s->mutex);msg_hub_destroy(&s->channel);pthread_cond_destroy(&s->changed);pthread_mutex_destroy(&s->mutex);mfo_drop(&s->context);return MSG_OK;
}
