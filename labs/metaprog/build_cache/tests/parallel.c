#define _POSIX_C_SOURCE 200809L
#include "build_cache.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
typedef struct {MpbPlan plan,output;MpbSchedule schedule;pthread_mutex_t mutex;pthread_cond_t changed;pthread_barrier_t first;unsigned active,peak,calls,fail,error;} Work;
static void *worker(void *context){Work *w=context;unsigned first=1;
    for(;;){MpbTicket ticket;int status;pthread_mutex_lock(&w->mutex);
        while((status=mpb_claim(&w->schedule,&ticket))==MPB_BUSY)pthread_cond_wait(&w->changed,&w->mutex);
        if(status!=MPB_OK){pthread_mutex_unlock(&w->mutex);return NULL;}
        ++w->active;++w->calls;if(w->active>w->peak)w->peak=w->active;pthread_mutex_unlock(&w->mutex);
        if(first){(void)pthread_barrier_wait(&w->first);first=0;}
        MpbEntry result=w->plan.entries[ticket.id];memset(result.artifact,0,sizeof result.artifact);int okay=mp_emit_table(&result.key,1,result.artifact,sizeof result.artifact,&result.artifact_length);
        pthread_mutex_lock(&w->mutex);--w->active;
        if(!okay)w->error=1;
        status=mpb_finish(&w->schedule,ticket,(!okay||(w->fail&&ticket.id==2))?NULL:&result);
        if(status!=MPB_OK&&status!=MPB_FAILED)w->error=1;
        pthread_cond_broadcast(&w->changed);pthread_mutex_unlock(&w->mutex);
    }
}
static int run(void){for(unsigned failure=0;failure<2;++failure){Work w={0};MpbNode n[3]={0};pthread_t threads[2];for(unsigned i=0;i<3;++i){n[i].id=i;n[i].revision=(int32_t)i;}n[1].dependencies=1;w.fail=failure;
    if(mpb_plan(n,3,0,&w.plan)!=MPB_OK||mpb_begin(&w.schedule,&w.plan)!=MPB_OK)return 1;
    if(pthread_mutex_init(&w.mutex,NULL)||pthread_cond_init(&w.changed,NULL)||pthread_barrier_init(&w.first,NULL,2))return 1;
    for(unsigned i=0;i<2;++i)if(pthread_create(&threads[i],NULL,worker,&w))return 1;
    for(unsigned i=0;i<2;++i)if(pthread_join(threads[i],NULL))return 1;
    if(w.error||w.active||w.peak!=2||w.schedule.running)return 1;
    int status=mpb_publish(&w.schedule,&w.output);
    if(failure){if(status!=MPB_FAILED||w.output.count)return 1;}else{if(status!=MPB_OK||w.output.count!=3||w.calls!=3)return 1;for(unsigned i=0;i<3;++i)if(!mpb_match(&w.output.entries[i],&w.plan.entries[i]))return 1;}
    if(pthread_barrier_destroy(&w.first)||pthread_cond_destroy(&w.changed)||pthread_mutex_destroy(&w.mutex))return 1;
 }
 puts("MPB_PARALLEL actual-two-workers exclusive-outputs dependency-wake failure-drain joined-publication");return 0;
}
int main(void){int status=run();if(status)fputs("MPO_CHECK MPB_PARALLEL contract failed\n",stderr);return status;}
