#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SESSION-CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static unsigned observer_calls,observer_errors,allocs,frees,fail_alloc;
static void observe(void *ctx,struct as_session *s) {
 (void)ctx;++observer_calls;
 if(!s->ready||s->state!=AS_NEW||s->count||!s->ledger->held[0]||s->ledger->held[1])++observer_errors;
 for(unsigned event=0;event<6;++event)if(as_event(s,(enum as_event)event)!=AS_INVALID)++observer_errors;
}
static void *allocate(void *ctx,size_t size,size_t alignment) {
 (void)ctx;if(fail_alloc||alignment>_Alignof(max_align_t))return 0;
 void *p=malloc(size);if(p)++allocs;return p;
}
static void release(void *ctx,void *p){(void)ctx;++frees;free(p);}
int main(void) {
 unsigned cases=0;
 /* Independent published relation: columns START,INC,STOP,SUSPEND,RESUME,CANCEL.
    -1 rejects, -2 increments with upper bound; other entries are next state. */
 const int relation[4][6]={{1,-1,-1,-1,-1,-1},{-1,-2,3,2,-1,3},
                         {-1,-1,3,-1,1,3},{-1,-1,3,-1,-1,3}};
 for(unsigned state=0;state<4;++state)for(unsigned count=0;count<16;++count)
  for(unsigned ev=0;ev<6;++ev) {
   struct ac_ledger l={0};struct as_session s;as_session_init(&s,&l);
   if(state!=AS_NEW) {
    CHECK(as_construct(&s,0,0,0)==AS_OK);
    if(state==AS_SUSPENDED)CHECK(as_event(&s,AS_SUSPEND)==AS_OK);
    if(state==AS_STOPPED)CHECK(as_event(&s,AS_STOP)==AS_OK);
   }
   s.count=count;struct as_session old=s;struct ac_ledger old_l=l;int next=relation[state][ev];
   enum as_status want=next==-1?AS_INVALID:(next==-2&&count==15?AS_FULL:AS_OK);
   CHECK(as_event(&s,(enum as_event)ev)==want);
   if(want!=AS_OK)CHECK(s.state==old.state&&s.count==old.count&&s.ready==old.ready&&l.count==old_l.count);
   else { CHECK(s.state==(enum as_state)(next==-2?(int)state:next));
    CHECK(s.count==(next==-2?count+1:(ev==AS_START?0:count))); }
   if(s.state==AS_ACTIVE||s.state==AS_SUSPENDED)CHECK(as_event(&s,AS_STOP)==AS_OK);
   CHECK(!l.held[0]&&!l.held[1]);++cases;
  }
 for(unsigned fail=0;fail<2;++fail) {
  struct ac_ledger l={0};struct as_session s;as_session_init(&s,&l);
  CHECK(as_construct(&s,fail,observe,0)==(fail?AS_FAILED:AS_OK));
  CHECK(!observer_errors && observer_calls==fail+1);
  if(!fail)CHECK(as_event(&s,AS_STOP)==AS_OK);
  CHECK(l.acquisitions[0]==1 && l.releases[0]==1);
  CHECK(l.acquisitions[1]==!fail && l.releases[1]==!fail);
  CHECK(as_event(&s,AS_STOP)==AS_OK);
 }
 struct ac_ledger l[3]={0};struct as_session s[3];struct as_backend b[3];
 for(unsigned i=0;i<3;++i) {as_session_init(s+i,l+i);CHECK(as_backend(&as_direct_ops,sizeof as_direct_ops,s+i,b+i)==AS_OK);CHECK(as_backend_run(b[i],AS_START)==AS_OK);}
 CHECK(b[0].run==b[1].run&&b[1].run==b[2].run);
 CHECK(as_backend_run(b[1],AS_INCREMENT)==AS_OK && s[0].count==0&&s[1].count==1&&s[2].count==0);
 struct as_extra extra={s,0};struct as_backend adapted;
 CHECK(as_backend(&as_adapted_ops,sizeof as_adapted_ops,&extra,&adapted)==AS_OK);
 CHECK(as_backend_run(adapted,AS_INCREMENT)==AS_OK && s[0].count==1&&extra.calls==1);
 CHECK(as_backend_flush(adapted)==AS_OK&&as_backend_flush(b[0])==AS_UNSUPPORTED);
 /* Per-instance copied table and shared const table have equivalent contracts. */
 struct as_ops_v1 copied[3]={as_direct_ops,as_direct_ops,as_direct_ops};
 for(unsigned i=0;i<3;++i) {
  struct as_backend copy;CHECK(as_backend(copied+i,sizeof copied[i],s+i,&copy)==AS_OK);
  CHECK(as_backend_run(copy,AS_STOP)==AS_OK);
 }
 struct as_backend sentinel={0,0,0};
 for(size_t n=0;n<sizeof(struct as_header);++n) {
  unsigned char *short_buffer=malloc(n?n:1);CHECK(short_buffer);
  CHECK(as_backend(short_buffer,n,s,&sentinel)==AS_INVALID && !sentinel.run);free(short_buffer);
 }
 struct as_ops_v2 bad=as_adapted_ops;bad.prefix.header.bytes=sizeof(struct as_header);
 CHECK(as_backend(&bad,sizeof bad,s,&sentinel)==AS_UNSUPPORTED&&!sentinel.run);
 bad=as_adapted_ops;bad.prefix.header.bytes=sizeof(struct as_ops_v1);
 CHECK(as_backend(&bad,sizeof bad,s,&sentinel)==AS_INVALID&&!sentinel.run);
 bad=as_adapted_ops;bad.prefix.header.version=3;
 CHECK(as_backend(&bad,sizeof bad,s,&sentinel)==AS_UNSUPPORTED);
 bad=as_adapted_ops;bad.flush=0;CHECK(as_backend(&bad,sizeof bad,&extra,&adapted)==AS_OK&&as_backend_flush(adapted)==AS_UNSUPPORTED);
 struct{struct as_ops_v2 known;unsigned extension[4];}future={as_adapted_ops,{1,2,3,4}};
 future.known.prefix.header.bytes=sizeof future;
 CHECK(as_backend(&future,sizeof future,&extra,&adapted)==AS_OK);
 future.known.prefix.header.flags=1;
 CHECK(as_backend(&future,sizeof future,&extra,&adapted)==AS_UNSUPPORTED);
 struct ac_ledger opaque_l={0};struct as_opaque *opaque=0;struct ac_allocate a={0,allocate,release};fail_alloc=1;
 CHECK(as_opaque_create(a,&opaque_l,&opaque)==AS_FAILED&&!opaque);fail_alloc=0;
 CHECK(as_opaque_create(a,&opaque_l,&opaque)==AS_OK);
 CHECK(as_opaque_event(opaque,AS_START)==AS_OK&&as_opaque_event(opaque,AS_INCREMENT)==AS_OK);
 unsigned value=99;CHECK(as_opaque_read(opaque,&value)==AS_OK&&value==1);
 CHECK(as_opaque_destroy(opaque)==AS_OK && allocs==1&&frees==1);
 /* Legal C assignment violates the exposed logical invariant, then admission rejects. */
 s[0].count=16;CHECK(as_event(s,AS_INCREMENT)==AS_INVALID);
 s[0].count=0;s[0].state=AS_ACTIVE;s[0].ready=1;
 CHECK(as_event(s,AS_INCREMENT)==AS_INVALID); /* Missing resources, valid C fields. */
 printf("SESSION-PASS:state_events=%u;constructor_callbacks=%u;instance=%zu;shared_table=%zu;copied_tables=%zu;typed_embedding=%zu\n",cases,observer_calls,sizeof(struct as_backend),sizeof as_direct_ops,sizeof copied,sizeof(struct as_session));
 return 0;
}
