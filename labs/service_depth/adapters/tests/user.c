#include "kernel_abi.h"
static void print(const char *s){size_t n=0;while(s[n])n++;cmk_write(s,n);}
static void fail(const char *s){print("SD_CHECK:");print(s);print("\n");cmk_call(CMK_EXIT,91,0,0);for(;;){}}
#define CHECK(x,s) do{if(!(x))fail(s);}while(0)
static void done(unsigned x){cmk_call(CMK_EXIT,x,0,0);for(;;){}}
static int64_t span_call(unsigned op,uint64_t h,void *p,size_t n,unsigned nonblock){struct sdk_span s={(uintptr_t)p,n};return cmk_call(op,h,(uintptr_t)&s,nonblock);}
static int64_t seek_to(uint64_t h,int64_t offset,unsigned origin){struct sdk_seek s={offset,origin,0};return cmk_call(SDK_SEEK,h,(uintptr_t)&s,0);}
static uint64_t send_cap,receive_cap;
static void sender(uintptr_t unused){(void)unused;unsigned char v=99;int64_t r=span_call(SDK_SEND,send_cap,&v,1,0);cmk2_thread_exit(r==SDK_CANCELLED?0:92);}
static void receiver(uintptr_t unused){(void)unused;unsigned char v=0;int64_t r;
  do{r=span_call(SDK_RECEIVE,receive_cap,&v,1,0);}while(r==SDK_RETRY);
  cmk2_thread_exit(r==1&&v==41?0:93);
}
static void closing_receiver(uintptr_t unused){(void)unused;unsigned char v=0;int64_t r;
  do{r=span_call(SDK_RECEIVE,receive_cap,&v,1,0);}while(r==SDK_RETRY);
  cmk2_thread_exit(r==SDK_CLOSED?0:94);
}
static void blocked(uint64_t thread){for(unsigned i=0;i<10000;i++){int64_t b=cmk_call(SDK_BLOCKED,thread,0,0);if(b==1)return;CHECK(b==0,"wait handle");cmk_call(CMK_YIELD,0,0,0);}fail("wait never registered");}
static volatile unsigned *race;
static unsigned ready,go;
static void racer(uintptr_t i){unsigned cpus=(unsigned)((uint64_t)cmk_call(CMK2_CPU_INFO,0,0,0)>>32);cmk_call(CMK2_SET_AFFINITY,1u<<((unsigned)i%cpus),0,0);
  __atomic_fetch_add(&ready,1,__ATOMIC_RELEASE);while(!__atomic_load_n(&go,__ATOMIC_ACQUIRE))cmk_call(CMK_YIELD,0,0,0);
  race[i]=(unsigned)i+51;cmk2_thread_exit(0);
}
void _start(void){
  int64_t cuts=cmk_call(SDK_LOAD_PROBE,0,0,0);CHECK(cuts>=33,"load rollback");
  print("PASS sd-elf-allocation-rollback\n");
  CHECK(cmk_call(SDK_RESERVED_30E,0,0,0)==SDK_INVALID,"reserved operation rejected");
  unsigned char bytes[40]={0};const char name[]="data";
  int64_t h=cmk_call(SDK_OPEN,(uintptr_t)name,4,0);CHECK(h>0,"archive import");
  int64_t copy=cmk_call(SDK_DUP,(uint64_t)h,0,0);CHECK(copy>0,"dup admission");
  CHECK(span_call(SDK_READ,(uint64_t)h,(void *)(uintptr_t)(CMK_USER_LIMIT-1),2,0)==SDK_COPY,"read copy rejection");
  CHECK(seek_to((uint64_t)copy,0,1)==0,"read rejection preserves offset");
  CHECK(span_call(SDK_READ,(uint64_t)h,bytes,3,0)==3&&bytes[0]=='a'&&bytes[2]=='c',"read exact bytes");
  CHECK(span_call(SDK_READ,(uint64_t)copy,bytes,2,0)==2&&bytes[0]=='d'&&bytes[1]=='e',"dup shares offset");
  int64_t file_child=cmk_call(SDK_CLONE_PRIVATE,0,0,0);CHECK(file_child>=0,"file table clone");
  if(!file_child){CHECK(span_call(SDK_READ,(uint64_t)copy,bytes,1,0)==1&&bytes[0]=='f',"child shared offset");CHECK(!cmk_call(SDK_CLOSE,(uint64_t)h,0,0),"child close independent");done(0);}
  CHECK(!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)file_child,0,0),"file child wait");
  int64_t other=cmk_call(SDK_OPEN,(uintptr_t)name,4,0);CHECK(other>0&&span_call(SDK_READ,(uint64_t)other,bytes,1,0)==1&&bytes[0]=='a',"open independent offset");
  CHECK(seek_to((uint64_t)copy,INT64_MAX,1)==SDK_INVALID&&seek_to((uint64_t)copy,-7,1)==SDK_INVALID,"seek overflow bounds");
  CHECK(!cmk_call(SDK_CLOSE,(uint64_t)h,0,0),"close first");
  CHECK(span_call(SDK_READ,(uint64_t)copy,bytes,1,0)==1&&bytes[0]=='g',"dup backing survives");
  CHECK(cmk_call(SDK_CLOSE,(uint64_t)h,0,0)==SDK_STALE,"stale close");
  CHECK(seek_to((uint64_t)copy,0,2)==37&&span_call(SDK_READ,(uint64_t)copy,0,0,0)==0,"EOF null zero");
  CHECK(!cmk_call(SDK_CLOSE,(uint64_t)copy,0,0)&&!cmk_call(SDK_CLOSE,(uint64_t)other,0,0),"file final release");
  print("PASS sd-archive-file-table\n");
  unsigned char initial[]={1,2,3,4,5},replacement[]={9,8};struct sdk_span mutation={(uintptr_t)initial,5};
  CHECK(!cmk_call(SDK_MUTATE,(uintptr_t)&mutation,0,0),"mutable initial");
  int64_t snapshot=cmk_call(SDK_OPEN_SNAPSHOT,0,0,0),live=cmk_call(SDK_OPEN_LIVE,0,0,0);CHECK(snapshot>0&&live>0,"mutable readers");
  CHECK(span_call(SDK_READ,(uint64_t)live,bytes,4,0)==4&&bytes[3]==4,"live initial read");
  mutation=(struct sdk_span){(uintptr_t)replacement,2};CHECK(!cmk_call(SDK_MUTATE,(uintptr_t)&mutation,0,0),"publish truncate");
  CHECK(span_call(SDK_READ,(uint64_t)live,bytes,1,0)==0,"truncate offset beyond EOF");
  CHECK(span_call(SDK_READ,(uint64_t)snapshot,bytes,5,0)==5&&bytes[0]==1&&bytes[4]==5,"snapshot retains old backing");
  CHECK(seek_to((uint64_t)live,0,0)==0&&span_call(SDK_READ,(uint64_t)live,bytes,2,0)==2&&bytes[0]==9&&bytes[1]==8,"live new version");
  CHECK(cmk_call(SDK_MUTATE,(uintptr_t)&mutation,0,0)==SDK_FULL,"pinned versions bound");
  CHECK(!cmk_call(SDK_CLOSE,(uint64_t)snapshot,0,0),"release old version");
  mutation=(struct sdk_span){0,0};CHECK(!cmk_call(SDK_MUTATE,(uintptr_t)&mutation,0,0),"empty mutable version");
  CHECK(span_call(SDK_READ,(uint64_t)live,0,0,0)==0&&!cmk_call(SDK_CLOSE,(uint64_t)live,0,0),"live final close");
  print("PASS sd-file-clone-snapshot-live\n");
  int64_t channel=cmk_call(SDK_CHANNEL,0,0,0);CHECK(channel>0,"channel create");
  send_cap=(uint64_t)cmk_call(SDK_ATTENUATE,(uint64_t)channel,SDK_SEND_RIGHT,0);
  receive_cap=(uint64_t)cmk_call(SDK_ATTENUATE,(uint64_t)channel,SDK_RECEIVE_RIGHT,0);
  CHECK((int64_t)send_cap>0&&(int64_t)receive_cap>0,"attenuation");
  CHECK(span_call(SDK_RECEIVE,send_cap,bytes,1,1)==SDK_DENIED,"directional rights");
  CHECK(cmk_call(SDK_ATTENUATE,send_cap,7,0)==SDK_DENIED,"amplification rejected");
  bytes[0]=11;CHECK(!span_call(SDK_SEND,send_cap,bytes,1,1),"enqueue first");
  bytes[0]=22;CHECK(!span_call(SDK_SEND,send_cap,bytes,1,1),"enqueue second");
  CHECK(span_call(SDK_RECEIVE,receive_cap,(void *)(uintptr_t)CMK_USER_LIMIT,1,1)==SDK_COPY,"channel copy rejection");
  void *stack1=cmk_alloc(4),*stack2=cmk_alloc(4);CHECK(stack1&&stack2,"thread stacks");
  int64_t t=cmk2_thread_create(sender,(unsigned char *)stack1+16384,0);CHECK(t>0,"blocked sender create");blocked((uint64_t)t);
  CHECK(!cmk_call(SDK_CANCEL,(uint64_t)t,0,0),"cancel sender");
  CHECK(cmk_call(SDK_BLOCKED,(uint64_t)t,0,0)==0,"cancel clears waiter");
  CHECK(!cmk2_thread_join((uint32_t)t),"cancel result");
  CHECK(span_call(SDK_RECEIVE,receive_cap,bytes,1,1)==1&&bytes[0]==11,"copy failure retains head");
  CHECK(span_call(SDK_RECEIVE,receive_cap,bytes,1,1)==1&&bytes[0]==22,"cancel has no effect");
  CHECK(span_call(SDK_RECEIVE,receive_cap,bytes,1,1)==SDK_FULL,"no cancelled payload");
  t=cmk2_thread_create(receiver,(unsigned char *)stack1+16384,0);CHECK(t>0,"receiver create");blocked((uint64_t)t);
  bytes[0]=41;CHECK(!span_call(SDK_SEND,send_cap,bytes,1,1),"wake receiver");CHECK(!cmk2_thread_join((uint32_t)t),"receiver copied after wake");
  t=cmk2_thread_create(closing_receiver,(unsigned char *)stack1+16384,0);CHECK(t>0,"close waiter create");blocked((uint64_t)t);
  CHECK(!cmk_call(SDK_CHANNEL_CLOSE,(uint64_t)channel,0,0),"close channel");CHECK(!cmk2_thread_join((uint32_t)t),"close wakes receiver");
  CHECK(span_call(SDK_SEND,send_cap,bytes,1,1)==SDK_CLOSED,"closed send");
  uint64_t incarnation=(uint64_t)cmk_call(SDK_INCARNATION,(uint64_t)channel,0,0);
  CHECK(!cmk_call(SDK_CHANNEL_DESTROY,(uint64_t)channel,0,0),"closed channel destroy");
  CHECK(span_call(SDK_RECEIVE,receive_cap,bytes,1,1)==SDK_DENIED,"destroyed capability stale");
  channel=cmk_call(SDK_CHANNEL,0,0,0);CHECK(channel>0,"channel incarnation reuse");send_cap=receive_cap=(uint64_t)channel;
  CHECK(cmk_call(SDK_DEFERRED_WAKE,incarnation,0,0)==SDK_STALE,"old deferred wake rejected");
  CHECK(!span_call(SDK_SEND,send_cap,initial,5,1),"partial message");
  CHECK(span_call(SDK_RECEIVE_PARTIAL,receive_cap,bytes,2,1)==2&&bytes[0]==1&&bytes[1]==2,"partial first prefix");
  CHECK(span_call(SDK_RECEIVE_PARTIAL,receive_cap,(void *)(uintptr_t)CMK_USER_LIMIT,2,1)==SDK_COPY,"partial copy rejection");
  CHECK(span_call(SDK_RECEIVE_PARTIAL,receive_cap,bytes,2,1)==2&&bytes[0]==3&&bytes[1]==4,"partial retry exact suffix");
  CHECK(span_call(SDK_RECEIVE_PARTIAL,receive_cap,bytes,2,1)==SDK_PARTIAL_END+1&&bytes[0]==5,"partial end marker");
  CHECK(!span_call(SDK_SEND,send_cap,0,0,1)&&span_call(SDK_RECEIVE_PARTIAL,receive_cap,0,0,1)==SDK_PARTIAL_END,"empty partial message");
  incarnation=(uint64_t)cmk_call(SDK_INCARNATION,(uint64_t)channel,0,0);
  CHECK(!cmk_call(SDK_CHANNEL_CLOSE,(uint64_t)channel,0,0)&&!cmk_call(SDK_CHANNEL_DESTROY,(uint64_t)channel,0,0),"partial queue cleanup");
  int64_t dead_sender=cmk_call(SDK_CLONE_PRIVATE,0,0,0);CHECK(dead_sender>=0,"sender process clone");
  if(!dead_sender){channel=cmk_call(SDK_CHANNEL,0,0,0);CHECK(channel>0,"dying queue");send_cap=(uint64_t)channel;bytes[0]=1;CHECK(!span_call(SDK_SEND,send_cap,bytes,1,1)&&!span_call(SDK_SEND,send_cap,bytes,1,1),"dying full queue");t=cmk2_thread_create(sender,(unsigned char *)stack1+16384,0);CHECK(t>0,"dying sender");blocked((uint64_t)t);done(0);}
  CHECK(!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)dead_sender,0,0),"sender process quiescence");
  channel=cmk_call(SDK_CHANNEL,0,0,0);CHECK(channel>0,"queue reclaimed after death");
  CHECK(cmk_call(SDK_DEFERRED_WAKE,incarnation+((uint64_t)1<<32),0,0)==SDK_STALE,"dead incarnation wake rejected");
  CHECK(span_call(SDK_RECEIVE,(uint64_t)channel,bytes,1,1)==SDK_FULL,"dead sender did not publish");
  CHECK(!cmk_call(SDK_CHANNEL_CLOSE,(uint64_t)channel,0,0)&&!cmk_call(SDK_CHANNEL_DESTROY,(uint64_t)channel,0,0),"death queue cleanup");
  print("PASS sd-protected-channel-cancel-copy\n");
  print("PASS sd-partial-copy-dead-incarnation\n");
  race=(volatile unsigned *)(uintptr_t)cmk_call(CMK2_RESERVE_ZERO,1,0,0);CHECK((uintptr_t)race>=CMK_USER_BASE,"reserve zero");race[0]=17;race[1]=29;
  int64_t child=cmk_call(SDK_CLONE_PRIVATE,0,0,0);CHECK(child>=0,"restricted clone");
  if(!child){race[0]=99;CHECK(race[0]==99,"child cow write");done(23);}
  CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0)==23&&race[0]==17,"cow parent isolation");
  int64_t hold=cmk_call(CMK2_CHANNEL_CREATE,0,0,0);CHECK(hold>0,"COW child hold");
  child=cmk_call(SDK_CLONE_PRIVATE,0,0,0);CHECK(child>=0,"second clone");if(!child){CHECK(cmk_call(CMK2_CHANNEL_RECV,(uint64_t)hold,0,0)==1,"child release");done(0);}
  ready=go=0;int64_t a=cmk2_thread_create(racer,(unsigned char *)stack1+16384,0),b=cmk2_thread_create(racer,(unsigned char *)stack2+16384,1);CHECK(a>0&&b>0,"fault racers");
  while(__atomic_load_n(&ready,__ATOMIC_ACQUIRE)!=2)cmk_call(CMK_YIELD,0,0,0);
  CHECK(cmk_call(SDK_CLONE_PRIVATE,0,0,0)==CMK2_EINVAL,"multithread clone denied");
  __atomic_store_n(&go,1,__ATOMIC_RELEASE);CHECK(!cmk2_thread_join((uint32_t)a)&&!cmk2_thread_join((uint32_t)b),"fault race join");
  CHECK(race[0]==51&&race[1]==52,"concurrent fault values");
  CHECK(!cmk_call(CMK2_CHANNEL_SEND,(uint64_t)hold,1,0)&&!cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0),"child quiescence");
  CHECK(!cmk_call(CMK2_CHANNEL_CLOSE,(uint64_t)hold,0,0)&&!cmk_call(CMK2_CHANNEL_DESTROY,(uint64_t)hold,0,0),"hold cleanup");
  volatile unsigned *immutable=cmk_alloc(1);CHECK(immutable!=0,"immutable frame");immutable[0]=123;
  CHECK(!cmk_call(SDK_SHARE_IMMUTABLE,(uintptr_t)immutable,0,0),"immutable publication");
  child=cmk_call(SDK_CLONE_IMMUTABLE,0,0,0);CHECK(child>=0,"immutable clone");
  if(!child){CHECK(immutable[0]==123,"immutable shared byte");immutable[0]=9;fail("immutable write admitted");}
  CHECK(cmk_call(CMK2_WAIT_PROCESS,(uint64_t)child,0,0)==-15&&immutable[0]==123,"immutable hardware write rejection");
  print("PASS sd-immutable-shared-variant\n");
  int64_t sh=cmk_call(CMK2_SHM_CREATE,0,0,0);CHECK(sh>0,"shared setup");
  CHECK(cmk_call(CMK2_SHM_MAP,(uint64_t)sh,0,0)>0,"shared mapping");
  CHECK(cmk_call(SDK_CLONE_PRIVATE,0,0,0)==SDK_DENIED,"shared clone rejected before mutation");
  CHECK(!cmk_call(CMK2_SHM_CLOSE,(uint64_t)sh,0,0),"shared close");
  print("PASS sd-restricted-cow-tlb-quiescence\n");done(0);
}
