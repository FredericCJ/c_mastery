#include "weak.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SC_CHECK: weak:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void sequential(void) {
    MfwDeque q;mfw_deque_init(&q);unsigned value=99;
    CHECK(mfw_pop(&q,&value)==MFW_EMPTY && value==99);
    CHECK(mfw_steal(&q,&value)==MFW_EMPTY && value==99);
    for(unsigned round=0;round<128;round++) {
        for(unsigned i=0;i<8;i++)CHECK(mfw_push(&q,round*8+i)==MFW_OK);
        size_t b=atomic_load(&q.bottom);CHECK(mfw_push(&q,99)==MFW_FULL);
        CHECK(atomic_load(&q.bottom)==b);
        for(unsigned i=0;i<4;i++){CHECK(mfw_pop(&q,&value)==MFW_OK);CHECK(value==round*8+7-i);}
        for(unsigned i=0;i<4;i++){CHECK(mfw_steal(&q,&value)==MFW_OK);CHECK(value==round*8+i);}
    }
    atomic_store(&q.top,SIZE_MAX-1);atomic_store(&q.bottom,SIZE_MAX-1);
    CHECK(mfw_push(&q,1)==MFW_OVERFLOW && atomic_load(&q.bottom)==SIZE_MAX-1);
    printf("weak_deque lock_free top=%d bottom=%d slot=%d\n",atomic_is_lock_free(&q.top),atomic_is_lock_free(&q.bottom),atomic_is_lock_free(&q.slots[0]));
}
typedef struct { MfwDeque q;atomic_bool done;atomic_uint seen[4000]; } Work;
static void record(Work*w,unsigned v){CHECK(v<4000);CHECK(atomic_fetch_add(&w->seen[v],1)==0);}
static void *stealer(void *arg) {
    Work*w=arg;for(;;){unsigned v;MfwStatus s=mfw_steal(&w->q,&v);
        if(s==MFW_OK)record(w,v);else CHECK(s==MFW_EMPTY || s==MFW_CONFLICT);
        if(s==MFW_EMPTY && atomic_load(&w->done))break;
        sched_yield();}
    return NULL;
}
static void workload(void) {
    Work*w=malloc(sizeof(*w));CHECK(w);mfw_deque_init(&w->q);atomic_init(&w->done,false);
    for(unsigned i=0;i<4000;i++)atomic_init(&w->seen[i],0);
    pthread_t threads[2];for(unsigned i=0;i<2;i++)CHECK(!pthread_create(&threads[i],NULL,stealer,w));
    for(unsigned i=0;i<4000;i++) {
        while(mfw_push(&w->q,i)==MFW_FULL)sched_yield();
        if(i%3==0){unsigned v;MfwStatus s=mfw_pop(&w->q,&v);if(s==MFW_OK)record(w,v);else CHECK(s==MFW_EMPTY || s==MFW_CONFLICT);}
    }
    atomic_store(&w->done,true);for(unsigned i=0;i<2;i++)CHECK(!pthread_join(threads[i],NULL));
    unsigned v;while(mfw_pop(&w->q,&v)==MFW_OK)record(w,v);
    for(unsigned i=0;i<4000;i++)CHECK(atomic_load(&w->seen[i])==1);
    free(w);
}
typedef struct {unsigned begin,end,value;MfwStatus status;} Event;
typedef struct {MfwDeque q;atomic_uint tick;Event events[3];} History;
typedef struct {History*h;unsigned id;} Call;
static void *call(void*arg){Call*c=arg;Event*e=&c->h->events[c->id];
    e->begin=atomic_fetch_add(&c->h->tick,1);e->value=99;
    e->status=c->id?mfw_steal(&c->h->q,&e->value):mfw_pop(&c->h->q,&e->value);
    e->end=atomic_fetch_add(&c->h->tick,1);return NULL;}
