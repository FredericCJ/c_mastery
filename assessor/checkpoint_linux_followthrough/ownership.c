#define _POSIX_C_SOURCE 200809L
#include "checkpoint_async.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"P5_OWNER_CHECK line=%d %s\n",__LINE__,#x);exit(1);}}while(0)
enum{ITEMS=64};
struct p5_copies{struct cpa_queue q;pthread_mutex_t results;unsigned seen[ITEMS],sum;};
static void *copied(void *arg){
 struct p5_copies *c=arg;
 for(;;){struct cpa_handle h;struct cp_message m;enum cpa_status s=cpa_take(&c->q,&h,&m,1);if(s==CPA_CLOSED)break;
  CHECK(s==CPA_OK && m.sequence<ITEMS && m.length==2 && m.payload[0]==m.sequence && m.payload[1]==(unsigned char)(m.sequence^0xa5));
  /* Decoder/input is not shared. Worker owns a local mutable copy only. */
  CHECK(!pthread_mutex_lock(&c->results));c->sum+=m.payload[0];++c->seen[m.sequence];CHECK(!pthread_mutex_unlock(&c->results));
  CHECK(cpa_finish(&c->q,h,m.sequence==17)==CPA_OK);
 }return 0;
}
struct p5_shared{pthread_mutex_t mutex;pthread_cond_t changed;struct cp_message message;unsigned seen[ITEMS],sum;int ready,closed;};
static void *mutable(void *arg){
 struct p5_shared *s=arg;CHECK(!pthread_mutex_lock(&s->mutex));
 for(;;){while(!s->ready && !s->closed)CHECK(!pthread_cond_wait(&s->changed,&s->mutex));if(!s->ready)break;
  CHECK(s->message.sequence<ITEMS && s->message.payload[0]==s->message.sequence && s->message.payload[1]==(unsigned char)(s->message.sequence^0xa5));
  s->sum+=s->message.payload[0];++s->seen[s->message.sequence];memset(s->message.payload,0,sizeof s->message.payload);s->ready=0;CHECK(!pthread_cond_broadcast(&s->changed));
 }CHECK(!pthread_mutex_unlock(&s->mutex));return 0;
}
int main(void){
 struct p5_copies c={0};CHECK(cpa_init(&c.q,128)==CPA_OK && !pthread_mutex_init(&c.results,0));pthread_t threads[2];
 for(unsigned i=0;i<2;i++)CHECK(!pthread_create(&threads[i],0,copied,&c));
 for(unsigned i=0;i<ITEMS;i++){struct cp_message m={.sequence=(uint16_t)i,.length=2,.payload={(unsigned char)i,(unsigned char)(i^0xa5)}};CHECK(cpa_offer(&c.q,&m,1)==CPA_OK);memset(&m,0,sizeof m);}
 enum cpa_status status=cpa_close(&c.q,0);CHECK(status==CPA_PENDING || status==CPA_CLOSED);
 for(unsigned i=0;i<2;i++)CHECK(!pthread_join(threads[i],0));
 CHECK(c.sum==2016 && c.q.accepted==ITEMS && c.q.completed==ITEMS && c.q.releases==ITEMS && c.q.report_failures==1);
 for(unsigned i=0;i<ITEMS;i++)CHECK(c.seen[i]==1);
 CHECK(cpa_destroy(&c.q)==CPA_OK && !pthread_mutex_destroy(&c.results));
 struct p5_shared s={0};CHECK(!pthread_mutex_init(&s.mutex,0) && !pthread_cond_init(&s.changed,0));CHECK(!pthread_create(&threads[0],0,mutable,&s));
 for(unsigned i=0;i<ITEMS;i++){
  CHECK(!pthread_mutex_lock(&s.mutex));while(s.ready)CHECK(!pthread_cond_wait(&s.changed,&s.mutex));
  s.message=(struct cp_message){.sequence=(uint16_t)i,.length=2,.payload={(unsigned char)i,(unsigned char)(i^0xa5)}};s.ready=1;CHECK(!pthread_cond_broadcast(&s.changed));CHECK(!pthread_mutex_unlock(&s.mutex));
 }
 CHECK(!pthread_mutex_lock(&s.mutex));s.closed=1;CHECK(!pthread_cond_broadcast(&s.changed));CHECK(!pthread_mutex_unlock(&s.mutex));CHECK(!pthread_join(threads[0],0));
 CHECK(s.sum==2016);for(unsigned i=0;i<ITEMS;i++)CHECK(s.seen[i]==1);
 CHECK(!pthread_cond_destroy(&s.changed) && !pthread_mutex_destroy(&s.mutex));
 puts("P5_OWNER_PASS same_workload=64 sum=2016 copy_workers=2 mutable_workers=1 no_performance_equivalence_claim");
}
