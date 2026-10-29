#define _GNU_SOURCE
#include "pool.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define CHECK(e) do{if(!(e)){printf("SP_NATIVE_CHECK:%u:%s\n",(unsigned)__LINE__,#e);return 1;}}while(0)
enum {JOBS=12,STACK_BYTES=1024*1024};
struct sa_context;
struct sa_argument {struct sa_context *context;unsigned index;};
struct sa_context {
 struct sp_launch launch;struct sp_pool pool;pthread_t threads[4];struct sa_argument arguments[4];
 unsigned allocate_fail,create_fail,join_fail,created[4],joined[4],released,mode,workers;
 int cpus[4];atomic_uint bad,ready,release,visits[JOBS],nested_full,steals;
 uint64_t costs[JOBS],offsets[JOBS],submitted[JOBS],results[JOBS],begin[JOBS],end[JOBS],worker_cpu[4],idle[4];int actual_cpu[JOBS];
};
static uint64_t now(clockid_t id){struct timespec t;if(clock_gettime(id,&t))abort();return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
static uint64_t work(uint64_t offset,uint64_t cost){uint64_t sum=0;for(uint64_t k=0;k<cost;k++){uint64_t x=offset+k+1;x^=x>>17;x*=UINT64_C(6364136223846793005);x^=x>>31;sum+=x;}return sum;}
static void execute(struct sa_context *c,unsigned index,struct sp_claim claim){
 unsigned job=(unsigned)claim.payload;
 if(job>=JOBS||atomic_fetch_add_explicit(&c->visits[job],1,memory_order_relaxed)){atomic_store(&c->bad,1);return;}
 if(c->mode==5&&job/3!=index)atomic_fetch_add(&c->steals,1);
 c->actual_cpu[job]=sched_getcpu();c->begin[job]=now(CLOCK_MONOTONIC);
 c->results[job]=work(c->offsets[job],claim.cost);c->end[job]=now(CLOCK_MONOTONIC);
 if(claim.worker!=index||sp_complete(&c->pool,claim,c->results[job])!=SP_OK)atomic_store(&c->bad,1);
}
static void *worker(void *pointer){
 struct sa_argument *a=pointer;struct sa_context *c=a->context;unsigned index=a->index;cpu_set_t set;CPU_ZERO(&set);CPU_SET(c->cpus[index],&set);
 if(pthread_setaffinity_np(pthread_self(),sizeof set,&set)||sched_getcpu()!=c->cpus[index])atomic_store(&c->bad,1);
 unsigned gate;while(!(gate=atomic_load_explicit(&c->launch.gate,memory_order_acquire)))sched_yield();
 uint64_t cpu_start=now(CLOCK_THREAD_CPUTIME_ID);
 if(gate==1&&c->mode==1){
  for(unsigned i=index;i<JOBS;i+=c->workers){
   if(atomic_fetch_add(&c->visits[i],1))atomic_store(&c->bad,1);
   c->actual_cpu[i]=sched_getcpu();c->begin[i]=now(CLOCK_MONOTONIC);c->results[i]=work(c->offsets[i],c->costs[i]);c->end[i]=now(CLOCK_MONOTONIC);
  }
  atomic_fetch_add_explicit(&c->ready,1,memory_order_release);
 }
 if(gate==1&&c->mode==3){
  struct sp_claim claim;if(sp_claim(&c->pool,index,&claim)!=SP_OK)atomic_store(&c->bad,1);
  else{atomic_fetch_add_explicit(&c->ready,1,memory_order_release);while(!atomic_load_explicit(&c->release,memory_order_acquire))sched_yield();execute(c,index,claim);}
 }
 if(gate==1&&c->mode==4){
  struct sp_claim parent;if(sp_claim(&c->pool,index,&parent)!=SP_OK)atomic_store(&c->bad,1);
  else{
   atomic_fetch_add_explicit(&c->ready,1,memory_order_release);while(!atomic_load_explicit(&c->release,memory_order_acquire))sched_yield();
   struct sp_id child={0,99,99};int r=sp_submit(&c->pool,0,9+index,1,&child);
   if(r==SP_FULL&&child.world==99)atomic_fetch_add(&c->nested_full,1);else atomic_store(&c->bad,1);
   /* Refusal is propagated; the caller still owns and completes the parent. */
   execute(c,index,parent);
   atomic_fetch_add_explicit(&c->ready,1,memory_order_release);
   while(atomic_load_explicit(&c->ready,memory_order_acquire)<2*c->workers)sched_yield();
  }
 }
 if(gate==1&&c->mode==5&&!index)while(!atomic_load_explicit(&c->release,memory_order_acquire))sched_yield();
 while(atomic_load_explicit(&c->launch.gate,memory_order_acquire)!=2){
  if(c->mode==2||c->mode==4||c->mode==5){struct sp_claim claim;int r=sp_claim(&c->pool,index,&claim);if(r==SP_OK)execute(c,index,claim);else if(r!=SP_EMPTY)atomic_store(&c->bad,1);else c->idle[index]++;}
  sched_yield();
 }
 c->worker_cpu[index]=now(CLOCK_THREAD_CPUTIME_ID)-cpu_start;return NULL;
}
static void *allocate(void *pointer,unsigned index){struct sa_context *c=pointer;void *p=NULL;if(index==c->allocate_fail)return NULL;return posix_memalign(&p,4096,STACK_BYTES)?NULL:p;}
static int create(void *pointer,unsigned index,void *stack,uintptr_t *handle){
 struct sa_context *c=pointer;if(index==c->create_fail)return EAGAIN;
 pthread_attr_t attr;if(pthread_attr_init(&attr))return EINVAL;
 int r=pthread_attr_setstack(&attr,stack,STACK_BYTES);c->arguments[index]=(struct sa_argument){c,index};
 if(!r)r=pthread_create(&c->threads[index],&attr,worker,&c->arguments[index]);
 if(pthread_attr_destroy(&attr))atomic_store(&c->bad,1);
 if(!r){c->created[index]=1;*handle=index+1;}return r;
}
static int join(void *pointer,uintptr_t handle){struct sa_context *c=pointer;unsigned i=(unsigned)handle-1;if(i>=4)return EINVAL;if(i==c->join_fail)return EIO;int r=pthread_join(c->threads[i],NULL);if(!r)c->joined[i]=1;return r;}
static void release(void *pointer,unsigned index,void *stack){struct sa_context *c=pointer;if(c->created[index]&&!c->joined[index]){atomic_store(&c->bad,1);return;}free(stack);c->released++;}
static int init(struct sa_context *c,unsigned workers,unsigned mode,uint64_t world){
 memset(c,0,sizeof *c);c->allocate_fail=c->create_fail=c->join_fail=4;c->workers=workers;c->mode=mode;
 atomic_init(&c->bad,0);atomic_init(&c->ready,0);atomic_init(&c->release,0);atomic_init(&c->nested_full,0);atomic_init(&c->steals,0);
 for(unsigned i=0;i<JOBS;i++)atomic_init(&c->visits[i],0);
 cpu_set_t allowed;CHECK(!sched_getaffinity(0,sizeof allowed,&allowed));unsigned found=0;
 for(int cpu=0;cpu<CPU_SETSIZE&&found<4;cpu++)if(CPU_ISSET(cpu,&allowed))c->cpus[found++]=cpu;
 CHECK(found>=workers&&sp_init(&c->pool,world,workers,0)==SP_OK);
 CHECK(sp_launch_init(&c->launch,(struct sp_launch_port){allocate,create,join,release,c})==SP_OK);return 0;
}
static int failures(void){
 struct sa_context c;unsigned cuts=0;
 for(unsigned n=0;n<=4;n++){
  CHECK(!init(&c,n,0,100+n)&&sp_launch(&c.launch,n)==SP_OK&&sp_shutdown(&c.launch)==SP_OK&&!atomic_load(&c.bad)&&c.released==n);cuts++;
  for(unsigned cut=0;cut<n;cut++)for(unsigned kind=0;kind<2;kind++){
   CHECK(!init(&c,n,0,200+cuts));if(kind)c.create_fail=cut;else c.allocate_fail=cut;
   CHECK(sp_launch(&c.launch,n)==SP_FAILED&&c.released==cut+kind&&c.launch.allocated==0&&c.launch.started==0&&!atomic_load(&c.bad));cuts++;
  }
 }
 CHECK(!init(&c,4,0,900)&&sp_launch(&c.launch,4)==SP_OK);c.join_fail=1;
 CHECK(sp_shutdown(&c.launch)==SP_DEGRADED&&c.launch.allocated==2&&c.launch.started==2&&c.released==3&&!atomic_load(&c.bad));
 c.join_fail=4;CHECK(sp_shutdown(&c.launch)==SP_OK&&c.released==4&&!atomic_load(&c.bad));
 printf("SP_NATIVE_LAUNCH:{\"cuts\":%u,\"degraded_retained\":1,\"retry_joined\":1,\"custom_stack_bytes\":%u}\n",cuts,STACK_BYTES);return 0;
}
static int cancellation(void){
 struct sa_context c;struct sp_id id;struct sp_snapshot s;unsigned state;uint64_t result;
 CHECK(!init(&c,1,3,1000)&&sp_submit(&c.pool,0,0,100,&id)==SP_OK&&sp_launch(&c.launch,1)==SP_OK);
 while(atomic_load_explicit(&c.ready,memory_order_acquire)!=1)sched_yield();
 CHECK(sp_cancel(&c.pool,id)==SP_REQUESTED&&sp_collect(&c.pool,id)==SP_BUSY);
 atomic_store_explicit(&c.release,1,memory_order_release);
 do{CHECK(sp_snapshot(&c.pool,&s)==SP_OK);}while(s.running);
 CHECK(sp_shutdown(&c.launch)==SP_OK&&!atomic_load(&c.bad)&&sp_result(&c.pool,id,&state,&result)==SP_OK&&state==SP_CANCELLED&&result==0&&atomic_load(&c.visits[0])==1);
 CHECK(!init(&c,1,2,1001)&&sp_submit(&c.pool,0,0,100,&id)==SP_OK&&sp_cancel(&c.pool,id)==SP_OK&&sp_close(&c.pool,0)==SP_OK&&sp_launch(&c.launch,1)==SP_OK&&sp_shutdown(&c.launch)==SP_OK&&!atomic_load(&c.visits[0])&&!atomic_load(&c.bad));
 puts("SP_NATIVE_CANCEL:queued-no-call;claimed-retained-until-completion");return 0;
}
static int nested(void){
 struct sa_context c;struct sp_id id;struct sp_snapshot s;CHECK(!init(&c,3,4,1100));
 for(unsigned i=0;i<3;i++)CHECK(sp_submit(&c.pool,0,i,100,&id)==SP_OK);
 CHECK(sp_launch(&c.launch,3)==SP_OK);while(atomic_load_explicit(&c.ready,memory_order_acquire)!=3)sched_yield();
 for(unsigned i=3;i<6;i++)CHECK(sp_submit(&c.pool,0,i,100,&id)==SP_OK);
 atomic_store_explicit(&c.release,1,memory_order_release);
 do{CHECK(sp_snapshot(&c.pool,&s)==SP_OK);}while(s.completed!=6);
 CHECK(sp_close(&c.pool,0)==SP_OK&&sp_shutdown(&c.launch)==SP_OK&&atomic_load(&c.nested_full)==3&&!atomic_load(&c.bad));
 for(unsigned i=0;i<6;i++)CHECK(atomic_load(&c.visits[i])==1);
 puts("SP_NATIVE_NESTED:3-running-parents;3-queued-children;3-nonblocking-refusals;6-completions");return 0;
}
static int stealing(void){
 struct sa_context c;struct sp_id ids[JOBS];struct sp_snapshot s;CHECK(!init(&c,4,5,1200)&&sp_init(&c.pool,1201,4,1)==SP_OK);
 for(unsigned i=0;i<JOBS;i++)CHECK(sp_submit(&c.pool,i/3,i,1000+i,&ids[i])==SP_OK);
 CHECK(sp_close(&c.pool,0)==SP_OK&&sp_launch(&c.launch,4)==SP_OK);
 do{CHECK(sp_snapshot(&c.pool,&s)==SP_OK);}while(s.completed!=JOBS);
 atomic_store_explicit(&c.release,1,memory_order_release);
 CHECK(sp_shutdown(&c.launch)==SP_OK&&!atomic_load(&c.bad)&&atomic_load(&c.steals)>=3);
 for(unsigned i=0;i<JOBS;i++){unsigned state;uint64_t value;CHECK(atomic_load(&c.visits[i])==1&&sp_result(&c.pool,ids[i],&state,&value)==SP_OK&&state==SP_COMPLETED&&value==work(0,1000+i));}
 printf("SP_NATIVE_STEAL:jobs=12 suspended-owner=0 minimum-stolen=3 actual-stolen=%u\n",atomic_load(&c.steals));return 0;
}
static int performance(unsigned workers,unsigned mode,unsigned trial,int chunked){
 struct sa_context c;struct sp_id ids[JOBS];struct sp_snapshot s;CHECK(!init(&c,workers,mode,2000+100*trial+10*workers+mode+(unsigned)chunked*1000));
 for(unsigned i=0;i<JOBS;i++)c.costs[i]=chunked?(i<6?200000:10000):(i==0?1200000:10000);
 /* Chunked variant preserves total iterations exactly: six chunks replace one
  * long job, remaining short jobs are represented by six larger short chunks. */
 if(chunked)for(unsigned i=6;i<JOBS;i++)c.costs[i]=(i==6?20000:18000);
 uint64_t offset=0;for(unsigned i=0;i<JOBS;i++){c.offsets[i]=offset;offset+=c.costs[i];}
 uint64_t wall=now(CLOCK_MONOTONIC),origin=wall,cpu=now(CLOCK_PROCESS_CPUTIME_ID);for(unsigned i=0;i<JOBS;i++)c.submitted[i]=origin;
 CHECK(sp_launch(&c.launch,workers)==SP_OK);unsigned max_queued=0,refusals=0;
 if(mode==2){
  for(unsigned i=0;i<JOBS;i++){int r;do{c.submitted[i]=now(CLOCK_MONOTONIC);r=sp_submit(&c.pool,0,i,c.costs[i],&ids[i]);if(r==SP_FULL){refusals++;sched_yield();}}while(r==SP_FULL);CHECK(r==SP_OK);CHECK(sp_snapshot(&c.pool,&s)==SP_OK);if(s.queued>max_queued)max_queued=s.queued;}
  CHECK(sp_close(&c.pool,0)==SP_OK);do{CHECK(sp_snapshot(&c.pool,&s)==SP_OK);}while(s.completed!=JOBS);
 }else while(atomic_load_explicit(&c.ready,memory_order_acquire)!=workers)sched_yield();
 CHECK(sp_shutdown(&c.launch)==SP_OK&&!atomic_load(&c.bad));cpu=now(CLOCK_PROCESS_CPUTIME_ID)-cpu;wall=now(CLOCK_MONOTONIC)-wall;
 uint64_t iterations=0,short_wait=0,aggregate=0;
 for(unsigned i=0;i<JOBS;i++){
  CHECK(atomic_load(&c.visits[i])==1&&c.results[i]==work(c.offsets[i],c.costs[i])&&c.end[i]>=c.begin[i]&&c.begin[i]>=c.submitted[i]);iterations+=c.costs[i];aggregate+=c.results[i];
  int found=0;for(unsigned w=0;w<workers;w++)found|=c.actual_cpu[i]==c.cpus[w];CHECK(found);
  if(i)short_wait+=c.begin[i]-(c.begin[0]<c.begin[i]?c.begin[0]:c.begin[i]);
  if(mode==2){unsigned state;uint64_t value;CHECK(sp_result(&c.pool,ids[i],&state,&value)==SP_OK&&state==SP_COMPLETED&&value==c.results[i]);}
 }
 CHECK(iterations==1310000&&aggregate==work(0,iterations)&&max_queued<=3);
 printf("SP_NATIVE_SAMPLE:{\"workers\":%u,\"policy\":\"%s\",\"trial\":%u,\"chunked\":%d,\"jobs\":12,\"iterations\":%llu,\"checksum\":%llu,\"max_observed_queued\":%u,\"full_refusals\":%u,\"wall_ns\":%llu,\"process_cpu_ns\":%llu,\"relative_start_sum_ns\":%llu,\"worker_cpu_ns\":[",workers,mode==1?"static":"dynamic",trial,chunked,(unsigned long long)iterations,(unsigned long long)aggregate,max_queued,refusals,(unsigned long long)wall,(unsigned long long)cpu,(unsigned long long)short_wait);
 for(unsigned w=0;w<workers;w++)printf("%s%llu",w?",":"",(unsigned long long)c.worker_cpu[w]);
 printf("],\"idle_polls\":[");for(unsigned w=0;w<workers;w++)printf("%s%llu",w?",":"",(unsigned long long)c.idle[w]);printf("],\"ledger\":[");
 for(unsigned i=0;i<JOBS;i++)printf("%s{\"id\":%u,\"offset\":%llu,\"count\":%llu,\"admission_attempt_ns\":%llu,\"start_ns\":%llu,\"completion_ns\":%llu,\"cpu\":%d,\"result\":%llu}",i?",":"",i,(unsigned long long)c.offsets[i],(unsigned long long)c.costs[i],(unsigned long long)(c.submitted[i]-origin),(unsigned long long)(c.begin[i]-origin),(unsigned long long)(c.end[i]-origin),c.actual_cpu[i],(unsigned long long)c.results[i]);
 puts("]}");return 0;
}
int main(void){
 if(failures()||cancellation()||nested()||stealing())return 1;
 for(unsigned trial=0;trial<3;trial++)for(unsigned n=1;n<=4;n*=2)for(unsigned mode=1;mode<=2;mode++)if(performance(n,mode,trial,0))return 1;
 for(unsigned trial=0;trial<3;trial++)if(performance(2,2,trial,1))return 1;
 puts("SP_NATIVE_PASS:owned-custom-stacks-jobs-refusal-cancellation-and-equivalent-policy-samples");return 0;
}
