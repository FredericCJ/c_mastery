#include "object_runtime_rc_concurrent.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"OR_CHECK:%d:%s\n",__LINE__,#c); return 1; } } while(0)
static const struct orr_handle nil={0,0};
static int zero(void) {
  struct orr_heap h,before;struct orr_handle a,b,out;uint16_t dead;unsigned i;
  orr_init(&h);CHECK(orr_new(&h,&a)==OR_OK && orr_new(&h,&b)==OR_OK);
  CHECK(orr_store(&h,a,0,b,0,&dead)==OR_OK && orr_store(&h,a,1,b,0,&dead)==OR_OK);
  CHECK(orr_release(&h,b,0,&dead)==OR_OK && !dead && h.counts.refs[1]==2);
  CHECK(orr_release(&h,a,0,&dead)==OR_OK && dead==3 && orr_invariant(&h));
  CHECK(orr_retain(&h,b)==OR_INVALID);
  orr_init(&h);CHECK(orr_new(&h,&a)==OR_OK);
  CHECK(orr_store(&h,a,0,a,0,&dead)==OR_OK && orr_release(&h,a,0,&dead)==OR_OK && !dead);
  before=h;dead=777;
  CHECK(orr_release(&h,a,0,&dead)==OR_STATE && dead==777 && !memcmp(&h,&before,sizeof h));
  CHECK(orr_store(&h,a,0,nil,0,&dead)==OR_OK && dead==1 && orr_invariant(&h));
  orr_init(&h);CHECK(orr_new(&h,&a)==OR_OK);
  for(i=1;i<255;i++)CHECK(orr_retain(&h,a)==OR_OK);
  before=h;CHECK(orr_retain(&h,a)==OR_FULL && !memcmp(&h,&before,sizeof h));
  for(i=0;i<255;i++)CHECK(orr_release(&h,a,0,&dead)==OR_OK && dead==(i==254?1:0));
  CHECK(orr_invariant(&h));
  orr_init(&h);
  for(i=0;i<48;i++){CHECK(orr_new(&h,&a)==OR_OK);CHECK(orr_release(&h,a,0,&dead)==OR_OK && dead);}
  out=(struct orr_handle){99,99};before=h;
  CHECK(orr_new(&h,&out)==OR_FULL && out.slot==99 && !memcmp(&h,&before,sizeof h));
  CHECK(orr_close(&h)==OR_OK && orr_close(&h)==OR_STATE);
  orr_init(&h);CHECK(orr_new(&h,&a)==OR_OK);
  while(h.revision<65535)CHECK(orr_store(&h,a,0,nil,0,&dead)==OR_OK);
  before=h;dead=777;
  CHECK(orr_release(&h,a,0,&dead)==OR_FULL && dead==777 && !memcmp(&h,&before,sizeof h));
  CHECK(orr_close(&h)==OR_OK); /* Explicit whole-owner terminal retirement. */
  return 0;
}
static int deferred(void) {
  struct orr_heap h,before,other;struct orr_handle a,b;struct orr_trial t;uint16_t dead;unsigned i,n;
  orr_init(&h);CHECK(orr_new(&h,&a)==OR_OK && orr_new(&h,&b)==OR_OK);
  CHECK(orr_store(&h,b,0,a,0,&dead)==OR_OK);
  for(i=0;i<8;i++)CHECK(orr_retain(&h,a)==OR_OK);
  for(i=0;i<8;i++)CHECK(orr_release(&h,a,1,&dead)==OR_OK && !dead && orr_invariant(&h));
  before=h;dead=777;
  CHECK(orr_release(&h,a,1,&dead)==OR_FULL && dead==777 && !memcmp(&h,&before,sizeof h));
  CHECK(orr_store(&h,b,0,b,1,&dead)==OR_FULL && dead==777 && !memcmp(&h,&before,sizeof h));
  CHECK(orr_flush(&h,0,&n,&dead)==OR_OK && !n && !dead && !memcmp(&h,&before,sizeof h));
  CHECK(orr_flush(&h,1,&n,&dead)==OR_OK && n==1 && !dead);
  CHECK(orr_release(&h,a,1,&dead)==OR_OK && !dead && h.length==8 && !h.roots[0]);
  CHECK(orr_flush(&h,99,&n,&dead)==OR_DONE && n==8 && !dead && h.counts.refs[0]==1);
  CHECK(orr_store(&h,b,0,nil,0,&dead)==OR_OK && dead==1);
  CHECK(orr_release(&h,b,0,&dead)==OR_OK && dead==2 && orr_invariant(&h));

  orr_init(&h);CHECK(orr_new(&h,&a)==OR_OK && orr_new(&h,&b)==OR_OK);
  CHECK(orr_store(&h,a,0,b,0,&dead)==OR_OK && orr_store(&h,b,0,a,0,&dead)==OR_OK);
  CHECK(orr_release(&h,a,1,&dead)==OR_OK && orr_release(&h,b,1,&dead)==OR_OK);
  CHECK(h.counts.refs[0]==2 && h.counts.refs[1]==2 && orr_invariant(&h));
  CHECK(orr_flush(&h,1,&n,&dead)==OR_OK && !dead && h.candidates==1);
  CHECK(orr_flush(&h,1,&n,&dead)==OR_DONE && !dead && h.candidates==3);
  CHECK(orr_prepare(&h,&t)==OR_OK && t.dead==3 && h.counts.live==3);
  orr_init(&other);before=other;dead=777;
  CHECK(orr_commit(&other,&t,&dead)==OR_STATE && dead==777 && !memcmp(&other,&before,sizeof other));
  CHECK(orr_commit(&h,&t,&dead)==OR_OK && dead==3 && orr_invariant(&h));
  dead=777;CHECK(orr_commit(&h,&t,&dead)==OR_STATE && dead==777);
  return 0;
}
struct or_worker { struct orr_concurrent *owner;struct orr_handle object;int release,stress,failed; };
static void *worker(void *opaque) {
  struct or_worker *w=opaque;struct orr_request q={0};struct orr_response r;unsigned i;
  q.object=w->object;
  for(i=0;i<(w->stress?1000u:1u);i++) {
    q.operation=w->release?ORR_RELEASE:ORR_RETAIN;
    if(orrc_call(w->owner,&q,&r)!=OR_OK)w->failed=1;
    if(w->stress){q.operation=ORR_RELEASE;if(orrc_call(w->owner,&q,&r)!=OR_OK)w->failed=1;}
  }
  return NULL;
}
static int concurrent(void) {
  struct orr_concurrent h;struct orr_request q={0};struct orr_response r,before;struct orr_handle a,b;
  struct orr_trial t;pthread_t thread;struct or_worker w;unsigned i;enum or_status s;
  CHECK(orrc_init(&h)==OR_OK);q.operation=ORR_NEW;CHECK(orrc_call(&h,&q,&r)==OR_OK);a=r.object;
  CHECK(orrc_call(&h,&q,&r)==OR_OK);b=r.object;
  q=(struct orr_request){.operation=ORR_STORE,.object=a,.target=b};CHECK(orrc_call(&h,&q,&r)==OR_OK);
  q.object=b;q.target=a;CHECK(orrc_call(&h,&q,&r)==OR_OK);
  q=(struct orr_request){.operation=ORR_RELEASE,.object=a};CHECK(orrc_call(&h,&q,&r)==OR_OK);
  q.object=b;CHECK(orrc_call(&h,&q,&r)==OR_OK);
  q=(struct orr_request){.operation=ORR_PREPARE};CHECK(orrc_call(&h,&q,&r)==OR_OK && r.trial.dead==3);t=r.trial;
  w=(struct or_worker){.owner=&h,.object=a};CHECK(!pthread_create(&thread,NULL,worker,&w));CHECK(!pthread_join(thread,NULL) && !w.failed);
  q=(struct orr_request){.operation=ORR_COMMIT,.trial=t};memset(&r,0x33,sizeof r);before=r;
  CHECK(orrc_call(&h,&q,&r)==OR_STATE && !memcmp(&r,&before,sizeof r));
  q.operation=ORR_INSPECT;CHECK(orrc_call(&h,&q,&r)==OR_OK && r.invariant && r.live==3);
  q.operation=ORR_PREPARE;CHECK(orrc_call(&h,&q,&r)==OR_OK && r.trial.dead==0);
  q.operation=ORR_COMMIT;q.trial=r.trial;CHECK(orrc_call(&h,&q,&r)==OR_OK && !r.dead);
  w.release=1;CHECK(!pthread_create(&thread,NULL,worker,&w));CHECK(!pthread_join(thread,NULL) && !w.failed);
  q.operation=ORR_PREPARE;CHECK(orrc_call(&h,&q,&r)==OR_OK && r.trial.dead==3);
  q.operation=ORR_COMMIT;q.trial=r.trial;CHECK(orrc_call(&h,&q,&r)==OR_OK && r.dead==3);
  CHECK(orrc_close(&h)==OR_OK);

  CHECK(orrc_init(&h)==OR_OK);q=(struct orr_request){.operation=ORR_NEW};CHECK(orrc_call(&h,&q,&r)==OR_OK);a=r.object;
  q=(struct orr_request){.operation=ORR_RELEASE,.object=a,.defer=1};CHECK(orrc_call(&h,&q,&r)==OR_OK);
  q.operation=ORR_PREPARE;memset(&r,0x33,sizeof r);before=r;
  CHECK(orrc_call(&h,&q,&r)==OR_STATE && !memcmp(&r,&before,sizeof r));
  q=(struct orr_request){.operation=ORR_FLUSH,.budget=8};CHECK(orrc_call(&h,&q,&r)==OR_DONE && r.dead==1);
  CHECK(orrc_close(&h)==OR_OK);

  CHECK(orrc_init(&h)==OR_OK);q=(struct orr_request){.operation=ORR_NEW};CHECK(orrc_call(&h,&q,&r)==OR_OK);a=r.object;
  w=(struct or_worker){.owner=&h,.object=a,.stress=1};CHECK(!pthread_create(&thread,NULL,worker,&w));
  for(i=0;i<1000;i++) {
    q=(struct orr_request){.operation=ORR_PREPARE};CHECK(orrc_call(&h,&q,&r)==OR_OK && !r.trial.dead);
    q.operation=ORR_COMMIT;q.trial=r.trial;s=orrc_call(&h,&q,&r);CHECK(s==OR_OK || s==OR_STATE);
  }
  CHECK(!pthread_join(thread,NULL) && !w.failed);
  q=(struct orr_request){.operation=ORR_INSPECT};CHECK(orrc_call(&h,&q,&r)==OR_OK && r.invariant && r.live==1);
  q=(struct orr_request){.operation=ORR_RELEASE,.object=a};CHECK(orrc_call(&h,&q,&r)==OR_OK && r.dead==1);
  CHECK(orrc_close(&h)==OR_OK);return 0;
}
static int oracle(void) {
  unsigned graph,mask,mode,i,j,code,n;struct orr_heap h;struct orr_handle a[3];struct orr_trial t;uint16_t dead;
  for(graph=0;graph<4096;graph++)for(mask=0;mask<8;mask++)for(mode=0;mode<2;mode++) {
    orr_init(&h);for(i=0;i<3;i++)CHECK(orr_new(&h,&a[i])==OR_OK);
    code=graph;
    for(i=0;i<3;i++)for(j=0;j<2;j++) { unsigned target=code%4;code/=4;CHECK(orr_store(&h,a[i],j,target?a[target-1]:nil,0,&dead)==OR_OK); }
    for(i=0;i<3;i++)if(!(mask>>i&1))CHECK(orr_release(&h,a[i],(int)mode,&dead)==OR_OK);
    CHECK(orr_flush(&h,8,&n,&dead)==OR_DONE && orr_prepare(&h,&t)==OR_OK);
    CHECK(orr_commit(&h,&t,&dead)==OR_OK && orr_invariant(&h));
    printf("R %u %u %u %u %u %u %u\n",graph,mask,mode,h.counts.live,h.counts.refs[0],h.counts.refs[1],h.counts.refs[2]);
  }
  return 0;
}
int main(int argc,char **argv) {
  if(argc!=2)return 2;
  if(!strcmp(argv[1],"zero"))return zero();
  if(!strcmp(argv[1],"deferred"))return deferred();
  if(!strcmp(argv[1],"concurrent"))return concurrent();
  if(!strcmp(argv[1],"oracle"))return oracle();
  return 2;
}