static bool legal(History*h,unsigned a,unsigned b,unsigned c){unsigned p[3]={a,b,c};
    unsigned values[2]={41,42},first=0,last=2;
    for(unsigned i=0;i<3;i++)for(unsigned j=i+1;j<3;j++)if(h->events[p[j]].end<h->events[p[i]].begin)return false;
    for(unsigned i=0;i<3;i++){Event*e=&h->events[p[i]];
        if(e->status==MFW_CONFLICT){if(e->value!=99)return false;continue;}
        if(e->status==MFW_EMPTY){if(first!=last || e->value!=99)return false;continue;}
        if(e->status!=MFW_OK || first==last)return false;
        unsigned wanted=p[i]?values[first++]:values[--last];if(e->value!=wanted)return false;
    }return true;}
static void histories(void){for(unsigned repeat=0;repeat<128;repeat++){
    History h;mfw_deque_init(&h.q);atomic_init(&h.tick,0);CHECK(mfw_push(&h.q,41)==MFW_OK);CHECK(mfw_push(&h.q,42)==MFW_OK);
    pthread_t threads[3];Call args[3];for(unsigned i=0;i<3;i++){args[i]=(Call){&h,i};CHECK(!pthread_create(&threads[i],NULL,call,&args[i]));}
    for(unsigned i=0;i<3;i++)CHECK(!pthread_join(threads[i],NULL));
    bool found=false;
    for(unsigned a=0;a<3;a++)for(unsigned b=0;b<3;b++)for(unsigned c=0;c<3;c++)if(a!=b && b!=c && a!=c)found|=legal(&h,a,b,c);
    CHECK(found);
}}
typedef struct {MfwSequence s;atomic_bool done;} SequenceWork;
static void *write_values(void*arg){SequenceWork*w=arg;for(unsigned i=1;i<=10000;i++)CHECK(mfw_write(&w->s,i,2*i)==MFW_OK);atomic_store(&w->done,true);return NULL;}
static void sequences(void){MfwSequence s;mfw_sequence_init(&s);unsigned out[2]={99,99};
    CHECK(mfw_read(&s,0,out)==MFW_BUSY && out[0]==99 && out[1]==99);
    atomic_store(&s.sequence,1);CHECK(mfw_read(&s,3,out)==MFW_BUSY && out[0]==99 && out[1]==99);
    CHECK(mfw_write(&s,1,2)==MFW_BUSY);atomic_store(&s.sequence,0);
    CHECK(mfw_write(&s,7,14)==MFW_OK);CHECK(mfw_read(&s,1,out)==MFW_OK && out[0]==7 && out[1]==14);
    atomic_store(&s.sequence,UINT_MAX-1);CHECK(mfw_write(&s,3,6)==MFW_OVERFLOW);
    CHECK(atomic_load(&s.sequence)==UINT_MAX-1 && atomic_load(&s.first)==7 && atomic_load(&s.second)==14);
    SequenceWork work;mfw_sequence_init(&work.s);atomic_init(&work.done,false);pthread_t thread;CHECK(!pthread_create(&thread,NULL,write_values,&work));
    do{out[0]=out[1]=99;MfwStatus result=mfw_read(&work.s,3,out);if(result==MFW_OK)CHECK(out[1]==2*out[0]);else CHECK(result==MFW_BUSY && out[0]==99 && out[1]==99);}while(!atomic_load(&work.done));
    CHECK(!pthread_join(thread,NULL));CHECK(mfw_read(&work.s,1,out)==MFW_OK && out[0]==10000 && out[1]==20000);
    printf("weak_sequence lock_free sequence=%d payload=%d\n",atomic_is_lock_free(&s.sequence),atomic_is_lock_free(&s.first));
}
int main(int argc,char**argv){CHECK(argc==2);if(!strcmp(argv[1],"deque")){sequential();histories();workload();}else if(!strcmp(argv[1],"sequence"))sequences();else CHECK(false);puts("weak native passed");return 0;}
