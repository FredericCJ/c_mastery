#define _POSIX_C_SOURCE 200809L
#include "lifecycle.h"
#include <errno.h>
#include <stdlib.h>
typedef struct { unsigned input, result; } TLCell;
struct TLPool { pthread_t threads[4]; TLCell cells[4]; unsigned created, joined; int create_error; };
static void *square(void *arg) { TLCell *c=arg; c->result=c->input*c->input; return NULL; }
TLPool *tl_pool_start(unsigned n,unsigned fail,int allocation,int *error) {
    if(!error)return NULL;
    if(n>4||fail>n||allocation<0||allocation>1){*error=EINVAL;return NULL;}
    TLPool *p=allocation?NULL:calloc(1,sizeof *p);
    if(!p){*error=ENOMEM;return NULL;}
    for(unsigned i=0;i<n;i++){
        p->cells[i].input=i+1;
        int e=fail==i+1?EAGAIN:pthread_create(&p->threads[i],NULL,square,&p->cells[i]);
        if(e){p->create_error=e;break;}
        p->created++;
    }
    *error=0;return p;
}
int tl_pool_collect(TLPool *p,TLJoin join,void *context,TLStats *out) {
    if(!p||!out)return EINVAL;
    while(p->joined<p->created){
        void *result=NULL;
        int e=join?join(p->threads[p->joined],&result,context):pthread_join(p->threads[p->joined],&result);
        if(e){
#ifdef TL_MUTANT2
            out->joined=p->joined;
#endif
            return e;
        }
        p->joined++;
    }
    unsigned sum=0;for(unsigned i=0;i<p->created;i++)sum+=p->cells[i].result;
#ifdef TL_MUTANT1
    sum=0;
#endif
    *out=(TLStats){p->created,p->joined,p->created,sum,p->create_error};return 0;
}
int tl_pool_dispose(TLPool **p){
    if(!p||!*p)return EINVAL;
    if((*p)->joined!=(*p)->created)return EBUSY;
    free(*p);*p=NULL;return 0;
}
typedef struct { TLMonitor *monitor; unsigned char bytes[16]; size_t length; } TLJob;
static void invariant(int e){if(e)abort();}
static void release_job(TLJob *j){
    TLMonitor *m=j->monitor;invariant(pthread_mutex_lock(&m->mutex));
#ifdef TL_MUTANT3
    if(m->live_refs==2&&!m->release_worker)m->completed=1;
#endif
    if(--m->live_refs==0){free(j);m->freed++;}
    invariant(pthread_cond_broadcast(&m->changed));invariant(pthread_mutex_unlock(&m->mutex));
}
static void *detached_worker(void *arg){
    TLJob *j=arg;TLMonitor *m=j->monitor;
    invariant(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL));
    invariant(pthread_mutex_lock(&m->mutex));
    while(!m->release_worker)invariant(pthread_cond_wait(&m->changed,&m->mutex));
    unsigned sum=0;for(size_t i=0;i<j->length;i++)sum+=j->bytes[i];
    m->sum=sum;m->completed=1;
    invariant(pthread_cond_broadcast(&m->changed));invariant(pthread_mutex_unlock(&m->mutex));
    release_job(j);return NULL;
}
static void creator_cleanup(void *arg){
    TLJob *j=arg;
    invariant(pthread_mutex_unlock(&j->monitor->mutex));
    release_job(j);
}
int tl_monitor_init(TLMonitor *m){
    if(!m)return EINVAL;
    int e=pthread_mutex_init(&m->mutex,NULL);if(e)return e;
    e=pthread_cond_init(&m->changed,NULL);if(e){invariant(pthread_mutex_destroy(&m->mutex));return e;}
    m->created=m->creator_waiting=m->release_worker=m->completed=0;
    m->live_refs=m->freed=m->sum=0;return 0;
}
int tl_monitor_destroy(TLMonitor *m){
    if(!m)return EINVAL;
    if(m->live_refs|| (m->created&&!m->freed))return EBUSY;
    int e=pthread_cond_destroy(&m->changed);if(e)return e;
    return pthread_mutex_destroy(&m->mutex);
}
void *tl_detached_creator(void *arg){
    TLCreator *c=arg;
    if(!c)return NULL;
    if(!c->monitor||c->length>16||(!c->input&&c->length)||c->wait_for_completion<0||c->wait_for_completion>1){c->error=EINVAL;return NULL;}
    int previous;invariant(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&previous));
    TLJob *j=malloc(sizeof *j);int e=j?0:ENOMEM;
    pthread_attr_t attr;int initialized=0;
    if(j){j->monitor=c->monitor;j->length=c->length;for(size_t i=0;i<c->length;i++)j->bytes[i]=c->input[i];
        e=pthread_attr_init(&attr);if(!e){initialized=1;e=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);}
    }
    TLMonitor *m=c->monitor;
    if(!e){
        invariant(pthread_mutex_lock(&m->mutex));m->live_refs=2;
        pthread_t thread;e=pthread_create(&thread,&attr,detached_worker,j);
        if(e)m->live_refs=0;else m->created=1;
        invariant(pthread_mutex_unlock(&m->mutex));
    }
    if(initialized)invariant(pthread_attr_destroy(&attr));
    if(e){free(j);c->error=e;invariant(pthread_setcancelstate(previous,NULL));return NULL;}
    c->error=0;
    pthread_cleanup_push(creator_cleanup,j);
    invariant(pthread_mutex_lock(&m->mutex));
    m->creator_waiting=1;invariant(pthread_cond_broadcast(&m->changed));
    invariant(pthread_setcancelstate(previous,NULL));
    while(c->wait_for_completion&&!m->completed)invariant(pthread_cond_wait(&m->changed,&m->mutex));
    invariant(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL));
    pthread_cleanup_pop(1);
    invariant(pthread_setcancelstate(previous,NULL));return NULL;
}
