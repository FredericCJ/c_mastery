#include "registration.h"
#include <errno.h>
#ifndef MFR_MUTANT
#define MFR_MUTANT 0
#endif
struct mfr_waiter {struct mfr_waiter *next;bool notified;};
static void notify(struct mfr_hub *h) {
    for (struct mfr_waiter *w=h->waiters;w;w=w->next)
        if (MFR_MUTANT!=91) w->notified=true;
    (void)pthread_cond_broadcast(&h->changed);
}
enum msg_status mfr_init(struct mfr_hub *h,size_t a,size_t b) {
    if (!h || !a || a>4 || !b || b>4) return MSG_INVALID;
    if (pthread_mutex_init(&h->registry,NULL)) return MSG_FAILED;
    if (pthread_mutex_init(&h->channel_mutex[0],NULL)) {
        pthread_mutex_destroy(&h->registry);return MSG_FAILED;
    }
    if (pthread_mutex_init(&h->channel_mutex[1],NULL)) {
        pthread_mutex_destroy(&h->channel_mutex[0]);
        pthread_mutex_destroy(&h->registry);return MSG_FAILED;
    }
    if (pthread_cond_init(&h->changed,NULL)) {
        pthread_mutex_destroy(&h->channel_mutex[1]);
        pthread_mutex_destroy(&h->channel_mutex[0]);
        pthread_mutex_destroy(&h->registry);return MSG_FAILED;
    }
    h->channel[0]=(struct msg_channel){.capacity=a};
    h->channel[1]=(struct msg_channel){.capacity=b};
    h->waiters=NULL;h->waiting=0;h->sleeps=0;h->next=0;h->stop=false;
    return MSG_OK;
}
enum msg_status mfr_send(struct mfr_hub *h,unsigned k,unsigned value) {
    if (!h || k>1) return MSG_INVALID;
    pthread_mutex_lock(&h->channel_mutex[k]);pthread_mutex_lock(&h->registry);
    struct msg_channel *c=&h->channel[k];
    enum msg_status result=MSG_OK;
    if (h->stop) result=MSG_CANCELLED;
    else if (c->closed) result=MSG_CLOSED;
    else if (c->count==c->capacity) result=MSG_FULL;
    else {
        c->values[(c->head+c->count)%4]=value;++c->count;notify(h);
    }
    pthread_mutex_unlock(&h->registry);pthread_mutex_unlock(&h->channel_mutex[k]);
    return result;
}
static enum msg_status scan(struct mfr_hub *h,unsigned mask,unsigned *which,unsigned *value) {
    pthread_mutex_lock(&h->registry);unsigned start=h->next;
    pthread_mutex_unlock(&h->registry);
    unsigned live=0;
    for (unsigned j=0;j<2;j++) {
        unsigned k=(start+j)%2;
        if (!(mask&(1u<<k))) continue;
        pthread_mutex_lock(&h->channel_mutex[k]);pthread_mutex_lock(&h->registry);
        struct msg_channel *c=&h->channel[k];enum msg_status result=MSG_EMPTY;
        if (h->stop) result=MSG_CANCELLED;
        else {
            if (!c->closed) ++live;
            if (c->count && !(MFR_MUTANT==92 && c->closed)) {
                *which=k;*value=c->values[c->head];c->head=(c->head+1)%4;
                --c->count;h->next=1-k;notify(h);result=MSG_OK;
            }
        }
        pthread_mutex_unlock(&h->registry);pthread_mutex_unlock(&h->channel_mutex[k]);
        if (result!=MSG_EMPTY) return result;
    }
    return live?MSG_EMPTY:MSG_CLOSED;
}
static enum msg_status select_impl(struct mfr_hub *h,unsigned mask,bool wait,unsigned *which,
                             unsigned *value,const struct timespec *deadline,
                             mfr_probe_fn probe,void *context) {
    if (!h || !which || !value || which==value || !mask || (mask&~3u)) return MSG_INVALID;
    struct mfr_waiter node={0};
    pthread_mutex_lock(&h->registry);
    if (h->waiting==SIZE_MAX) {pthread_mutex_unlock(&h->registry);return MSG_OVERFLOW;}
    node.next=h->waiters;h->waiters=&node;++h->waiting;
    pthread_mutex_unlock(&h->registry);
    enum msg_status result;bool expired=false,probed=false;
    for (;;) {
        pthread_mutex_lock(&h->registry);node.notified=false;
        pthread_mutex_unlock(&h->registry);
        result=scan(h,mask,which,value);
        if (result!=MSG_EMPTY || !wait || expired) break;
        if (probe && !probed) {probed=true;probe(context);}
        pthread_mutex_lock(&h->registry);
        if (h->stop) result=MSG_CANCELLED;
        else if (!node.notified) {
            if (h->sleeps<UINT64_MAX) ++h->sleeps;
            int rc=deadline?pthread_cond_timedwait(&h->changed,&h->registry,deadline):
                            pthread_cond_wait(&h->changed,&h->registry);
            if (rc==ETIMEDOUT) expired=true;
            else if (rc) result=MSG_FAILED;
        }
        pthread_mutex_unlock(&h->registry);
        if (result!=MSG_EMPTY) break;
    }
    pthread_mutex_lock(&h->registry);
    struct mfr_waiter **at=&h->waiters;
    while (*at!=&node) at=&(*at)->next;
    *at=node.next;--h->waiting;
    pthread_mutex_unlock(&h->registry);
    return result;
}
enum msg_status mfr_select(struct mfr_hub *h,unsigned mask,bool wait,unsigned *which,unsigned *value) {
    return select_impl(h,mask,wait,which,value,NULL,NULL,NULL);
}
enum msg_status mfr_select_probe(struct mfr_hub *h,unsigned mask,unsigned *which,unsigned *value,
                           const struct timespec *deadline,mfr_probe_fn probe,void *context) {
    return select_impl(h,mask,true,which,value,deadline,probe,context);
}
enum msg_status mfr_close(struct mfr_hub *h,unsigned k) {
    if (!h || k>1) return MSG_INVALID;
    pthread_mutex_lock(&h->channel_mutex[k]);pthread_mutex_lock(&h->registry);
    h->channel[k].closed=true;notify(h);
    pthread_mutex_unlock(&h->registry);pthread_mutex_unlock(&h->channel_mutex[k]);
    return MSG_OK;
}
void mfr_cancel(struct mfr_hub *h) {
    pthread_mutex_lock(&h->registry);h->stop=true;notify(h);
    pthread_mutex_unlock(&h->registry);
}
void mfr_observe(struct mfr_hub *h,size_t *waiting,uint64_t *sleeps) {
    pthread_mutex_lock(&h->registry);*waiting=h->waiting;*sleeps=h->sleeps;
    pthread_mutex_unlock(&h->registry);
}
enum msg_status mfr_destroy(struct mfr_hub *h) {
    pthread_mutex_lock(&h->registry);bool busy=h->waiting!=0;
    pthread_mutex_unlock(&h->registry);
    if (busy) return MSG_BUSY;
    pthread_cond_destroy(&h->changed);pthread_mutex_destroy(&h->channel_mutex[1]);
    pthread_mutex_destroy(&h->channel_mutex[0]);pthread_mutex_destroy(&h->registry);
    return MSG_OK;
}
