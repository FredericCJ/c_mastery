#include "transaction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BF_CHECK %d %s\n",__LINE__,#x);exit(1);}}while(0)
struct bf_backend{unsigned mask,acq_fail,commit_fail,fail[3],trace[64],count;};
static void event(struct bf_backend*b,unsigned value){CHECK(b->count<64);b->trace[b->count++]=value;}
static int acquire(void*p,unsigned i){struct bf_backend*b=p;CHECK(i<3&&!(b->mask&(1u<<i)));event(b,i);if(i==b->acq_fail)return 10+(int)i;b->mask|=1u<<i;return 0;}
static int release(void*p,unsigned i){struct bf_backend*b=p;CHECK(i<3&&(b->mask&(1u<<i)));CHECK(!(b->mask&~((1u<<(i+1))-1u)));event(b,10+i);if(b->fail[i]){--b->fail[i];return 20+(int)i;}b->mask&=~(1u<<i);return 0;}
static void nofail(void*p,unsigned i){CHECK(release(p,i)==0);}
static int commit(void*p){struct bf_backend*b=p;CHECK(b->mask==7);event(b,3);return b->commit_fail?13:0;}
static struct bf_tx_backend backend(struct bf_backend*b){struct bf_tx_backend r={acquire,release,commit,b};return r;}
static void styles(void){
 for(unsigned failure=0;failure<4;++failure){struct bf_backend reference={0};reference.acq_fail=failure;unsigned published=99;struct b2_backend rb={acquire,nofail,&reference};int expected=bf_construct(0,rb,&published);CHECK(expected==(failure==3?B2_OK:B2_FAILED)&&published==(failure==3?3u:99u));if(failure==3){nofail(&reference,2);nofail(&reference,1);nofail(&reference,0);}CHECK(reference.mask==0);
  for(unsigned style=1;style<4;++style){struct bf_backend b={0};b.acq_fail=failure;published=99;struct b2_backend bb={acquire,nofail,&b};CHECK(bf_construct(style,bb,&published)==expected&&published==(failure==3?3u:99u));if(failure==3){nofail(&b,2);nofail(&b,1);nofail(&b,0);}CHECK(!b.mask&&b.count==reference.count&&memcmp(b.trace,reference.trace,b.count*sizeof(unsigned))==0);}
 }
 struct bf_backend b={0};b.acq_fail=2;struct b2_backend bb={acquire,nofail,&b};unsigned out=99;CHECK(bf_construct(1,bb,&out)==B2_FAILED);unsigned first=b.count;CHECK(bf_construct(1,bb,&out)==B2_FAILED&&b.count==2*first&&out==99);
}
static void recovery(void){
 struct bf_backend b={0};b.acq_fail=2;b.fail[1]=1;struct bf_tx tx;bf_tx_init(&tx);unsigned out=99;CHECK(bf_tx_begin(&tx,backend(&b),&out)==BF_TX_RECOVERY&&out==99);CHECK(tx.primary==12&&tx.stage==2&&tx.cleanup_error==21&&tx.failed_cleanup==2&&tx.mask==3&&b.mask==3&&tx.state==BF_TX_PENDING);
 CHECK(bf_tx_begin(&tx,backend(&b),&out)==B2_INVALID&&b.count==4);CHECK(bf_tx_close(&tx)==B2_FAILED&&tx.state==BF_TX_DONE&&!b.mask&&tx.primary==12&&tx.cleanup_error==21&&tx.failed_cleanup==2);unsigned n=b.count;CHECK(bf_tx_close(&tx)==B2_FAILED&&b.count==n);
 for(unsigned i=0;i<3;++i){struct bf_backend c={0};c.acq_fail=3;c.fail[i]=1;bf_tx_init(&tx);out=99;CHECK(bf_tx_begin(&tx,backend(&c),&out)==B2_OK&&out==3);CHECK(bf_tx_close(&tx)==BF_TX_RECOVERY&&tx.primary==0&&tx.mask==((1u<<(i+1))-1u));CHECK(bf_tx_close(&tx)==B2_OK&&!c.mask&&tx.state==BF_TX_DONE);}
}
struct bf_registry{int *memory,*channel;unsigned registered,fail_compensation,events;};
static int reg_acquire(void*p,unsigned i){struct bf_registry*r=p;++r->events;if(i==0){r->memory=malloc(sizeof(int));if(!r->memory)return 10;*r->memory=7;}else if(i==1){r->channel=malloc(sizeof(int));if(!r->channel)return 11;*r->channel=11;}else{CHECK(r->memory&&r->channel&&!r->registered);r->registered=1;}return 0;}
static int reg_release(void*p,unsigned i){struct bf_registry*r=p;++r->events;if(i==2){CHECK(r->registered&&*r->memory==7&&*r->channel==11);if(r->fail_compensation){--r->fail_compensation;return 22;}r->registered=0;}else{CHECK(!r->registered);if(i==1){free(r->channel);r->channel=NULL;}else{free(r->memory);r->memory=NULL;}}return 0;}
static int reg_commit(void*p){struct bf_registry*r=p;CHECK(r->registered);++r->events;return 41;}
static void registration(void){
 /* Static dependency witness runs before heap-backed ownership controls. */
 struct bf_backend b={0};b.acq_fail=3;b.commit_fail=1;b.fail[2]=2;struct bf_tx tx;bf_tx_init(&tx);unsigned out=99;CHECK(bf_tx_begin(&tx,backend(&b),&out)==BF_TX_RECOVERY&&tx.mask==7&&b.mask==7&&out==99);CHECK(tx.primary==13&&tx.stage==3&&tx.cleanup_error==22);CHECK(bf_tx_close(&tx)==BF_TX_RECOVERY&&b.mask==7);CHECK(bf_tx_close(&tx)==B2_FAILED&&!b.mask);
 struct bf_registry r={0};r.fail_compensation=1;struct bf_tx_backend rb={reg_acquire,reg_release,reg_commit,&r};bf_tx_init(&tx);CHECK(bf_tx_begin(&tx,rb,&out)==BF_TX_RECOVERY&&r.registered&&r.memory&&r.channel&&tx.primary==41&&out==99);CHECK(bf_tx_close(&tx)==B2_FAILED&&!r.registered&&!r.memory&&!r.channel&&tx.primary==41&&tx.cleanup_error==22&&r.events==8);unsigned events=r.events;CHECK(bf_tx_close(&tx)==B2_FAILED&&r.events==events);
}
static void admission(void){struct bf_backend b={0};b.acq_fail=3;struct bf_tx tx;bf_tx_init(&tx);unsigned out=99;struct bf_tx_backend full=backend(&b),bad=full;bad.acquire=NULL;CHECK(bf_tx_begin(&tx,bad,&out)==B2_INVALID&&out==99&&!b.count);bad=full;bad.release=NULL;CHECK(bf_tx_begin(&tx,bad,&out)==B2_INVALID&&!b.count);bad=full;bad.commit=NULL;CHECK(bf_tx_begin(&tx,bad,&out)==B2_INVALID&&!b.count);CHECK(bf_tx_begin(&tx,full,NULL)==B2_INVALID&&!b.count);CHECK(bf_tx_close(&tx)==B2_INVALID);CHECK(bf_tx_begin(&tx,full,&out)==B2_OK);CHECK(bf_tx_begin(&tx,full,&out)==B2_INVALID&&b.count==4);CHECK(bf_tx_close(&tx)==B2_OK);
 for(unsigned style=0;style<5;++style){struct b2_backend old={NULL,nofail,&b};CHECK(bf_construct(style,old,&out)==B2_INVALID&&b.count==7);old.acquire=acquire;old.release=NULL;CHECK(bf_construct(style,old,&out)==B2_INVALID&&b.count==7);}
}
static void print_state(int result,unsigned published,const struct bf_tx*t,const struct bf_backend*b){printf("%d,%u,%u,%u,%u,%d,%d,%u,%u|",result,published,t->state,t->mask,t->stage,t->primary,t->cleanup_error,t->failed_cleanup,b->mask);}
int main(int argc,char**argv){CHECK(argc==2);
 if(!strcmp(argv[1],"oracle")){unsigned failure,commit_failure,f0,f1,f2;while(scanf("%u %u %u %u %u",&failure,&commit_failure,&f0,&f1,&f2)==5){struct bf_backend b={0};b.acq_fail=failure;b.commit_fail=commit_failure;b.fail[0]=f0;b.fail[1]=f1;b.fail[2]=f2;struct bf_tx tx;bf_tx_init(&tx);unsigned out=99;int first=bf_tx_begin(&tx,backend(&b),&out);print_state(first,out,&tx,&b);
  for(unsigned k=0;k<8;++k){int status=bf_tx_close(&tx);print_state(status,out,&tx,&b);}for(unsigned i=0;i<b.count;++i)printf("%u,",b.trace[i]);puts("");}return 0;}
 if(!strcmp(argv[1],"metrics")){printf("{\"transaction_bytes\":%zu,\"resources\":3,\"styles\":4,\"live_mask\":7}\n",sizeof(struct bf_tx));return 0;}
 if(!strcmp(argv[1],"styles"))styles();else if(!strcmp(argv[1],"recovery"))recovery();else if(!strcmp(argv[1],"registration"))registration();else if(!strcmp(argv[1],"admission"))admission();else CHECK(0);printf("BF %s validated\n",argv[1]);return 0;
}
