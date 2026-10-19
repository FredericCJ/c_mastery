#define _POSIX_C_SOURCE 200809L
#include "checkpoint_async.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"P5_WAIT_CHECK line=%d %s\n",__LINE__,#x);exit(1);}}while(0)
int __real_pthread_cond_wait(pthread_cond_t *,pthread_mutex_t *);
static CPAQueue q;static pthread_t victim;static atomic_int ready,armed,stolen;
static pthread_mutex_t gate=PTHREAD_MUTEX_INITIALIZER;static pthread_cond_t changed=PTHREAD_COND_INITIALIZER;static int stage;
/* Valid condition-wait wrapper scheduling seam: after the real wait reacquires,
   release/reacquire once more before returning. The original predicate checker
   must tolerate a consumer taking the item in that real unlocked interval.
   This is not claimed as a trace inside a particular libc implementation. */
int __wrap_pthread_cond_wait(pthread_cond_t *cond,pthread_mutex_t *mutex){
 int r=__real_pthread_cond_wait(cond,mutex);
 if(!r && cond==&q.changed && atomic_load(&ready) && pthread_equal(pthread_self(),victim) && q.count && atomic_exchange(&armed,0)){
  CHECK(!pthread_mutex_unlock(mutex));CHECK(!pthread_mutex_lock(&gate));stage=1;CHECK(!pthread_cond_broadcast(&changed));
  while(stage!=2)CHECK(!__real_pthread_cond_wait(&changed,&gate));
  CHECK(!pthread_mutex_unlock(&gate));CHECK(!pthread_mutex_lock(mutex));
 }
 return r;
}
static void *waiter(void *arg){
 (void)arg;victim=pthread_self();atomic_store(&ready,1);CPAHandle h;CPMessage m;
 CPAStatus s=cpa_take(&q,&h,&m,1);CHECK(s==CPA_CLOSED);return 0;
}
static void *thief(void *arg){
 (void)arg;CHECK(!pthread_mutex_lock(&gate));while(stage!=1)CHECK(!pthread_cond_wait(&changed,&gate));CHECK(!pthread_mutex_unlock(&gate));
 CPAHandle h;CPMessage m;CHECK(cpa_take(&q,&h,&m,0)==CPA_OK && m.sequence==7);CHECK(cpa_finish(&q,h,0)==CPA_OK);
 CHECK(!pthread_mutex_lock(&gate));stage=2;atomic_store(&stolen,1);CHECK(!pthread_cond_broadcast(&changed));CHECK(!pthread_mutex_unlock(&gate));return 0;
}
static void wait_for(unsigned wakes){
 for(unsigned n=0;n<5000;n++){
  CHECK(!pthread_mutex_lock(&q.mutex));int ok=q.waiters==1 && q.wakes>=wakes;CHECK(!pthread_mutex_unlock(&q.mutex));
  if(ok)return;
  struct timespec t={0,1000000};nanosleep(&t,0);
 }CHECK(0);
}
int main(void){
 CHECK(cpa_init(&q,8)==CPA_OK);atomic_init(&ready,0);atomic_init(&armed,1);atomic_init(&stolen,0);pthread_t a,b;
 CHECK(!pthread_create(&a,0,waiter,0) && !pthread_create(&b,0,thief,0));wait_for(0);
 CPMessage m={.sequence=7,.length=1,.payload={42}};CHECK(cpa_offer(&q,&m,0)==CPA_OK);
 wait_for(1);CHECK(atomic_load(&stolen));CHECK(cpa_close(&q,0)==CPA_CLOSED);
 CHECK(!pthread_join(a,0) && !pthread_join(b,0));CHECK(q.completed==1 && q.releases==1 && cpa_destroy(&q)==CPA_OK);
 CHECK(!pthread_cond_destroy(&changed) && !pthread_mutex_destroy(&gate));puts("P5_WAIT_PASS real_unlocked_steal=1 predicate_rechecked=1");
}
