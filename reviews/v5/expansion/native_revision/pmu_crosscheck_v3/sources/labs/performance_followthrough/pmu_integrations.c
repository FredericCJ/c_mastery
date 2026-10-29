#define _GNU_SOURCE
#include "pmu_integrations.h"
#include "events.h"
#include "collection.h"
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>

typedef struct { _Alignas(64) _Atomic uint64_t value; } PaddedCounter;
typedef struct {
    _Atomic uint64_t *counter;
    uint64_t iterations;
    pthread_barrier_t *start;
} Peer;
static void *increment(void *context){
    Peer *p=context;
    int result=pthread_barrier_wait(p->start);
    if(result!=0&&result!=PTHREAD_BARRIER_SERIAL_THREAD)return (void *)p;
    for(uint64_t i=0;i<p->iterations;++i)
        atomic_fetch_add_explicit(p->counter,1,memory_order_relaxed);
    return NULL;
}
uint64_t pf_concurrency_work(size_t n,unsigned variant,unsigned repetitions){
    _Alignas(64) _Atomic uint64_t adjacent[2];
    PaddedCounter padded[2];
    atomic_init(&adjacent[0],0);atomic_init(&adjacent[1],0);
    atomic_init(&padded[0].value,0);atomic_init(&padded[1].value,0);
    _Atomic uint64_t *mine=variant?&padded[0].value:&adjacent[0];
    _Atomic uint64_t *theirs=variant?&padded[1].value:&adjacent[1];
    if(!atomic_is_lock_free(mine)||!atomic_is_lock_free(theirs))return UINT64_MAX;
    pthread_barrier_t barrier;
    if(pthread_barrier_init(&barrier,NULL,2))return UINT64_MAX;
    Peer peer={theirs,(uint64_t)n*repetitions,&barrier};pthread_t thread;
    if(pthread_create(&thread,NULL,increment,&peer)){
        pthread_barrier_destroy(&barrier);return UINT64_MAX;
    }
    Peer local={mine,peer.iterations,&barrier};void *mine_result=increment(&local),*peer_result=NULL;
    int joined=pthread_join(thread,&peer_result);
    int destroyed=pthread_barrier_destroy(&barrier);
    if(mine_result||peer_result||joined||destroyed)return UINT64_MAX;
    return atomic_load_explicit(mine,memory_order_relaxed)+
           atomic_load_explicit(theirs,memory_order_relaxed);
}

static int drain(EvQueue *queue,EvSum *sum){
    EvEvent event;
    while(evq_pop(queue,&event)==EV_OK)
        if(evs_event(sum,&event)!=EV_OK)return 0;
    return 1;
}
static uint64_t consume(size_t chunk){
    static const unsigned char document[]="<sum><n>7</n><n>11</n></sum>";
    EvParser parser;EvQueue queue;EvSum sum;
    if(ev_init(&parser,EV_DEPTH)!=EV_OK)return UINT64_MAX;
    evq_init(&queue);evs_init(&sum);
    size_t offset=0;
    while(offset<sizeof document-1){
        size_t wanted=sizeof document-1-offset;if(wanted>chunk)wanted=chunk;
        size_t used=0;
        EvStatus status=ev_feed(&parser,document+offset,wanted,evq_offer,&queue,&used);
        if(used>wanted||(status!=EV_OK&&status!=EV_PAUSED))return UINT64_MAX;
        offset+=used;
        if(!drain(&queue,&sum))return UINT64_MAX;
    }
    for(unsigned attempts=0;attempts<8;++attempts){
        EvStatus status=ev_finish(&parser,evq_offer,&queue);
        if(!drain(&queue,&sum))return UINT64_MAX;
        if(status==EV_DONE)return sum.done&&sum.depth==0?sum.sum:UINT64_MAX;
        if(status!=EV_PAUSED)return UINT64_MAX;
    }
    return UINT64_MAX;
}
uint64_t pf_service_work(size_t n,unsigned variant,unsigned repetitions){
    if(!n||n>1024||variant>1)return UINT64_MAX;
    TdTimer timers[1024];size_t index[1024],position[1024];TcCollection queue;
    uint64_t checksum=0;
    for(unsigned repeat=0;repeat<repetitions;++repeat){
        if(tc_init(&queue,variant?TC_HEAP:TC_SCAN,timers,n,index,position,
                   NULL,NULL,NULL,0,0,100)!=0)return UINT64_MAX;
        for(size_t i=0;i<n;++i){
            TcToken token;
            if(tc_arm(&queue,i,(i*17)%n+1,0,0,&token)!=1)return UINT64_MAX;
            if(i%5==0&&tc_cancel(&queue,token)!=1)return UINT64_MAX;
        }
        TcToken token;TdDispatch dispatch;int status;
        while((status=tc_begin(&queue,n+1,&token,&dispatch))==1){
            uint64_t value=consume(token.slot%7+1);
            if(value!=18||dispatch.effects!=1||tc_finish(&queue,token)!=1)return UINT64_MAX;
            checksum+=value+token.slot;
        }
        if(status!=0)return UINT64_MAX;
    }
    return checksum;
}
