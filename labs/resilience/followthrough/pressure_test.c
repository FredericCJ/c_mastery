#include "pressure.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#define CHECK(c) do{if(!(c)){fprintf(stderr,"RZ_CHECK: %s:%d %s\n",__FILE__,__LINE__,#c);exit(1);}}while(0)

typedef struct {atomic_uint made,dropped,ids[256];} Counter;
typedef struct {Counter *counter;unsigned id;} Payload;
static void drop(void *p) {Payload *v=p;CHECK(atomic_fetch_add(&v->counter->ids[v->id],1)==0);atomic_fetch_add(&v->counter->dropped,1);free(v);}
static RzpValue value(Counter *c,unsigned number) {
    unsigned id=atomic_fetch_add(&c->made,1);CHECK(id<256);Payload *p=malloc(sizeof *p);CHECK(p);*p=(Payload){c,id};return (RzpValue){number,p,drop};
}
static void *allocate(void *p,size_t n){(void)p;return malloc(n);}
static void deallocate(void *p,void *v){atomic_fetch_add((atomic_uint *)p,1);free(v);}
static RzfScope *scope(atomic_uint *freed,uint64_t deadline) {
    RzfScope *s=NULL;CHECK(rzf_scope_create(NULL,0,deadline,(RzfAllocator){freed,allocate,deallocate},&s)==RZ_OK);return s;
}
static void check_balance(RzpPump *p) {
    RzpSnapshot s;CHECK(rzp_snapshot(p,&s)==RZ_OK);
    CHECK(s.credits+s.queued+s.transport+s.leased+s.retired==4);
    CHECK(s.consumed>=s.returned && s.consumed-s.returned==4-s.credits);
    CHECK(s.pending<=2 && s.pending+s.queued+s.transport+s.leased<=6);
}
static void submit(RzpPump *p,RzfScope *s,Counter *c,unsigned n) {
    RzpValue v=value(c,n);RzpTicket ticket;int result=99;
    CHECK(rzp_begin(p,0,s,0,&v,&ticket)==RZ_OK && !v.owned);
    CHECK(rzp_poll(p,ticket,false,&result)==RZ_OK && result==RZ_OK);check_balance(p);
}
static void drain(RzpPump *p) {
    for(;;) {
        int forward=rzp_forward(p);CHECK(forward==RZ_OK || forward==RZ_FULL || forward==RZ_EMPTY);
        RzpValue v={0};RzpCredit credit;int status=rzp_receive(p,&v,&credit);
        if(status==RZ_OK)CHECK(rzp_release(p,credit)==RZ_OK);
        else CHECK(status==RZ_EMPTY || status==RZ_CLOSED);
        RzpSnapshot snapshot;CHECK(rzp_snapshot(p,&snapshot)==RZ_OK);check_balance(p);
        if(!snapshot.queued && !snapshot.transport && !snapshot.leased)break;
    }
}
static void policies(void) {
    for(unsigned policy=0;policy<5;++policy)for(unsigned count=0;count<=4;++count) {
        Counter c={0};RZScope s;CHECK(rz_scope_init(&s,NULL,0,10)==RZ_OK);RzpOwnedQueue q;rzp_owned_init(&q);
        for(unsigned i=0;i<count;++i){RzpValue v=value(&c,i+1);CHECK(rzp_owned_offer(&q,&s,0,RZ_BLOCK,&v)==RZ_OK && !v.owned);}
        RzpValue v=value(&c,9);int result=rzp_owned_offer(&q,&s,0,(RZPolicy)policy,&v);
        int want=count<4?RZ_OK:policy==0?RZ_PENDING:policy==1?RZ_FULL:policy==2?RZ_DROPPED:policy==3?RZ_OK:RZ_DEGRADED;
        CHECK(result==want);CHECK((v.owned!=NULL)==(result==RZ_PENDING));rzp_value_drop(&v);
        unsigned output[4],n=0;while(rzp_owned_pop(&q,&v)==RZ_OK){output[n++]=v.value;rzp_value_drop(&v);}
        CHECK(n==(count<4?count+1:4));
        if(count==4 && policy==3)CHECK(output[0]==2 && output[3]==9);
        if(count==4 && policy==4)CHECK(output[0]==1 && output[3]==9);
        CHECK(c.made==c.dropped);
        rzp_owned_close(&q);v=value(&c,10);CHECK(rzp_owned_offer(&q,&s,0,RZ_BLOCK,&v)==RZ_CLOSED && !v.owned);
        CHECK(c.made==c.dropped);
    }
}
typedef struct {RzpPump *pump;RzpTicket ticket;atomic_uint entered;int result;} Waiter;
static void *waiter(void *p) {Waiter *w=p;atomic_store(&w->entered,1);CHECK(rzp_poll(w->pump,w->ticket,true,&w->result)==RZ_OK);return NULL;}
static void waiting(RzpPump *p,unsigned producer) {
    for(;;) {
        pthread_mutex_lock(&p->mutex);bool wait=p->producer[producer].polling;pthread_mutex_unlock(&p->mutex);
        if(wait)return;
        sched_yield();
    }
}
static void duplex(void) {
    Counter counter={0};atomic_uint freed=0;RzfScope *s=scope(&freed,10);RzpPump endpoints[2];Waiter waiters[2];pthread_t threads[2];
    for(unsigned k=0;k<2;++k) {
        CHECK(rzp_init(&endpoints[k],1)==RZ_OK);
        for(unsigned i=0;i<4;++i)submit(&endpoints[k],s,&counter,10*k+i);
        RzpValue v=value(&counter,10*k+4);RzpTicket ticket={0};
        CHECK(rzp_begin(&endpoints[k],0,s,0,&v,&ticket)==RZ_PENDING && !v.owned);
        waiters[k]=(Waiter){&endpoints[k],ticket,0,99};
        CHECK(!pthread_create(&threads[k],NULL,waiter,&waiters[k]));waiting(&endpoints[k],0);
        int sentinel=99;CHECK(rzp_poll(&endpoints[k],ticket,false,&sentinel)==RZ_BUSY && sentinel==99);
    }
    /* Both endpoint actors now wait on sends; forwarding alone returns no capacity. */
    for(unsigned k=0;k<2;++k) {
        CHECK(rzp_forward(&endpoints[k])==RZ_OK);
        RzpSnapshot snapshot;CHECK(rzp_snapshot(&endpoints[k],&snapshot)==RZ_OK && snapshot.pending==1 && snapshot.credits==0);
    }
    /* A separate receive-progress agent breaks the wait-for cycle in both directions. */
    for(unsigned k=0;k<2;++k) {
        RzpValue v;RzpCredit credit;CHECK(rzp_receive(&endpoints[k],&v,&credit)==RZ_OK);
        CHECK(v.value==10*k);CHECK(rzp_release(&endpoints[k],credit)==RZ_OK);
        CHECK(!pthread_join(threads[k],NULL) && waiters[k].result==RZ_OK);
        CHECK(rzp_close(&endpoints[k])==RZ_OK);drain(&endpoints[k]);CHECK(rzp_destroy(&endpoints[k])==RZ_OK);
    }
    rzf_scope_release(&s);CHECK(freed==1 && counter.made==counter.dropped);
}
static void tests(void) {
    policies();duplex();Counter c={0};atomic_uint freed=0;RzfScope *s=scope(&freed,10);RzpPump p;
    CHECK(rzp_init(&p,1)==RZ_OK);
    for(unsigned i=1;i<=4;++i)submit(&p,s,&c,i);
    RzpValue a=value(&c,5),b=value(&c,6);RzpTicket ta={0},tb={0};
    CHECK(rzp_begin(&p,0,s,0,&a,&ta)==RZ_PENDING && !a.owned && ta.owner==&p);
    CHECK(rzp_begin(&p,1,s,0,&b,&tb)==RZ_PENDING && !b.owned && tb.owner==&p);
    RzpValue extra=value(&c,7);RzpTicket untouched={0};
    CHECK(rzp_begin(&p,0,s,0,&extra,&untouched)==RZ_BUSY && extra.owned && !untouched.owner);rzp_value_drop(&extra);
    CHECK(rzp_forward(&p)==RZ_OK && rzp_forward(&p)==RZ_FULL);check_balance(&p);
    Waiter wa={&p,ta,0,99},wb={&p,tb,0,99};pthread_t at,bt;
    CHECK(!pthread_create(&at,NULL,waiter,&wa));CHECK(!pthread_create(&bt,NULL,waiter,&wb));
    while(!atomic_load(&wa.entered) || !atomic_load(&wb.entered))sched_yield();
    RzpValue borrowed;RzpCredit credit;CHECK(rzp_receive(&p,&borrowed,&credit)==RZ_OK && borrowed.value==1);
    RzpSnapshot snapshot;CHECK(rzp_snapshot(&p,&snapshot)==RZ_OK && snapshot.credits==0 && snapshot.pending==2);
    CHECK(rzp_release(&p,credit)==RZ_OK);check_balance(&p);
    /* Cancelling the other registered operation releases it without a phantom send. */
    CHECK(rzp_cancel(&p,ta)==RZ_OK);CHECK(!pthread_join(at,NULL));CHECK(!pthread_join(bt,NULL));
    CHECK(wa.result==RZ_CANCELLED && wb.result==RZ_OK);
    CHECK(rzp_poll(&p,ta,false,&wa.result)==RZ_STALE);rzf_scope_release(&s);CHECK(freed==1);
    CHECK(rzp_close(&p)==RZ_OK && rzp_destroy(&p)==RZ_BUSY);drain(&p);
    CHECK(rzp_destroy(&p)==RZ_OK && c.made==c.dropped);

    s=scope(&freed,10);CHECK(rzp_init(&p,1)==RZ_OK);submit(&p,s,&c,1);
    CHECK(rzp_forward(&p)==RZ_OK && rzp_receive(&p,&borrowed,&credit)==RZ_OK);
    CHECK(rzp_release(&p,credit)==RZ_OK);CHECK(rzp_release(&p,credit)==RZ_STALE);check_balance(&p);
    submit(&p,s,&c,2);CHECK(rzp_release(&p,credit)==RZ_STALE);
    CHECK(rzp_close(&p)==RZ_OK);drain(&p);CHECK(rzp_destroy(&p)==RZ_OK);rzf_scope_release(&s);

    s=scope(&freed,2);CHECK(rzp_init(&p,1)==RZ_OK);
    for(unsigned i=0;i<4;++i)submit(&p,s,&c,i);
    a=value(&c,5);CHECK(rzp_begin(&p,0,s,0,&a,&ta)==RZ_PENDING);
    CHECK(rzp_tick(&p,2)==RZ_OK);int result=99;CHECK(rzp_poll(&p,ta,false,&result)==RZ_OK && result==RZ_EXPIRED);
    CHECK(rzp_tick(&p,1)==RZ_RANGE);CHECK(rzp_close(&p)==RZ_OK);drain(&p);CHECK(rzp_destroy(&p)==RZ_OK);rzf_scope_release(&s);

    s=scope(&freed,10);CHECK(rzp_init(&p,1)==RZ_OK);p.slots[0].generation=UINT_MAX-1;
    submit(&p,s,&c,1);CHECK(rzp_forward(&p)==RZ_OK && rzp_receive(&p,&borrowed,&credit)==RZ_OK && credit.generation==UINT_MAX);
    CHECK(rzp_release(&p,credit)==RZ_OK);check_balance(&p);CHECK(rzp_snapshot(&p,&snapshot)==RZ_OK && snapshot.credits==3 && snapshot.retired==1);
    p.producer[0].generation=UINT_MAX;a=value(&c,2);
    CHECK(rzp_begin(&p,0,s,0,&a,&ta)==RZ_FULL && a.owned);rzp_value_drop(&a);
    CHECK(rzp_close(&p)==RZ_OK && rzp_destroy(&p)==RZ_OK);rzf_scope_release(&s);CHECK(c.made==c.dropped);
    s=scope(&freed,10);CHECK(rzp_init(&p,1)==RZ_OK);p.consumed=p.returned=UINT64_MAX;
    a=value(&c,1);CHECK(rzp_begin(&p,0,s,0,&a,&ta)==RZ_FULL && !a.owned);
    CHECK(rzp_poll(&p,ta,false,&result)==RZ_OK && result==RZ_FULL);check_balance(&p);
    CHECK(rzp_close(&p)==RZ_OK && rzp_destroy(&p)==RZ_OK);rzf_scope_release(&s);CHECK(c.made==c.dropped);
}
static void policy_oracle(void) {
    unsigned policy;char word[32];
    while(scanf("%u %31s",&policy,word)==2) {
        Counter c={0};RZScope s;CHECK(rz_scope_init(&s,NULL,0,2)==RZ_OK);RzpOwnedQueue q;rzp_owned_init(&q);uint64_t now=0;
        for(size_t i=0;word[i];++i) {
            int status=RZ_OK;unsigned observed=99;RzpValue v={0};
            switch(word[i]) {
            case 'a':case 'b':v=value(&c,word[i]=='a'?1:9);status=rzp_owned_offer(&q,&s,now,(RZPolicy)policy,&v);rzp_value_drop(&v);break;
            case 'p':status=rzp_owned_pop(&q,&v);if(status==RZ_OK){observed=v.value;rzp_value_drop(&v);}break;
            case 't':now=2;break;
            case 'x':rz_cancel(&s);break;
            case 'c':rzp_owned_close(&q);break;
            default:CHECK(false);
            }
            printf("%d:%u:%u[",status,observed,atomic_load(&c.dropped));
            for(size_t j=0;j<q.core.count;++j)printf("%u,",q.values[(q.core.head+j)%4].value);
            printf("]; ");
        }
        RzpValue v;while(rzp_owned_pop(&q,&v)==RZ_OK)rzp_value_drop(&v);CHECK(c.made==c.dropped);puts("");
    }
}
static void pump_oracle(void) {
    unsigned capacity;char word[32];
    while(scanf("%u %31s",&capacity,word)==2) {
        Counter c={0};atomic_uint freed=0;RzfScope *s=scope(&freed,2);RzpPump p;CHECK(rzp_init(&p,capacity)==RZ_OK);
        RzpTicket tickets[2]={{0},{0}};RzpCredit credits[4],last={0};unsigned held=0,sequence=0;uint64_t now=0;
        for(size_t i=0;word[i];++i) {
            int status=RZ_OK;unsigned observed=99;RzpValue v={0};
            switch(word[i]) {
            case 'a':case 'b': {
                unsigned producer=word[i]=='a'?0:1;v=value(&c,++sequence);RzpTicket ticket={0};
                status=rzp_begin(&p,producer,s,now,&v,&ticket);rzp_value_drop(&v);
                if(ticket.owner) {
                    tickets[producer]=ticket;
                    if(status!=RZ_PENDING){int result;CHECK(rzp_poll(&p,ticket,false,&result)==RZ_OK && result==status);}
                }
                break;
            }
            case 'f':status=rzp_forward(&p);break;
            case 'r': {
                RzpCredit credit;status=rzp_receive(&p,&v,&credit);
                if(status==RZ_OK){CHECK(held<4);credits[held++]=credit;observed=v.value;}
                break;
            }
            case 'x':
                if(held){last=credits[0];for(unsigned j=1;j<held;++j)credits[j-1]=credits[j];--held;}
                status=rzp_release(&p,last);break;
            case 'c':status=rzp_close(&p);break;
            case 't':now=2;status=rzp_tick(&p,now);break;
            case 'p': {
                int results[2]={99,99};int statuses[2];
                for(unsigned j=0;j<2;++j)statuses[j]=rzp_poll(&p,tickets[j],false,&results[j]);
                status=statuses[0]*100+statuses[1];observed=(unsigned)(results[0]*100+results[1]);break;
            }
            default:CHECK(false);
            }
            RzpSnapshot snap;CHECK(rzp_snapshot(&p,&snap)==RZ_OK);check_balance(&p);
            printf("%d:%u:%u:%u:%u:%u:%u:%u:%"PRIu64":%"PRIu64":%u;",status,observed,snap.queued,snap.transport,snap.leased,
                   snap.pending,snap.terminal,snap.credits,snap.consumed,snap.returned,atomic_load(&c.dropped));
        }
        CHECK(rzp_close(&p)==RZ_OK);
        for(unsigned j=0;j<2;++j){int result;int status=rzp_poll(&p,tickets[j],false,&result);CHECK(status==RZ_OK || status==RZ_STALE);}
        for(unsigned j=0;j<held;++j)CHECK(rzp_release(&p,credits[j])==RZ_OK);
        drain(&p);CHECK(rzp_destroy(&p)==RZ_OK);rzf_scope_release(&s);CHECK(freed==1 && c.made==c.dropped);puts("");
    }
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    if(!strcmp(argv[1],"test"))tests();
    else if(!strcmp(argv[1],"policy"))policy_oracle();
    else if(!strcmp(argv[1],"pump"))pump_oracle();
    else return 2;
    return 0;
}
