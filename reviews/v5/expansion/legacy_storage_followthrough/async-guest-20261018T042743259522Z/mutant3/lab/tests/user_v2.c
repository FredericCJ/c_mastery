#include "cmk2_abi.h"
#include "cmk2_parallel.h"
static uint32_t channel;
static unsigned shared_count,admission_release;
static void admitted(uintptr_t unused){(void)unused;while(!__atomic_load_n(&admission_release,__ATOMIC_ACQUIRE))cmk_call(CMK_YIELD,0,0,0);cmk2_thread_exit(0);}
static unsigned char io_data[4096];
static void message(const char *s){size_t n=0;while(s[n])n++;cmk_write(s,n);}
static void measure(unsigned mode,const struct cmk2_parallel_result *r){
  char line[]="MEASURE mode=0 cpus=0 tsc=0000000000000000 jobs=0000 mask=0\n";
  line[13]=(char)(48+mode);line[20]=(char)(48+((uint64_t)cmk_call(CMK2_CPU_INFO,0,0,0)>>32));
  for(unsigned i=0;i<16;i++)line[26+i]="0123456789abcdef"[(r->cycles>>((15-i)*4))&15];
  unsigned n=r->jobs;for(unsigned i=0;i<4;i++){line[51-i]=(char)(48+n%10);n/=10;}line[58]="0123456789abcdef"[r->mask&15];cmk_write(line,sizeof line-1);
}
static void die(unsigned status){if(status&&status!=23){char buf[16];unsigned n=0,v=status;do{buf[n++]=(char)(48+v%10);v/=10;}while(v);while(n)cmk_write(buf+--n,1);cmk_write("\n",1);}cmk_call(CMK_EXIT,status,0,0);for(;;){}}
#define CHECK(x) do{if(!(x)){message("FAIL v2 userspace\n");die(__LINE__);}}while(0)
#include "sa_impl.c"
#include "sa_guest.c"
static void producer(uintptr_t index){
  for(unsigned i=0;i<32;i++)if(cmk_call(CMK2_CHANNEL_SEND,channel,(unsigned)index*32+i,0))cmk2_thread_exit(99);
  __atomic_fetch_add(&shared_count,1,__ATOMIC_RELAXED);
  cmk2_thread_exit((unsigned)index+10);
}
static void simple_thread(uintptr_t value){cmk2_thread_exit((unsigned)value);}
static void closing_sender(uintptr_t q){int64_t r=cmk_call(CMK2_CHANNEL_SEND,q,99,0);cmk2_thread_exit(r==CMK2_ECLOSED?0:91);}
static volatile unsigned *race_page;
static unsigned race_ready,race_go;
static volatile unsigned *exit_shared;
static void exit_writer(uintptr_t unused){(void)unused;__atomic_store_n(exit_shared+1,1,__ATOMIC_RELEASE);for(;;)__atomic_fetch_add(exit_shared,1,__ATOMIC_RELAXED);}
static void fault_racer(uintptr_t index){
  unsigned n=(unsigned)((uint64_t)cmk_call(CMK2_CPU_INFO,0,0,0)>>32);
  cmk_call(CMK2_SET_AFFINITY,1u<<((unsigned)index%n),0,0);
  __atomic_fetch_add(&race_ready,1,__ATOMIC_RELEASE);
  while(!__atomic_load_n(&race_go,__ATOMIC_ACQUIRE))cmk_call(CMK_YIELD,0,0,0);
  race_page[index]=(unsigned)index+51;cmk2_thread_exit(0);
}
static void race_faults(void *a,void *b){
  race_ready=race_go=0;
  int64_t x=cmk2_thread_create(fault_racer,(unsigned char *)a+16384,0),y=cmk2_thread_create(fault_racer,(unsigned char *)b+16384,1);CHECK(x>0&&y>0);
  while(__atomic_load_n(&race_ready,__ATOMIC_ACQUIRE)!=2)cmk_call(CMK_YIELD,0,0,0);
  __atomic_store_n(&race_go,1,__ATOMIC_RELEASE);CHECK(!cmk2_thread_join((uint32_t)x));CHECK(!cmk2_thread_join((uint32_t)y));CHECK(race_page[0]==51&&race_page[1]==52);
}
void _start(void){
  void *early=cmk_alloc(4);CHECK(early);int64_t plain=cmk2_thread_create(simple_thread,(unsigned char *)early+16384,13);CHECK(plain>0);CHECK(cmk2_thread_join((uint32_t)plain)==13);
  uint32_t admitted_handles[3];void *admitted_stacks[3];for(unsigned i=0;i<3;i++){admitted_stacks[i]=cmk_alloc(4);CHECK(admitted_stacks[i]);int64_t h=cmk2_thread_create(admitted,(unsigned char *)admitted_stacks[i]+16384,0);CHECK(h>0);admitted_handles[i]=(uint32_t)h;}
  CHECK(cmk2_thread_create(admitted,(unsigned char *)early+16384,0)==CMK2_EAGAIN);__atomic_store_n(&admission_release,1,__ATOMIC_RELEASE);for(unsigned i=0;i<3;i++)CHECK(!cmk2_thread_join(admitted_handles[i]));
  message("PASS v2-four-thread-admission\nPASS v2-process-thread-split\n");
  unsigned cpus=(unsigned)((uint64_t)cmk_call(CMK2_CPU_INFO,0,0,0)>>32);
  for(unsigned cpu=0;cpu<cpus;cpu++){CHECK(!cmk_call(CMK2_SET_AFFINITY,1u<<cpu,0,0));CHECK((unsigned)cmk_call(CMK2_CPU_INFO,0,0,0)==cpu);}
  CHECK(cmk_call(CMK2_SET_AFFINITY,0,0,0)==CMK2_EINVAL);CHECK(!cmk_call(CMK2_SET_AFFINITY,(1u<<cpus)-1,0,0));
  message("PASS smp-affinity-migration\n");
  CHECK(!cmk_call(CMK2_TLB_SYNC,0,0,0));message("PASS smp-tlb-shootdown-ack\n");
  struct cmk2_parallel_result pr;uint64_t sum=0;for(unsigned i=1;i<=257;i++)sum+=(uint64_t)i*i;
  CHECK(!cmk2_parallel_run(&pr,1));CHECK(pr.sum==sum&&pr.jobs==257&&pr.mask==(1u<<cpus)-1&&!pr.errors&&pr.cycles>0);measure(1,&pr);message("PASS smp-parallel-reduction\n");
  CHECK(!cmk2_parallel_run(&pr,2));CHECK(pr.sum==sum*2&&pr.jobs==514&&pr.mask==(1u<<cpus)-1&&!pr.errors);measure(2,&pr);message("PASS smp-reusable-workpool\n");
  CHECK(!cmk2_parallel_run(&pr,3));CHECK(pr.sum==sum*2&&pr.jobs==514&&!pr.errors&&pr.dag_value>0);measure(3,&pr);message("PASS smp-dag-dependencies\n");
  const char child_name[]="spawn";int64_t spawned=cmk_call(CMK2_SPAWN,(uintptr_t)child_name,5,0);CHECK(spawned>0);CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)spawned,0,0)==7);
  message("PASS v2-spawn-last-thread-wait\n");
  volatile unsigned *zero=(volatile unsigned *)(uintptr_t)cmk_call(CMK2_RESERVE_ZERO,2,0,0);CHECK((uintptr_t)zero>=UINT64_C(0x40000000));CHECK(zero[0]==0&&zero[1024]==0);zero[0]=17;zero[1024]=29;
  message("PASS v2-demand-zero\n");
  int64_t child=cmk2_fork();CHECK(child>=0);
  if(!child){CHECK(zero[0]==17&&zero[1024]==29);zero[0]=42;CHECK(zero[0]==42);die(23);}
  CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0)==23);CHECK(zero[0]==17&&zero[1024]==29);CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0)==CMK2_EINVAL);
  message("PASS v2-restricted-cow-wait\n");
  int64_t q=cmk_call(CMK2_CHANNEL_CREATE,0,0,0);CHECK(q>0);channel=(uint32_t)q;
  CHECK(cmk_call(CMK2_CHANNEL_RECV,channel,0,1)==CMK2_EAGAIN);
  void *a=cmk_alloc(4),*b=cmk_alloc(4);CHECK(a&&b);
  int64_t first=cmk2_thread_create(producer,(unsigned char *)a+16384,0);
  int64_t second=cmk2_thread_create(producer,(unsigned char *)b+16384,1);CHECK(first>0&&second>0);
  CHECK(cmk2_fork()==CMK2_EINVAL);
  uint64_t seen=0;
  for(unsigned i=0;i<64;i++){int64_t v=cmk_call(CMK2_CHANNEL_RECV,channel,0,0);CHECK(v>=0&&v<64);CHECK(!(seen&(UINT64_C(1)<<v)));seen|=UINT64_C(1)<<v;}
  CHECK(seen==UINT64_MAX);CHECK(cmk2_thread_join((uint32_t)first)==10);CHECK(cmk2_thread_join((uint32_t)second)==11);CHECK(__atomic_load_n(&shared_count,__ATOMIC_RELAXED)==2);
  CHECK(cmk2_thread_join((uint32_t)first)==CMK2_EINVAL);
  CHECK(!cmk_call(CMK2_CHANNEL_CLOSE,channel,0,0));CHECK(cmk_call(CMK2_CHANNEL_SEND,channel,1,1)==CMK2_ECLOSED);
  CHECK(cmk_call(CMK2_CHANNEL_RECV,channel,0,0)==CMK2_ECLOSED);CHECK(!cmk_call(CMK2_CHANNEL_DESTROY,channel,0,0));
  q=cmk_call(CMK2_CHANNEL_CREATE,0,0,0);CHECK(q>0);for(unsigned i=0;i<8;i++)CHECK(!cmk_call(CMK2_CHANNEL_SEND,(uint64_t)q,i,0));
  first=cmk2_thread_create(closing_sender,(unsigned char *)a+16384,(uintptr_t)q);CHECK(first>0);for(unsigned i=0;i<10;i++)cmk_call(CMK_YIELD,0,0,0);
  CHECK(!cmk_call(CMK2_CHANNEL_CLOSE,(uint64_t)q,0,0));CHECK(!cmk2_thread_join((uint32_t)first));
  for(unsigned i=0;i<8;i++)CHECK(cmk_call(CMK2_CHANNEL_RECV,(uint64_t)q,0,0)==i);
  CHECK(cmk_call(CMK2_CHANNEL_RECV,(uint64_t)q,0,0)==CMK2_ECLOSED);CHECK(!cmk_call(CMK2_CHANNEL_DESTROY,(uint64_t)q,0,0));
  message("PASS v2-thread-join-blocking-channel\n");
  race_page=(volatile unsigned *)(uintptr_t)cmk_call(CMK2_RESERVE_ZERO,1,0,0);CHECK((uintptr_t)race_page>=UINT64_C(0x40000000));race_faults(a,b);
  int64_t raced=cmk2_fork();CHECK(raced>=0);if(!raced)die(0);race_faults(a,b);CHECK(!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)raced,0,0));
  message("PASS v2-concurrent-zero-cow-faults\n");
  int64_t sh=cmk_call(CMK2_SHM_CREATE,0,0,0);CHECK(sh>0);
  volatile unsigned *shared=(volatile unsigned *)(uintptr_t)cmk_call(CMK2_SHM_MAP,(uint64_t)sh,0,0);CHECK((uintptr_t)shared>=UINT64_C(0x40000000));shared[0]=1;
  child=cmk2_fork();CHECK(child>=0);if(!child){shared[0]=91;die(0);}CHECK(!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0));
  CHECK(shared[0]==91);CHECK(!cmk_call(CMK2_SHM_CLOSE,(uint64_t)sh,0,0));CHECK(cmk_call(CMK2_SHM_MAP,(uint64_t)sh,0,0)==CMK2_EPERM);CHECK(shared[0]==91);
  int64_t alive_object=cmk_call(CMK2_SHM_CREATE,0,0,0);CHECK(alive_object>0);exit_shared=(volatile unsigned *)(uintptr_t)cmk_call(CMK2_SHM_MAP,(uint64_t)alive_object,0,0);CHECK((uintptr_t)exit_shared>=UINT64_C(0x40000000));
  child=cmk2_fork();CHECK(child>=0);
  if(!child){int64_t h=cmk2_thread_create(exit_writer,(unsigned char *)a+16384,0);CHECK(h>0);while(!__atomic_load_n(exit_shared+1,__ATOMIC_ACQUIRE))cmk_call(CMK_YIELD,0,0,0);die(0);}
  CHECK(!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0));unsigned stable=__atomic_load_n(exit_shared,__ATOMIC_RELAXED);
  uint64_t until=(uint64_t)cmk_call(CMK_TICKS,0,0,0)+3;while((uint64_t)cmk_call(CMK_TICKS,0,0,0)<until)cmk_call(CMK_YIELD,0,0,0);CHECK(stable==__atomic_load_n(exit_shared,__ATOMIC_RELAXED));CHECK(!cmk_call(CMK2_SHM_CLOSE,(uint64_t)alive_object,0,0));
  message("PASS v2-exit-quiescent-wait\n");
  child=cmk2_fork();CHECK(child>=0);if(!child){unsigned made=0;while(cmk_call(CMK2_CHANNEL_CREATE,0,0,0)>0)made++;CHECK(made>=4);die(0);}CHECK(!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0));
  unsigned made=0;uint32_t handles[8];for(;;){int64_t h=cmk_call(CMK2_CHANNEL_CREATE,0,0,0);if(h<0)break;CHECK(made<8);handles[made++]=(uint32_t)h;}CHECK(made>=4);
  for(unsigned i=0;i<made;i++){CHECK(!cmk_call(CMK2_CHANNEL_CLOSE,handles[i],0,0));CHECK(!cmk_call(CMK2_CHANNEL_DESTROY,handles[i],0,0));}
  message("PASS v2-orphan-channel-reuse\n");
  message("PASS v2-shared-object-lifetime\n");
  for(unsigned i=0;i<4096;i++)io_data[i]=(unsigned char)(i*7+31);
  void *disk=(void *)(uintptr_t)CMK_DISK_HANDLE;
  CHECK(!cmk2_block(disk,1,7,io_data));CHECK(!cmk2_block(disk,4,0,0));
  for(unsigned i=0;i<4096;i++)io_data[i]=0;
  CHECK(!cmk2_block(disk,0,7,io_data));
  for(unsigned i=0;i<4096;i++)CHECK(io_data[i]==(unsigned char)(i*7+31));
  CHECK(cmk_call(CMK2_BLOCK_WAIT,UINT32_MAX,0,0)==CMK2_EINVAL);
  spawned=cmk_call(CMK2_SPAWN,(uintptr_t)child_name,5,1);CHECK(spawned>0);CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)spawned,0,0)==7);CHECK(!cmk2_block(disk,4,0,0));
  message("PASS v2-completed-IO-owner-exit\nPASS v2-async-block-completion\n");
  uint64_t old=(uint64_t)spawned;for(unsigned i=0;i<12;i++){spawned=cmk_call(CMK2_SPAWN,(uintptr_t)child_name,5,0);CHECK(spawned>0&&(uint64_t)spawned!=old);CHECK(cmk_call(CMK2_WAIT_PROCESS,old,0,0)==CMK2_EINVAL);CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)spawned,0,0)==7);old=(uint64_t)spawned;}
  sa_guest();message("PASS v2-process-slot-generation-reuse\n");die(0);
}
