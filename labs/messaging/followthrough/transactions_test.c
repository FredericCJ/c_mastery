#include "transactions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SC_CHECK: transactions:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct{atomic_uint allocations,frees,drops,calls;unsigned fail;} Memory;
typedef struct{Memory*memory;unsigned number;} Payload;
static void*allocate(void*ctx,size_t n){Memory*m=ctx;if(atomic_fetch_add(&m->calls,1)+1==m->fail)return NULL;void*p=malloc(n);if(p)atomic_fetch_add(&m->allocations,1);return p;}
static void deallocate(void*ctx,void*p){Memory*m=ctx;atomic_fetch_add(&m->frees,1);free(p);}
static void drop(void*ptr){Payload*p=ptr;atomic_fetch_add(&p->memory->drops,1);free(p);}
static MfoValue value(Memory*m,unsigned n){Payload*p=malloc(sizeof(*p));CHECK(p);*p=(Payload){m,n};return(MfoValue){p,drop};}
static unsigned number(const MfoValue*v){CHECK(v&&v->data);return((Payload*)v->data)->number;}
static void init(Memory*m,MftStore*s){atomic_init(&m->allocations,0);atomic_init(&m->frees,0);atomic_init(&m->drops,0);atomic_init(&m->calls,0);m->fail=0;CHECK(mft_init(s,(MflAllocator){m,allocate,deallocate})==MSG_OK);}
static void insert(MftStore*s,Memory*m,unsigned k,unsigned n){MftTxn t;CHECK(mft_begin(s,&t)==MSG_OK);MfoValue v=value(m,n);CHECK(mft_write(&t,k,&v)==MSG_OK && !v.data);CHECK(mft_commit(&t,NULL,NULL)==MSG_OK);}
typedef struct{MftStore*store;Memory*memory;unsigned calls,immediate;bool always;} Body;
static MsgStatus body(void*ctx,MftTxn*t){Body*b=ctx;++b->calls;++b->immediate;const MfoValue*seen=NULL;MsgStatus status=mft_read(t,0,&seen);if(status!=MSG_OK)return status;
    unsigned n=number(seen);CHECK(mft_log(t,n+100)==MSG_OK);MfoValue v=value(b->memory,n+1);CHECK(mft_write(t,0,&v)==MSG_OK && !v.data);
    if(b->calls==1 || b->always)insert(b->store,b->memory,0,n+10);
    return MSG_OK;}
