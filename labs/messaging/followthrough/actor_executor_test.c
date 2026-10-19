#include "actor_executor.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#define CHECK(x) do {if(!(x)) {fprintf(stderr,"SC_CHECK:%d:%s\n",__LINE__,#x);exit(1);}} while(0)
typedef struct State State;
typedef struct {unsigned id;State *state;} Payload;
struct State {MfeExecutor *executor;atomic_uint drops;unsigned calls,dead,depth,peak;unsigned seen[2000],last[2];uint64_t hash[2],reference[2];bool self,failed;};
static void drop(void *p) {Payload *v=p;atomic_fetch_add(&v->state->drops,1);free(v);}
static MfoValue make(State *s,unsigned id) {Payload *p=malloc(sizeof *p);CHECK(p);*p=(Payload){id,s};return (MfoValue){p,drop};}
static bool handle(void *p,const MfoValue *owner) {
    State *s=p;Payload *v=owner->data;++s->depth;if(s->depth>s->peak)s->peak=s->depth;++s->calls;
    if (s->self && v->id==1) {
        MfoValue second=make(s,2);uint64_t generation;CHECK(mfe_generation(s->executor,0,&generation)==MSG_OK);
        CHECK(mfe_send(s->executor,0,generation,&second,false)==MSG_OK && !second.data);
    }
    bool ok=v->id!=9999;
    if(!ok) s->failed=true; /* Observable synthetic effect survives failure. */
    if(v->id<2000) {CHECK(s->seen[v->id]++==0);s->hash[v->id/1000]=s->hash[v->id/1000]*33u+v->id;}
    --s->depth;return ok;
}
static void dead(void *p,uint64_t generation,MfoValue owner) {
    State *s=p;(void)generation;++s->dead;mfo_drop(&owner);
}
static void init(MfeExecutor *x,State *s) {
    *s=(State){.executor=x};atomic_init(&s->drops,0);void *contexts[2]={s,s};
    CHECK(mfe_init(x,4,4,handle,contexts,dead,s)==MSG_OK);
}
static void deliver(MfeExecutor *x,State *s,unsigned channel,unsigned id,uint64_t gen) {
    MfoValue value=make(s,id);CHECK(mfe_send(x,channel,gen,&value,false)==MSG_OK && !value.data && !value.drop);
}
typedef struct {MfeExecutor *x;State *s;unsigned channel;} Producer;
static void *producer(void *p) {
    Producer *a=p;uint64_t gen;CHECK(mfe_generation(a->x,a->channel,&gen)==MSG_OK);
    for(unsigned i=0;i<1000;i++) {
        MfoValue value=make(a->s,a->channel*1000+i);MsgStatus status;
        do {status=mfe_send(a->x,a->channel,gen,&value,true);if(status==MSG_FULL) {CHECK(value.data);sched_yield();}} while(status==MSG_FULL);
        CHECK(status==MSG_OK && !value.data);
    }
    CHECK(mfe_close(a->x,a->channel)==MSG_OK);return NULL;
}
typedef struct {MfeExecutor *executor;MfoValue value;MsgStatus status;} One;
static void *one_sender(void *p) {One *one=p;one->status=mfe_send(one->executor,0,1,&one->value,true);return NULL;}
static void wait_offer(MfeExecutor *x) {
    bool offered;
    do {pthread_mutex_lock(&x->input.hub.mutex);offered=x->input.hub.channel[0].offered;pthread_mutex_unlock(&x->input.hub.mutex);if(!offered)sched_yield();} while(!offered);
}
static bool oracle_handle(void *p,const MfoValue *owner) {State *s=p;Payload *v=owner->data;++s->calls;if(v->id==9999)s->failed=true;return v->id!=9999;}
static int oracle(void) {
    char word[16];
    while(fgets(word,sizeof word,stdin)) {
        MfeExecutor x;State s={.executor=&x};atomic_init(&s.drops,0);void *contexts[2]={&s,&s};
        CHECK(mfe_init(&x,2,2,oracle_handle,contexts,dead,&s)==MSG_OK);MfeEvent event={0,0,(MsgStatus)99};
        for(unsigned i=0;word[i] && word[i]!='\n';i++) {
            MsgStatus status=MSG_INVALID;
            switch(word[i]) {
            case 'a':case 'b':case 'f': {
                MfoValue owner=make(&s,word[i]=='f'?9999:i);
                status=mfe_send(&x,0,word[i]=='b'?2:1,&owner,false);mfo_drop(&owner);break;
            }
            case 'p':status=mfe_pump(&x,false,&event);break;
            case 'r':status=mfe_restart(&x,0);break;
            case 'c':CHECK(mfe_close(&x,0)==MSG_OK);status=mfe_close(&x,1);break;
            case 'x':mfe_cancel(&x);status=MSG_OK;break;
            default:return 2;
            }
            printf("%u:%u:%" PRIu64 ":%u:%u:%u:%u,",(unsigned)status,(unsigned)event.delivery,x.actor[0].actor.generation,s.calls,s.dead,atomic_load(&s.drops),(unsigned)x.actor[0].actor.alive);
        }
        CHECK(mfe_destroy(&x)==MSG_OK);printf("D%u\n",atomic_load(&s.drops));
    }
    return 0;
}
int main(int argc,char **argv) {
    if(argc==2 && !strcmp(argv[1],"oracle"))return oracle();
    MfeExecutor x;State state;MfeEvent event;init(&x,&state);state.self=true;
    deliver(&x,&state,0,1,1);CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_OK);
    CHECK(state.calls==1 && state.peak==1);CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_OK);
    CHECK(state.calls==2 && state.peak==1 && atomic_load(&state.drops)==2);
    for(unsigned i=3;i<30;i++) {deliver(&x,&state,0,i,1);CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_OK);}
    CHECK(atomic_load(&state.drops)==29);CHECK(mfe_destroy(&x)==MSG_OK);
    init(&x,&state);deliver(&x,&state,0,9999,1);deliver(&x,&state,0,41,1);
    CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_FAILED && state.failed);
    CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_CLOSED && state.dead==1);
    deliver(&x,&state,0,42,1);CHECK(mfe_restart(&x,0)==MSG_OK);
    CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_STALE && event.generation==1);
    CHECK(state.calls==1 && state.dead==2 && atomic_load(&state.drops)==3);
    deliver(&x,&state,0,43,2);CHECK(mfe_pump(&x,false,&event)==MSG_OK && event.delivery==MSG_OK && state.calls==2);
    CHECK(mfe_destroy(&x)==MSG_OK && atomic_load(&state.drops)==4);
    init(&x,&state);deliver(&x,&state,0,1,1);mfe_cancel(&x);event=(MfeEvent){9,13,MSG_INVALID};
    CHECK(mfe_pump(&x,false,&event)==MSG_CANCELLED && event.destination==9 && event.generation==13);
    MfoValue denied=make(&state,2);CHECK(mfe_send(&x,0,1,&denied,false)==MSG_CANCELLED && denied.data);mfo_drop(&denied);
    CHECK(mfe_destroy(&x)==MSG_OK && atomic_load(&state.drops)==2);
    init(&x,&state);
    for(unsigned i=0;i<8;i++) deliver(&x,&state,i/4,i,1);
    denied=make(&state,9);CHECK(mfe_send(&x,0,1,&denied,false)==MSG_FULL && denied.data);mfo_drop(&denied);
    CHECK(mfe_destroy(&x)==MSG_OK && atomic_load(&state.drops)==9);
    state=(State){.executor=&x};atomic_init(&state.drops,0);void *contexts[2]={&state,&state};
    CHECK(mfe_init(&x,0,0,handle,contexts,dead,&state)==MSG_OK);One one={&x,make(&state,1),MSG_INVALID};pthread_t sender;
    CHECK(pthread_create(&sender,NULL,one_sender,&one)==0);wait_offer(&x);
    CHECK(mfe_destroy(&x)==MSG_BUSY);mfe_cancel(&x);CHECK(pthread_join(sender,NULL)==0);
    CHECK(one.status==MSG_CANCELLED && one.value.data);mfo_drop(&one.value);
    CHECK(mfe_destroy(&x)==MSG_OK && atomic_load(&state.drops)==1);
    state=(State){.executor=&x};atomic_init(&state.drops,0);
    CHECK(mfe_init(&x,0,0,handle,contexts,dead,&state)==MSG_OK);one=(One){&x,make(&state,1),MSG_INVALID};
    CHECK(pthread_create(&sender,NULL,one_sender,&one)==0);wait_offer(&x);
    CHECK(mfe_pump(&x,true,&event)==MSG_OK && event.delivery==MSG_OK);
    CHECK(pthread_join(sender,NULL)==0 && one.status==MSG_OK && !one.value.data);
    CHECK(mfe_destroy(&x)==MSG_OK && atomic_load(&state.drops)==1);
    init(&x,&state);CHECK(mfa_destroy(&x.actor[0])==MSG_OK);x.actor[0].actor.generation=UINT64_MAX;x.published[0]=UINT64_MAX;
    CHECK(mfe_restart(&x,0)==MSG_OVERFLOW && x.actor[0].actor.generation==UINT64_MAX);CHECK(mfe_destroy(&x)==MSG_OK);
    init(&x,&state);Producer args[2]={{&x,&state,0},{&x,&state,1}};pthread_t threads[2];
    CHECK(pthread_create(&threads[0],NULL,producer,&args[0])==0 && pthread_create(&threads[1],NULL,producer,&args[1])==0);
    pthread_mutex_t reference;CHECK(pthread_mutex_init(&reference,NULL)==0);unsigned count[2]={0};
    for(;;) {
        MsgStatus status=mfe_pump(&x,true,&event);if(status==MSG_CLOSED) break;
        CHECK(status==MSG_OK && event.delivery==MSG_OK);unsigned k=event.destination;
        /* Same per-channel FIFO order; compare a mutex-protected shared state
           under the executor's observed cross-channel interleaving. */
        unsigned id=k*1000+count[k]++;
        CHECK(pthread_mutex_lock(&reference)==0);state.reference[k]=state.reference[k]*33u+id;CHECK(pthread_mutex_unlock(&reference)==0);
        CHECK(state.hash[k]==state.reference[k]);
    }
    CHECK(pthread_join(threads[0],NULL)==0 && pthread_join(threads[1],NULL)==0);
    CHECK(count[0]==1000 && count[1]==1000 && state.calls==2000 && state.peak==1 && state.dead==0 && atomic_load(&state.drops)==2000);
    CHECK(mfe_destroy(&x)==MSG_OK && pthread_mutex_destroy(&reference)==0);
    puts("channel/actor ownership, incarnation, self-enqueue and actual2000-message mutex-reference trace passed");return 0;
}
