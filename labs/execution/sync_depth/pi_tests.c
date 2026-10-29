#define _POSIX_C_SOURCE 200809L
#include "pi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef CDPI_TRACE_MUTANT
#define CDPI_TRACE_MUTANT 0
#endif
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CDPI_CHECK:%d:%s\n",__LINE__,#x); exit(1); } } while (0)
static unsigned add(cmx_kernel *k, unsigned priority) {
  unsigned n = 0;
  while (n < CMX_TASKS && k->task[n].state != CMX_UNUSED) ++n;
  int t = cmx_task_add(k, priority, 0, (uintptr_t)(4096u + n * 512u), 256);
  CHECK(t >= 0); return (unsigned)t;
}
static void mutex(cmx_kernel *k, cmx_mutex *m, unsigned t) {
  CHECK(cmx_mutex_init(k,m,0,0)==CMX_OK);
  CHECK(cmx_mutex_lock(k,m,t,cmx_forever())==CMX_OK);
}
static void chain(void) {
  cmx_kernel k; cmx_mutex a,b; struct cdpi_stats stats;
  cmx_init(&k,CMX_FIXED);
  unsigned l=add(&k,9),m=add(&k,7),h=add(&k,3),h5=add(&k,5);
  mutex(&k,&a,l); mutex(&k,&b,m);
  CHECK(cmx_mutex_lock(&k,&a,m,cmx_forever())==CMX_BLOCKED);
  CHECK(k.task[l].priority==7);
  CHECK(cmx_mutex_lock(&k,&b,h,cmx_at(10))==CMX_BLOCKED);
  CHECK(k.task[l].priority==3 && k.task[m].priority==3);
  CHECK(cdpi_reprioritize(&k,l,9,&stats)==CMX_OK);
  CHECK(stats.edges==2 && stats.donated_pairs==3 && stats.closure_probes==256);
  CHECK(k.task[l].priority==3 && k.task[m].priority==3 && k.task[l].base_priority==9);
  CHECK(cmx_mutex_lock(&k,&a,h5,cmx_forever())==CMX_BLOCKED);
  cmx_tick(&k,10);
  CHECK(k.task[h].result==CMX_TIMEOUT && k.task[l].priority==5 && k.task[m].priority==7);
  CHECK(cmx_cancel(&k,m)==CMX_OK && k.task[l].priority==5);
  CHECK(cmx_cancel(&k,h5)==CMX_OK && k.task[l].priority==9);
  CHECK(cdpi_reprioritize(&k,l,9,&stats)==CMX_OK && stats.edges==0);
  /* Lower/raise a base while donated, then remove donors without losing it. */
  CHECK(cmx_mutex_lock(&k,&a,h,cmx_forever())==CMX_BLOCKED);
  CHECK(cdpi_reprioritize(&k,l,11,&stats)==CMX_OK && k.task[l].priority==3 && k.task[l].base_priority==11);
  CHECK(cdpi_reprioritize(&k,h,6,&stats)==CMX_OK && k.task[l].priority==6);
  CHECK(cmx_cancel(&k,h)==CMX_OK && k.task[l].priority==11);
  CHECK(cdpi_reprioritize(&k,l,4,&stats)==CMX_OK && k.task[l].priority==4);
  puts("CDPI_CHAIN:transitive=3;partial=5;base=11;restored=11;closure_probes=256");
}
static void nested(void) {
  cmx_kernel k; cmx_mutex a,b; struct cdpi_stats stats;
  cmx_init(&k,CMX_FIXED);
  unsigned l=add(&k,9),h3=add(&k,3),h5=add(&k,5);
  mutex(&k,&a,l);mutex(&k,&b,l);
  CHECK(cmx_mutex_lock(&k,&a,h5,cmx_forever())==CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&k,&b,h3,cmx_forever())==CMX_BLOCKED);
  CHECK(cdpi_reprioritize(&k,l,10,&stats)==CMX_OK && k.task[l].priority==3);
  CHECK(cmx_mutex_unlock(&k,&b,l)==CMX_OK && k.task[l].priority==5 && b.owner==(int)h3);
  CHECK(cdpi_reprioritize(&k,l,12,&stats)==CMX_OK && k.task[l].priority==5);
  CHECK(cmx_cancel(&k,h5)==CMX_OK && k.task[l].priority==12);
  CHECK(cmx_mutex_unlock(&k,&a,l)==CMX_OK);
  /* Actual CMX refuses the last dependency edge without losing ownership. */
  CHECK(cmx_mutex_lock(&k,&a,l,cmx_forever())==CMX_OK);
  CHECK(cmx_mutex_lock(&k,&b,l,cmx_forever())==CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&k,&a,h3,cmx_forever())==CMX_DEADLOCK);
  CHECK(a.owner==(int)l && b.owner==(int)h3 && k.task[h3].state==CMX_READY);
  CHECK(cmx_cancel(&k,l)==CMX_OK);
  struct cdpi_stats sentinel={99,98,97,96}; stats=sentinel;
  CHECK(cdpi_reprioritize(&k,l,256,&stats)==CMX_INVALID && stats.edges==98 && k.task[l].base_priority==12);
  /* Injected graph cycle: extension must reject before changing any priority. */
  k.task[l].state=CMX_WAITING;k.task[l].wait=CMX_W_MUTEX;k.task[l].object=&b;
  k.task[h3].state=CMX_WAITING;k.task[h3].wait=CMX_W_MUTEX;k.task[h3].object=&a;
  unsigned before=k.task[l].priority;
  CHECK(cdpi_reprioritize(&k,l,1,&stats)==CMX_DEADLOCK && k.task[l].priority==before && stats.edges==98);
  puts("CDPI_NESTED:remaining-donation=5;cycle-refused;invalid-output-preserved");
}
static void limits(void) {
  cmx_kernel k;cmx_mutex mutexes[CMX_TASKS];struct cdpi_stats stats={91,92,93,94};
  cmx_init(&k,CMX_FIXED);
  for(unsigned t=0;t<CMX_TASKS;++t){CHECK(add(&k,t?255:0)==t);mutex(&k,&mutexes[t],t);}
  for(unsigned t=0;t+1<CMX_TASKS;++t)CHECK(cmx_mutex_lock(&k,&mutexes[t+1],t,cmx_forever())==CMX_BLOCKED);
  CHECK(cdpi_reprioritize(&k,0,1,&stats)==CMX_OK && stats.vertices==16 && stats.edges==15 && stats.donated_pairs==120);
  for(unsigned t=0;t<CMX_TASKS;++t)CHECK(k.task[t].priority==1);
  CHECK(cdpi_reprioritize(&k,0,255,&stats)==CMX_OK);
  for(unsigned t=0;t<CMX_TASKS;++t)CHECK(k.task[t].priority==255);
  cmx_init(&k,CMX_FIXED);unsigned t=add(&k,9);
  CHECK(cmx_mutex_init(&k,&mutexes[0],5,1)==CMX_OK);
  CHECK(cmx_mutex_lock(&k,&mutexes[0],t,cmx_forever())==CMX_OK);
  CHECK(cdpi_reprioritize(&k,t,11,&stats)==CMX_OK && k.task[t].priority==5);
  stats.edges=92;
  CHECK(cdpi_reprioritize(&k,t,3,&stats)==CMX_INVALID && k.task[t].base_priority==11 && stats.edges==92);
  CHECK(cmx_mutex_unlock(&k,&mutexes[0],t)==CMX_OK && k.task[t].priority==11);
  puts("CDPI_LIMIT:16-task-chain;120-donor-pairs;0-and-255;ceiling-refusal-atomic");
}
static void schedule_trace(unsigned inherited, char *trace) {
  cmx_kernel k;cmx_mutex a,b;cmx_init(&k,CMX_FIXED);
  unsigned l=add(&k,9),m=add(&k,5),h=add(&k,3);
  mutex(&k,&a,l);mutex(&k,&b,l);
  CHECK(cmx_mutex_lock(&k,&a,h,cmx_forever())==CMX_BLOCKED);
  unsigned low=5,medium=6,n=0;
  for (;;) {
    /* Explicit experiment policy projection; original CMX stays unchanged. */
    if (!inherited || CDPI_TRACE_MUTANT) for(unsigned t=0;t<CMX_TASKS;++t)k.task[t].priority=k.task[t].base_priority;
    int selected=cmx_schedule(&k);CHECK(selected>=0 && n<20);
    if((unsigned)selected==l){trace[n++]='L';CHECK(low>0);--low;
      if(low==2)CHECK(cmx_mutex_unlock(&k,&b,l)==CMX_OK);
      if(!low){CHECK(cmx_mutex_unlock(&k,&a,l)==CMX_OK);CHECK(cmx_sleep(&k,l,cmx_forever())==CMX_BLOCKED);}}
    else if((unsigned)selected==m){trace[n++]='M';CHECK(medium>0);if(!--medium)CHECK(cmx_sleep(&k,m,cmx_forever())==CMX_BLOCKED);}
    else {CHECK((unsigned)selected==h && !low && a.owner==(int)h);trace[n++]='H';break;}
  }
  trace[n]=0;
}
static uint64_t now(void){struct timespec t;CHECK(clock_gettime(CLOCK_MONOTONIC,&t)==0);return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
static void costs(void) {
  char on[21],off[21];schedule_trace(1,on);schedule_trace(0,off);
  CHECK(!strcmp(on,"LLLLLH") && !strcmp(off,"MMMMMMLLLLLH"));
  cmx_kernel k;cmx_mutex a,b;struct cdpi_stats stats;cmx_init(&k,CMX_FIXED);
  unsigned l=add(&k,9),h=add(&k,3);mutex(&k,&a,l);mutex(&k,&b,l);
  CHECK(cmx_mutex_lock(&k,&a,h,cmx_forever())==CMX_BLOCKED);
  uint64_t smallest=UINT64_MAX,largest=0,total=0;
  for(unsigned sample=0;sample<31;++sample){uint64_t start=now();
    for(unsigned i=0;i<1000;++i)CHECK(cdpi_reprioritize(&k,l,9+(i&1u),&stats)==CMX_OK);
    uint64_t elapsed=now()-start;CHECK(elapsed>0);if(elapsed<smallest)smallest=elapsed;if(elapsed>largest)largest=elapsed;total+=elapsed;}
  CHECK(k.task[l].base_priority==10 && k.task[l].priority==3);
  CHECK(cmx_cancel(&k,h)==CMX_OK && k.task[l].priority==10);
  CHECK(cmx_mutex_unlock(&k,&b,l)==CMX_OK && cmx_mutex_unlock(&k,&a,l)==CMX_OK);
  volatile unsigned balance=0,checksum=~0u;
  uint64_t critical_min=UINT64_MAX,critical_max=0;
  for(unsigned sample=0;sample<31;++sample){uint64_t start=now();
    for(unsigned i=0;i<1000;++i){
      CHECK(cmx_mutex_lock(&k,&a,l,cmx_forever())==CMX_OK);
      CHECK(cmx_mutex_lock(&k,&b,l,cmx_forever())==CMX_OK);
      balance+=1;checksum=~balance;
      CHECK(cmx_mutex_unlock(&k,&b,l)==CMX_OK && cmx_mutex_unlock(&k,&a,l)==CMX_OK);
    }
    uint64_t elapsed=now()-start;CHECK(elapsed>0);if(elapsed<critical_min)critical_min=elapsed;if(elapsed>critical_max)critical_max=elapsed;
  }
  CHECK(balance==31000 && checksum==~balance);
  printf("CDPI_COST:on=%s;off=%s;owner-units=5;medium-units=6;calls=31000;batch-min-ns=%llu;batch-max-ns=%llu;total-ns=%llu;critical-batch-min-ns=%llu;critical-batch-max-ns=%llu\n",on,off,(unsigned long long)smallest,(unsigned long long)largest,(unsigned long long)total,(unsigned long long)critical_min,(unsigned long long)critical_max);
}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"chain"))chain();else if(!strcmp(argv[1],"nested"))nested();else if(!strcmp(argv[1],"limits"))limits();else if(!strcmp(argv[1],"costs"))costs();else return 2;return 0;}