static MsgStatus failure_body(void*ctx,MftTxn*t){(void)ctx;CHECK(mft_log(t,99)==MSG_OK);return MSG_FAILED;}
static void effects(void){Memory m;MftStore s;init(&m,&s);insert(&s,&m,0,1);Body b={&s,&m,0,0,false};unsigned attempts=99;
    CHECK(mft_run(&s,3,body,&b,&attempts)==MSG_OK && attempts==2 && b.calls==2 && b.immediate==2);
    CHECK(s.log_count==1 && s.log[0]==111);MftTxn t;const MfoValue*seen=NULL;CHECK(mft_begin(&s,&t)==MSG_OK);CHECK(mft_read(&t,0,&seen)==MSG_OK && number(seen)==12);mft_abort(&t);
    CHECK(mft_run(&s,3,failure_body,NULL,&attempts)==MSG_FAILED && attempts==1 && s.log_count==1);
    b.calls=b.immediate=0;b.always=true;CHECK(mft_run(&s,2,body,&b,&attempts)==MSG_CONFLICT && attempts==2 && s.log_count==1);
    CHECK(mft_run(&s,0,body,&b,&attempts)==MSG_INVALID && attempts==2);
    for(unsigned i=1;i<64;i++){CHECK(mft_begin(&s,&t)==MSG_OK);CHECK(mft_log(&t,i)==MSG_OK);CHECK(mft_commit(&t,NULL,NULL)==MSG_OK);}
    CHECK(s.log_count==64);CHECK(mft_begin(&s,&t)==MSG_OK);CHECK(mft_log(&t,999)==MSG_OK);MfoValue v=value(&m,999);CHECK(mft_write(&t,0,&v)==MSG_OK);CHECK(mft_commit(&t,NULL,NULL)==MSG_FULL && s.log_count==64);
    CHECK(mft_begin(&s,&t)==MSG_OK);CHECK(mft_read(&t,0,&seen)==MSG_OK && number(seen)==32);mft_abort(&t);
    CHECK(mft_begin(&s,&t)==MSG_OK);for(unsigned i=0;i<4;i++)CHECK(mft_log(&t,i)==MSG_OK);CHECK(mft_log(&t,4)==MSG_FULL);mft_abort(&t);
    CHECK(mft_destroy(&s)==MSG_OK && atomic_load(&m.allocations)==atomic_load(&m.frees));
}
static void*remove_node(void*ctx){MftStore*s=ctx;MftTxn t;CHECK(mft_begin(s,&t)==MSG_OK);CHECK(mft_remove(&t,0)==MSG_OK);CHECK(mft_commit(&t,NULL,NULL)==MSG_OK);return NULL;}
static void privatization(void){Memory m;MftStore s;init(&m,&s);insert(&s,&m,0,42);MftTxn a;CHECK(mft_begin(&s,&a)==MSG_OK);const MfoValue*borrow=NULL;CHECK(mft_read(&a,0,&borrow)==MSG_OK && number(borrow)==42);
    CHECK(mft_read(&a,4,&borrow)==MSG_INVALID && a.active && number(borrow)==42);CHECK(mft_read(&a,0,NULL)==MSG_INVALID && a.active);
    CHECK(mft_destroy(&s)==MSG_BUSY);pthread_t thread;CHECK(!pthread_create(&thread,NULL,remove_node,&s));CHECK(!pthread_join(thread,NULL));
    CHECK(atomic_load(&m.drops)==0);CHECK(number(borrow)==42);mft_abort(&a);CHECK(atomic_load(&m.drops)==1);
    CHECK(mft_begin(&s,&a)==MSG_OK);borrow=(const MfoValue*)&a;CHECK(mft_read(&a,0,&borrow)==MSG_OK && !borrow);
    MfoValue v=value(&m,7);m.fail=atomic_load(&m.calls)+1;CHECK(mft_write(&a,1,&v)==MSG_FAILED && v.data);m.fail=0;
    CHECK(mft_write(&a,1,&v)==MSG_OK && !v.data);CHECK(mft_read(&a,1,&borrow)==MSG_OK && number(borrow)==7);
    v=value(&m,8);CHECK(mft_write(&a,1,&v)==MSG_OK);CHECK(atomic_load(&m.drops)==2);mft_abort(&a);CHECK(atomic_load(&m.drops)==3);
    CHECK(mft_begin(&s,&a)==MSG_OK);v=value(&m,9);CHECK(mft_write(&a,2,&v)==MSG_OK);CHECK(mft_remove(&a,2)==MSG_OK);CHECK(atomic_load(&m.drops)==4);CHECK(mft_commit(&a,NULL,NULL)==MSG_OK);
    insert(&s,&m,0,11);CHECK(mft_begin(&s,&a)==MSG_OK);insert(&s,&m,0,12);borrow=(const MfoValue*)&a;CHECK(mft_read(&a,0,&borrow)==MSG_CONFLICT && borrow==(const MfoValue*)&a && !a.active);
    CHECK(mft_destroy(&s)==MSG_OK && atomic_load(&m.allocations)==atomic_load(&m.frees));
}
typedef struct{MsgStore*store;unsigned event,fail,counts[7],payload_conflicts;} Fault;
static bool phase(void*ctx,MftPhase p,unsigned key){Fault*f=ctx;++f->counts[p];++f->event;
    if(p==MFT_PAYLOAD){MsgTxn observer;unsigned out=99;msg_begin(f->store,&observer);CHECK(msg_read(&observer,key,&out)==MSG_CONFLICT && out==99);++f->payload_conflicts;}
    return f->event!=f->fail;
}
static void stage(MsgStore*s,MsgTxn*t){msg_begin(s,t);for(unsigned i=0;i<4;i++)CHECK(msg_write(t,i,i+1)==MSG_OK);}
typedef struct{MsgStore*store;} Increment;
static void*increment(void*ctx){Increment*w=ctx;unsigned committed=0,tries=0;while(committed<1000){CHECK(++tries<1000000);MsgTxn t;unsigned a,b;msg_begin(w->store,&t);
    if(msg_read(&t,0,&a)!=MSG_OK)continue;
    if(msg_read(&t,1,&b)!=MSG_OK)continue;
    CHECK(a==b);CHECK(msg_write(&t,0,a+1)==MSG_OK && msg_write(&t,1,b+1)==MSG_OK);MsgStatus status=mft_commit_phase(&t,NULL,NULL);CHECK(status==MSG_OK || status==MSG_CONFLICT);if(status==MSG_OK)++committed;
}return NULL;}
static void phases(void){
    for(unsigned cut=1;cut<=7;cut++){MsgStore s;MsgTxn t;msg_store_init(&s);stage(&s,&t);Fault f={.store=&s,.fail=cut};CHECK(mft_commit_phase(&t,phase,&f)==MSG_FAILED && !t.active);
        for(unsigned i=0;i<4;i++)CHECK(atomic_load(&s.cells[i].stamp)==0 && atomic_load(&s.cells[i].value)==0);
        CHECK(atomic_load(&s.clock)==(unsigned)(cut>=5));
    }
    for(unsigned cut=8;cut<=16;cut++){MsgStore s;MsgTxn t;msg_store_init(&s);stage(&s,&t);Fault f={.store=&s,.fail=cut};CHECK(mft_commit_phase(&t,phase,&f)==MSG_OK);
        CHECK(f.payload_conflicts==4 && f.counts[MFT_LOCKED]==4 && f.counts[MFT_CLOCK]==1 && f.counts[MFT_VALIDATED]==1 && f.counts[MFT_PREPUBLISH]==1 && f.counts[MFT_UNLOCKED]==4 && f.counts[MFT_FINISHED]==1);
        for(unsigned i=0;i<4;i++)CHECK(atomic_load(&s.cells[i].stamp)==2 && atomic_load(&s.cells[i].value)==i+1);
    }
    MsgStore s;MsgTxn t;msg_store_init(&s);stage(&s,&t);atomic_store(&s.cells[2].stamp,1);CHECK(mft_commit_phase(&t,NULL,NULL)==MSG_CONFLICT);
    CHECK(atomic_load(&s.cells[0].stamp)==0 && atomic_load(&s.cells[1].stamp)==0 && atomic_load(&s.cells[2].stamp)==1);
    atomic_store(&s.cells[2].stamp,0);atomic_store(&s.clock,UINT_MAX/2);stage(&s,&t);CHECK(mft_commit_phase(&t,NULL,NULL)==MSG_OVERFLOW);
    for(unsigned i=0;i<4;i++)CHECK(atomic_load(&s.cells[i].stamp)==0 && atomic_load(&s.cells[i].value)==0);
    msg_store_init(&s);msg_begin(&s,&t);Fault readonly={.store=&s};CHECK(mft_commit_phase(&t,phase,&readonly)==MSG_OK && atomic_load(&s.clock)==0 && readonly.counts[MFT_LOCKED]==0 && readonly.counts[MFT_CLOCK]==0 && readonly.counts[MFT_PAYLOAD]==0);
    Increment worker={&s};pthread_t workers[2];for(unsigned i=0;i<2;i++)CHECK(!pthread_create(&workers[i],NULL,increment,&worker));for(unsigned i=0;i<2;i++)CHECK(!pthread_join(workers[i],NULL));CHECK(atomic_load(&s.cells[0].value)==2000 && atomic_load(&s.cells[1].value)==2000);
    msg_store_init(&s);MsgTxn other;unsigned out;msg_begin(&s,&t);CHECK(msg_read(&t,0,&out)==MSG_OK);msg_begin(&s,&other);CHECK(msg_write(&other,0,4)==MSG_OK);CHECK(msg_commit(&other)==MSG_OK);CHECK(msg_write(&t,1,5)==MSG_OK);
    CHECK(mft_commit_phase(&t,NULL,NULL)==MSG_CONFLICT && atomic_load(&s.cells[1].value)==0 && atomic_load(&s.cells[1].stamp)==0);
}
static void oracle(void){unsigned initial,r0,r1,w0,w1;char word[16];while(scanf("%u %u %u %u %u %15s",&initial,&r0,&r1,&w0,&w1,word)==6){
    MsgStore s;msg_store_init(&s);atomic_store(&s.cells[0].value,initial);atomic_store(&s.cells[1].value,initial);MsgTxn t[2];unsigned steps[2]={0},read[2]={0},keys[2]={r0,r1},writes[2]={w0,w1};
    for(unsigned i=0;i<8;i++){unsigned k=(unsigned)(word[i]-'a'),observed=99;CHECK(k<2 && steps[k]<4);MsgStatus status=MSG_OK;
        if(steps[k]==0)msg_begin(&s,&t[k]);else if(steps[k]==1){status=msg_read(&t[k],keys[k],&observed);if(status==MSG_OK)read[k]=observed;}
        else if(steps[k]==2)status=msg_write(&t[k],writes[k],read[k]+1);else status=mft_commit_phase(&t[k],NULL,NULL);
        ++steps[k];printf("%d:%u,",status,observed);
    }
    printf("%u:%u:%u\n",atomic_load(&s.cells[0].value),atomic_load(&s.cells[1].value),atomic_load(&s.clock));
}}
int main(int argc,char**argv){CHECK(argc==2);if(!strcmp(argv[1],"effects"))effects();else if(!strcmp(argv[1],"privatization"))privatization();else if(!strcmp(argv[1],"phases"))phases();else if(!strcmp(argv[1],"oracle")){oracle();return 0;}else CHECK(false);puts("transactions passed");return 0;}
