#define _GNU_SOURCE
#include "notifications_native.h"
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#define CHECK(c) do { if (!(c)) { fprintf(stderr,"CDNN_CHECK:%d:%s\n",__LINE__,#c); return 1; } } while (0)
enum { EACH = 2000, SAMPLES = 31 };
struct cmx_worker {
  struct cdn_native *port;
  int cpu, producer, failed;
  unsigned accepted, refusals, samples;
  uint64_t wait[SAMPLES], work[SAMPLES];
};
static int pin(int cpu) {
  cpu_set_t set, actual; CPU_ZERO(&set); CPU_SET(cpu, &set);
  return !pthread_setaffinity_np(pthread_self(),sizeof set,&set) &&
         !pthread_getaffinity_np(pthread_self(),sizeof actual,&actual) &&
         CPU_COUNT(&actual)==1 && CPU_ISSET(cpu,&actual) && sched_getcpu()==cpu;
}
static void *work(void *arg) {
  struct cmx_worker *w = arg;
  if (!pin(w->cpu)) { w->failed=1; return 0; }
  unsigned limit = w->producer ? EACH : 2*EACH;
  for (unsigned attempts = 0; w->accepted < limit && attempts < 1000000; attempts++) {
    struct cdnn_observation observation;
    int status = w->producer ? cdnn_post(w->port,1,&observation) : cdnn_take(w->port,7,&observation);
    if (status != CMX_OK && status != (w->producer ? CMX_FULL : CMX_TIMEOUT)) { w->failed=2; return 0; }
    if (observation.cpu != w->cpu) { w->failed=3; return 0; }
    if (w->samples < SAMPLES) {
      w->wait[w->samples]=observation.lock_wait_ns;
      w->work[w->samples++]=observation.protected_work_ns;
    }
    if (status==CMX_OK) w->accepted+=observation.count;
    else if (w->producer) {
      if (observation.refused!=1 || !observation.fault_sequence) { w->failed=4; return 0; }
      w->refusals++;
    }
    if (status!=CMX_OK) sched_yield();
  }
  if (w->accepted != limit) w->failed=5;
  return 0;
}
/* Race-free controlled counterexample: two relaxed atomic loads followed by
   stores do not form an atomic increment. A mutex/condition fixes the schedule
   so both loads observe zero, without introducing a C data race. */
struct cmx_counter {
  atomic_uint count;
  pthread_mutex_t mutex;
  pthread_cond_t condition;
  unsigned loaded, repair;
  int failed;
};
static void *counter_worker(void *arg) {
  struct cmx_counter *c = arg;
  if (pthread_mutex_lock(&c->mutex)) return c;
  unsigned old=atomic_load_explicit(&c->count,memory_order_relaxed);
  c->loaded++;
  if (pthread_cond_broadcast(&c->condition)) c->failed=1;
  while(c->loaded<2) if(pthread_cond_wait(&c->condition,&c->mutex)) { c->failed=1; break; }
  if (pthread_mutex_unlock(&c->mutex)) return c;
  if(c->repair) (void)atomic_fetch_add_explicit(&c->count,1,memory_order_relaxed);
  else atomic_store_explicit(&c->count,old+1,memory_order_relaxed);
  return 0;
}
static int counter(unsigned repair) {
  struct cmx_counter c={0}; pthread_t threads[2];
  atomic_init(&c.count,0);c.repair=repair;
  CHECK(!pthread_mutex_init(&c.mutex,0) && !pthread_cond_init(&c.condition,0));
  for(unsigned i=0;i<2;i++)CHECK(!pthread_create(&threads[i],0,counter_worker,&c));
  for(unsigned i=0;i<2;i++){void *result;CHECK(!pthread_join(threads[i],&result)&&!result);}
  CHECK(!c.failed && !pthread_cond_destroy(&c.condition) && !pthread_mutex_destroy(&c.mutex));
  unsigned actual=atomic_load_explicit(&c.count,memory_order_relaxed);
  printf("CDNN_ATOMIC:repair=%u;actual=%u;expected=2\n",repair,actual);
  CHECK(actual==2);
  return 0;
}
int main(int argc,char **argv) {
  if(argc==2&&!strcmp(argv[1],"lost-update"))return counter(0);
  if(argc==2&&!strcmp(argv[1],"atomic-repair"))return counter(1);
  if(argc!=1)return 2;
  cpu_set_t available; int cpus[2]={-1,-1};
  CHECK(!pthread_getaffinity_np(pthread_self(),sizeof available,&available));
  for(int i=0;i<CPU_SETSIZE;i++)if(CPU_ISSET(i,&available)){
    if(cpus[0]<0)cpus[0]=i;else {cpus[1]=i;break;}
  }
  CHECK(cpus[1]>=0); /* Missing two allowed CPUs is not a passing variant. */
  cmx_kernel k; struct cd_notify n; struct cdn_handle h; struct cdn_native port; struct cdn_post post;
  cmx_init(&k,CMX_FIXED);CHECK(cmx_task_add(&k,5,0,0x1000,256)==0);
  cdn_init(&n,&k);CHECK(cdn_attach(&n,0,8,&h)==CMX_OK && cdnn_init(&port,h)==CMX_OK);
  struct cdnn_observation observed;struct cdnn_state state;
  CHECK(cdnn_post(&port,3,&observed)==CMX_OK);
  CHECK(cdnn_snapshot(&port,&state)==CMX_OK && state.pending==3 && state.accepted==3);
  CHECK(cdnn_take(&port,UINT32_MAX,&observed)==CMX_OK && observed.count==3);
  /* Retire preflight before the independent 4000-occurrence workload. */
  CHECK(cdnn_destroy(&port)==CMX_OK && cdn_close(h)==CMX_OK && cdn_detach(h)==CMX_OK);
  CHECK(cmx_exit(&k,0,0)==CMX_OK && cmx_reap(&k,0)==CMX_OK && cmx_task_add(&k,5,0,0x1000,256)==0);
  CHECK(cdn_attach(&n,0,8,&h)==CMX_OK && cdnn_init(&port,h)==CMX_OK);
  struct cmx_worker workers[3]={{.port=&port,.cpu=cpus[1],.producer=1},
                     {.port=&port,.cpu=cpus[1],.producer=1},
                     {.port=&port,.cpu=cpus[0],.producer=0}};
  pthread_t threads[3];
  for(unsigned i=0;i<3;i++)CHECK(!pthread_create(&threads[i],0,work,&workers[i]));
  for(unsigned i=0;i<3;i++)CHECK(!pthread_join(threads[i],0)&&!workers[i].failed);
  CHECK(cdnn_snapshot(&port,&state)==CMX_OK && state.invariant && state.accepted==4000 && state.consumed==4000 && state.pending==0 && state.cancelled==0);
  CHECK(state.rejected==workers[0].refusals+workers[1].refusals);
  CHECK(cdnn_destroy(&port)==CMX_OK && cdn_post(h,1,&post)==CMX_OK && cdn_close(h)==CMX_OK && cdn_detach(h)==CMX_OK);
  for(unsigned i=0;i<3;i++)for(unsigned j=0;j<workers[i].samples;j++)
    printf("CDNN_SAMPLE:worker=%u;cpu=%d;lock_wait_ns=%" PRIu64 ";protected_work_ns=%" PRIu64 "\n",i,workers[i].cpu,workers[i].wait[j],workers[i].work[j]);
  printf("CDNN_NATIVE:producer_cpu=%d;consumer_cpu=%d;accepted=4000;consumed=4000;refused_attempts=%" PRIu64 ";pending=0\n",cpus[1],cpus[0],state.rejected);
  return 0;
}
