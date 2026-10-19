#define _GNU_SOURCE
#include "rwlock.h"
#include "rwphase.h"
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(c) do{if(!(c)){fprintf(stderr,"CDRN_CHECK:%d:%s\n",__LINE__,#c);return 1;}}while(0)
enum{WRITER_PREFERRED,PHASE_FAIR,ORDINARY_MUTEX};
enum{ROUNDS=200,WORDS=64,EVENTS=ROUNDS*4*2};
typedef struct{char kind;unsigned task,round;uint64_t time;} Event;
typedef struct{
  cmx_kernel kernel;CDRW rw;CPF phase;cmx_mutex ordinary;
  CDRHandle rh[4];CPFHandle ph[4];pthread_mutex_t lock;pthread_cond_t changed;
  unsigned mode,rounds,cohort,ready,start,cohort_readers,readers,writer,max_readers;
  unsigned payload[WORDS],events;Event trace[EVENTS];int cpus[2];
} Native;
typedef struct{Native *n;unsigned id;int failure,cpu;uint64_t checksum,wait_ns;} Worker;
static int now(uint64_t *out){
  struct timespec t;if(clock_gettime(CLOCK_MONOTONIC_RAW,&t)||t.tv_sec<0||t.tv_nsec<0||t.tv_nsec>=1000000000L||
    (uint64_t)t.tv_sec>(UINT64_MAX-(uint64_t)t.tv_nsec)/UINT64_C(1000000000))return 0;
  *out=(uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;return 1;
}
static int event(Native *n,char kind,unsigned task,unsigned round){
  uint64_t stamp;if(n->events==EVENTS||!now(&stamp))return 0;
  n->trace[n->events++]=(Event){kind,task,round,stamp};return 1;
}
static int acquire(Native *n,unsigned id){
  int result;
  if(n->mode==WRITER_PREFERRED){
    CDRTicket t;result=cdr_begin(n->rh[id],id==3?CDR_WRITE:CDR_READ,cmx_forever(),&t);
    if(result!=CMX_OK&&result!=CMX_BLOCKED)return result;
    while((result=cdr_result(t))==CMX_BLOCKED)if(pthread_cond_wait(&n->changed,&n->lock))return CMX_INVALID;
  }else if(n->mode==PHASE_FAIR){
    CPFTicket t;result=cpf_begin(n->ph[id],id==3?CPF_WRITE:CPF_READ,cmx_forever(),&t);
    if(result!=CMX_OK&&result!=CMX_BLOCKED)return result;
    while((result=cpf_result(t))==CMX_BLOCKED)if(pthread_cond_wait(&n->changed,&n->lock))return CMX_INVALID;
  }else{
    result=cmx_mutex_lock(&n->kernel,&n->ordinary,id,cmx_forever());
    if(result==CMX_BLOCKED){
      while(n->kernel.task[id].state==CMX_WAITING)if(pthread_cond_wait(&n->changed,&n->lock))return CMX_INVALID;
      result=n->kernel.task[id].result;
    }
  }
  return result;
}
static int release(Native *n,unsigned id){
  if(n->mode==WRITER_PREFERRED)return cdr_unlock(n->rh[id]);
  if(n->mode==PHASE_FAIR)return cpf_unlock(n->ph[id]);
  return cmx_mutex_unlock(&n->kernel,&n->ordinary,id);
}
static void *worker(void *arg){
  Worker *w=arg;Native *n=w->n;cpu_set_t affinity;CPU_ZERO(&affinity);CPU_SET(n->cpus[w->id%2],&affinity);
  if(pthread_setaffinity_np(pthread_self(),sizeof affinity,&affinity)){w->failure=1;return 0;}
  if(pthread_mutex_lock(&n->lock)){w->failure=2;return 0;}
  n->ready++;if(pthread_cond_broadcast(&n->changed)){w->failure=3;return 0;}
  while(!n->start)if(pthread_cond_wait(&n->changed,&n->lock)){w->failure=4;return 0;}
  if(n->cohort&&w->id==3)while(n->cohort_readers<3)if(pthread_cond_wait(&n->changed,&n->lock)){w->failure=5;return 0;}
  if(pthread_mutex_unlock(&n->lock)){w->failure=6;return 0;}
  for(unsigned round=0;round<n->rounds;round++){
    uint64_t before,after;if(!now(&before)){w->failure=7;return 0;}
    if(pthread_mutex_lock(&n->lock)||acquire(n,w->id)!=CMX_OK||!now(&after)){w->failure=8;return 0;}
    w->wait_ns+=after-before;
    if(w->id==3){if(n->writer||n->readers){w->failure=9;return 0;}n->writer=1;}
    else{if(n->writer||(n->mode==ORDINARY_MUTEX&&n->readers)){w->failure=10;return 0;}
      n->readers++;if(n->readers>n->max_readers)n->max_readers=n->readers;}
    if(!event(n,w->id==3?'W':'R',w->id,round)){w->failure=11;return 0;}
    if(n->cohort&&w->id!=3){
      n->cohort_readers++;if(pthread_cond_broadcast(&n->changed)){w->failure=12;return 0;}
      while(n->cohort_readers<3)if(pthread_cond_wait(&n->changed,&n->lock)){w->failure=13;return 0;}
    }
    if(pthread_mutex_unlock(&n->lock)){w->failure=14;return 0;}
    w->cpu=sched_getcpu();if(w->cpu!=n->cpus[w->id%2]){w->failure=15;return 0;}
    if(w->id==3){for(unsigned i=0;i<WORDS;i++)n->payload[i]=round+1;}
    else{
      unsigned value=n->payload[0];uint64_t sum=0;
      for(unsigned i=0;i<WORDS;i++){if(n->payload[i]!=value){w->failure=16;return 0;}sum+=n->payload[i];}
      if(value>n->rounds||sum!=(uint64_t)WORDS*value){w->failure=17;return 0;}w->checksum+=sum;
    }
    if(pthread_mutex_lock(&n->lock)){w->failure=18;return 0;}
    if(!event(n,w->id==3?'w':'r',w->id,round)){w->failure=19;return 0;}
    if(w->id==3)n->writer=0;else n->readers--;
    if(release(n,w->id)!=CMX_OK||pthread_cond_broadcast(&n->changed)||pthread_mutex_unlock(&n->lock)){w->failure=20;return 0;}
  }
  return 0;
}
static int preflight(void){
  cmx_kernel k;CDRW rw;CPF p;CDRHandle rh[4];CPFHandle ph[4];CDRTicket rt[4];CPFTicket pt[4];
  cmx_init(&k,CMX_FIXED);
  for(unsigned i=0;i<4;i++)CHECK(cmx_task_add(&k,5,0,0x1000+256*i,256)==(int)i);
  CHECK(cdr_init(&rw,&k,4)==CMX_OK);
  for(unsigned i=0;i<4;i++)CHECK(cdr_register(&rw,i,&rh[i])==CMX_OK);
  CHECK(cdr_begin(rh[0],CDR_READ,cmx_forever(),&rt[0])==CMX_OK&&cdr_result(rt[0])==CMX_OK);
  CHECK(cdr_begin(rh[1],CDR_WRITE,cmx_forever(),&rt[1])==CMX_BLOCKED);
  CHECK(cdr_begin(rh[2],CDR_READ,cmx_forever(),&rt[2])==CMX_BLOCKED);
  CHECK(cdr_unlock(rh[0])==CMX_OK&&cdr_result(rt[1])==CMX_OK);
  CHECK(cdr_unlock(rh[1])==CMX_OK&&cdr_result(rt[2])==CMX_OK&&cdr_unlock(rh[2])==CMX_OK);
  CHECK(cdr_close(&rw)==CMX_OK);
  for(unsigned i=0;i<4;i++)CHECK(cdr_release(&rh[i])==CMX_OK);
  CHECK(cdr_destroy(&rw)==CMX_OK&&cpf_init(&p,&k)==CMX_OK);
  for(unsigned i=0;i<4;i++)CHECK(cpf_register(&p,i,&ph[i])==CMX_OK);
  CHECK(cpf_begin(ph[0],CPF_READ,cmx_forever(),&pt[0])==CMX_OK&&cpf_result(pt[0])==CMX_OK);
  CHECK(cpf_begin(ph[1],CPF_WRITE,cmx_forever(),&pt[1])==CMX_BLOCKED);
  CHECK(cpf_begin(ph[2],CPF_READ,cmx_forever(),&pt[2])==CMX_BLOCKED);
  CHECK(cpf_begin(ph[3],CPF_WRITE,cmx_forever(),&pt[3])==CMX_BLOCKED);
  CHECK(cpf_unlock(ph[0])==CMX_OK&&cpf_result(pt[1])==CMX_OK&&cpf_unlock(ph[1])==CMX_OK);
  CHECK(p.readers==4&&cpf_result(pt[2])==CMX_OK&&cpf_unlock(ph[2])==CMX_OK);
  CHECK(cpf_result(pt[3])==CMX_OK&&cpf_unlock(ph[3])==CMX_OK&&cpf_close(&p)==CMX_OK);
  for(unsigned i=0;i<4;i++)CHECK(cpf_release(&ph[i])==CMX_OK);
  CHECK(cpf_destroy(&p)==CMX_OK);return 0;
}
static int sample(unsigned mode,unsigned repeat,unsigned cohort){
  Native n={.mode=mode,.rounds=cohort?1:ROUNDS,.cohort=cohort};Worker w[4];pthread_t threads[4];cpu_set_t affinity;
  CHECK(!pthread_getaffinity_np(pthread_self(),sizeof affinity,&affinity));unsigned count=0;
  for(unsigned i=0;i<CPU_SETSIZE&&count<2;i++)if(CPU_ISSET(i,&affinity))n.cpus[count++]=(int)i;
  CHECK(count==2);cmx_init(&n.kernel,CMX_FIXED);
  for(unsigned i=0;i<4;i++)CHECK(cmx_task_add(&n.kernel,5,0,0x1000+256*i,256)==(int)i);
  CHECK(cdr_init(&n.rw,&n.kernel,4)==CMX_OK&&cpf_init(&n.phase,&n.kernel)==CMX_OK&&cmx_mutex_init(&n.kernel,&n.ordinary,0,0)==CMX_OK);
  for(unsigned i=0;i<4;i++)CHECK(cdr_register(&n.rw,i,&n.rh[i])==CMX_OK&&cpf_register(&n.phase,i,&n.ph[i])==CMX_OK);
  CHECK(!pthread_mutex_init(&n.lock,0)&&!pthread_cond_init(&n.changed,0));
  for(unsigned i=0;i<4;i++){w[i]=(Worker){.n=&n,.id=i,.cpu=-1};CHECK(!pthread_create(&threads[i],0,worker,&w[i]));}
  CHECK(!pthread_mutex_lock(&n.lock));while(n.ready<4)CHECK(!pthread_cond_wait(&n.changed,&n.lock));
  uint64_t begin,end;CHECK(now(&begin));n.start=1;CHECK(!pthread_cond_broadcast(&n.changed)&&!pthread_mutex_unlock(&n.lock));
  for(unsigned i=0;i<4;i++)CHECK(!pthread_join(threads[i],0)&&!w[i].failure);
  CHECK(now(&end)&&end>=begin&&n.events==n.rounds*8&&n.readers==0&&n.writer==0);
  for(unsigned i=0;i<WORDS;i++)CHECK(n.payload[i]==n.rounds);
  if(cohort)CHECK(n.max_readers==3);
  if(mode==ORDINARY_MUTEX)CHECK(n.max_readers==1);
  CHECK(cdr_close(&n.rw)==CMX_OK&&cpf_close(&n.phase)==CMX_OK&&cmx_mutex_close(&n.kernel,&n.ordinary)==CMX_OK);
  for(unsigned i=0;i<4;i++)CHECK(cdr_release(&n.rh[i])==CMX_OK&&cpf_release(&n.ph[i])==CMX_OK);
  CHECK(cdr_destroy(&n.rw)==CMX_OK&&cpf_destroy(&n.phase)==CMX_OK);
  CHECK(!pthread_cond_destroy(&n.changed)&&!pthread_mutex_destroy(&n.lock));
  const char *name=mode==WRITER_PREFERRED?"writer":mode==PHASE_FAIR?"phase":"mutex";
  printf("CDRN_SAMPLE:mode=%s;repeat=%u;cohort=%u;elapsed_ns=%" PRIu64 ";read_ops=%u;write_ops=%u;payload_words=%u;max_readers=%u;cpu0=%d;cpu1=%d\n",
    name,repeat,cohort,end-begin,3*n.rounds,n.rounds,WORDS,n.max_readers,n.cpus[0],n.cpus[1]);
  for(unsigned i=0;i<4;i++)printf("CDRN_WORKER:mode=%s;repeat=%u;cohort=%u;task=%u;cpu=%d;wait_ns=%" PRIu64 ";checksum=%" PRIu64 "\n",name,repeat,cohort,i,w[i].cpu,w[i].wait_ns,w[i].checksum);
  for(unsigned i=0;i<n.events;i++)printf("CDRN_INTERVAL:mode=%s;repeat=%u;cohort=%u;index=%u;kind=%c;task=%u;round=%u;time=%" PRIu64 "\n",name,repeat,cohort,i,n.trace[i].kind,n.trace[i].task,n.trace[i].round,n.trace[i].time);
  return 0;
}
int main(void){
  CHECK(preflight()==0);
  CHECK(sample(WRITER_PREFERRED,0,1)==0&&sample(PHASE_FAIR,0,1)==0);
  for(unsigned repeat=0;repeat<5;repeat++)for(unsigned index=0;index<3;index++)CHECK(sample((index+repeat)%3,repeat,0)==0);
  puts("CDRN_PASS:two-effective-CPUs;live-reader-cohort3;15-equal-work-samples;actual-CMX-mutex+rw+phase;ordinary-payload-publication");return 0;
}
