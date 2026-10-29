#include "cmk2_abi.h"
#include "pool.h"
#include "dag.h"
#ifndef DGG_MUTANT
#define DGG_MUTANT 0
#endif
static struct sa_dg graph;
static struct dg_spec spec;
static struct sp_launch launch;
static unsigned char *stacks[3];
static unsigned cpus,variant,mode;
static atomic_uint bad,arrived,mask,registered,visits[16];
static unsigned owner[16],actual[16];
static uint64_t begins[16],ends[16];
static void text(const char *s){size_t n=0;while(s[n])n++;cmk_write(s,n);}
static void number(uint64_t v){char out[24];unsigned n=0;do{out[n++]=(char)('0'+v%10);v/=10;}while(v);while(n)cmk_write(&out[--n],1);}
static void stop(unsigned status){cmk_call(CMK_EXIT,status,0,0);for(;;){}}
#define CHECK(e) do{if(!(e)){text("DG_GUEST_CHECK:");number(__LINE__);text(":" #e "\n");stop(91);}}while(0)
static void yield(void){cmk_call(CMK_YIELD,0,0,0);}
static uint64_t stamp(void){unsigned lo,hi;__asm__ volatile("lfence;rdtsc":"=a"(lo),"=d"(hi)::"memory");return (uint64_t)hi<<32|lo;}
static uint64_t mix(uint64_t offset,uint64_t count){uint64_t sum=0;for(uint64_t i=0;i<count;i++){uint64_t x=offset+i+1;x^=x>>17;x*=UINT64_C(6364136223846793005);x^=x>>31;sum+=x&1023;}return sum;}
static void execute(unsigned worker){
 unsigned wanted=DGG_MUTANT==1?0:worker;CHECK(!cmk_call(CMK2_SET_AFFINITY,1u<<wanted,0,0));unsigned cpu=(unsigned)cmk_call(CMK2_CPU_INFO,0,0,0);if(cpu!=worker)atomic_store(&bad,1);atomic_fetch_or(&mask,1u<<cpu);
 atomic_fetch_add_explicit(&registered,1,memory_order_release);while(atomic_load_explicit(&registered,memory_order_acquire)!=cpus)yield();
 while(dg_drained(&graph)!=DG_OK){
  struct dg_claim claim;int status=dg_claim(&graph,worker,&claim);if(status==DG_EMPTY){yield();continue;}CHECK(status==DG_OK);
  unsigned node=claim.node;CHECK(!atomic_fetch_add(&visits[node],1));owner[node]=worker;actual[node]=(unsigned)cmk_call(CMK2_CPU_INFO,0,0,0);begins[node]=stamp();uint64_t result;
  if(node==0){volatile uint64_t observed=mix(100000,claim.cost*1000);(void)observed;result=0;}
  else if(node==1)result=mix(5000,3000);
  else if(node==2)result=mix(0,claim.cost*1000);
  else if(variant==2&&node==3)result=mix(2000,3000);
  else{volatile uint64_t observed=mix(200000,claim.cost*1000);(void)observed;result=claim.input;}
  if(mode==1&&node==0){CHECK(dg_cancel(&graph,0)==DG_REQUESTED&&dg_drained(&graph)==DG_BUSY);}
  if(variant==0&&mode==0&&cpus>1&&(node==1||node==2)){atomic_fetch_add_explicit(&arrived,1,memory_order_release);while(atomic_load_explicit(&arrived,memory_order_acquire)!=2)yield();}
  CHECK(dg_finish(&graph,claim,mode==2&&node==1?DG_FAILED:DG_DONE,result)==DG_OK);ends[node]=stamp();
 }
}
static void entry(uintptr_t arg){unsigned gate;while(!(gate=atomic_load_explicit(&launch.gate,memory_order_acquire)))yield();if(gate==1)execute((unsigned)arg);cmk2_thread_exit(0);}
static void *allocate(void *unused,unsigned i){(void)unused;return stacks[i];}
static int create(void *unused,unsigned i,void *p,uintptr_t *handle){(void)unused;int64_t h=cmk2_thread_create(entry,(unsigned char *)p+16384,i);if(h<0)return 1;*handle=(uintptr_t)h;return 0;}
static int join(void *unused,uintptr_t handle){(void)unused;return cmk2_thread_join((uint32_t)handle)!=0;}
static void release(void *unused,unsigned i,void *p){(void)unused;CHECK(p==stacks[i]);}
static void setup(unsigned v){
 spec=(struct dg_spec){0};spec.count=v==2?5:4;spec.costs[0]=2;spec.costs[1]=3;spec.costs[2]=v==2?2:5;spec.costs[3]=v==2?3:1;if(v==2)spec.costs[4]=1;
 spec.edges[spec.edge_count++]=(struct dg_edge){0,1};spec.edges[spec.edge_count++]=(struct dg_edge){0,2};
 if(v==2){spec.edges[spec.edge_count++]=(struct dg_edge){0,3};for(unsigned i=1;i<=3;i++)spec.edges[spec.edge_count++]=(struct dg_edge){i,4};}
 else{spec.edges[spec.edge_count++]=(struct dg_edge){1,3};spec.edges[spec.edge_count++]=(struct dg_edge){2,3};if(v==1)spec.edges[spec.edge_count++]=(struct dg_edge){1,2};}
}
void _start(void){
 cpus=(unsigned)((uint64_t)cmk_call(CMK2_CPU_INFO,0,0,0)>>32);CHECK(cpus==1||cpus==2||cpus==4);for(unsigned i=0;i<3;i++){stacks[i]=cmk_alloc(4);CHECK(stacks[i]);}
 for(unsigned experiment=0;experiment<5;experiment++){
  variant=experiment<3?experiment:0;mode=experiment<3?0:experiment-2;setup(variant);atomic_init(&bad,0);atomic_init(&arrived,0);atomic_init(&mask,0);atomic_init(&registered,0);for(unsigned i=0;i<16;i++){atomic_init(&visits[i],0);begins[i]=ends[i]=0;owner[i]=actual[i]=4;}
  CHECK(dg_init(&graph,experiment+1,&spec)==DG_OK&&dg_seal(&graph)==DG_OK&&sp_launch_init(&launch,(struct sp_launch_port){allocate,create,join,release,NULL})==SP_OK);
  CHECK(sp_launch(&launch,cpus-1)==SP_OK);execute(cpus-1);CHECK(sp_shutdown(&launch)==SP_OK);
  if(atomic_load(&bad)){text("DG_GUEST_CHECK:worker-placement\n");stop(91);}
  struct dg_snapshot s;CHECK(dg_snapshot(&graph,&s)==DG_OK&&s.terminal==spec.count&&atomic_load(&mask)==(1u<<cpus)-1);
  if(!mode)CHECK(s.results[spec.count-1]==mix(0,8000));else CHECK(s.states[3]==DG_SKIPPED&&s.results[3]==0);
  text("DG_GUEST_CASE:cpus=");number(cpus);text(" experiment=");number(experiment);text(" nodes=");number(spec.count);text(" work=");number(s.work);text(" span=");number(s.span);text(" mask=");number(atomic_load(&mask));text(" terminal=");number(s.terminal);text(" result=");number(s.results[spec.count-1]);text("\n");
  for(unsigned i=0;i<spec.count;i++){
   CHECK(atomic_load(&visits[i])==(s.states[i]==DG_DONE||s.states[i]==DG_FAILED||s.states[i]==DG_CANCELLED?1u:0u));
   text("DG_GUEST_NODE:id=");number(i);text(" state=");number(s.states[i]);text(" visits=");number(atomic_load(&visits[i]));text(" ready=");number(s.ready_events[i]);text(" owner=");number(owner[i]);text(" cpu=");number(actual[i]);text(" cost=");number(spec.costs[i]);text(" cycles=");number(ends[i]-begins[i]);text("\n");
  }
 }
 /* Runtime admission on this actual target, both before/after completion. */
 struct dg_spec empty={0};CHECK(dg_init(&graph,20,&empty)==DG_OK);unsigned node,parent=0;struct dg_claim a,b;
 CHECK(dg_append(&graph,1,1,NULL,0,&node)==DG_OK&&dg_claim(&graph,0,&a)==DG_OK&&dg_append(&graph,2,1,&parent,1,&node)==DG_OK&&dg_finish(&graph,a,DG_DONE,7)==DG_OK&&dg_claim(&graph,0,&b)==DG_OK&&b.input==9&&dg_finish(&graph,b,DG_DONE,9)==DG_OK);parent=1;
 CHECK(dg_append(&graph,3,1,&parent,1,&node)==DG_OK&&dg_claim(&graph,0,&a)==DG_OK&&a.input==12&&dg_finish(&graph,a,DG_DONE,12)==DG_OK&&dg_seal(&graph)==DG_OK&&dg_drained(&graph)==DG_OK);
 text("DG_GUEST_RUNTIME:before-after=1 final=12\nDG_GUEST_PASS:validated-arbitrary-DAG-pool-lifecycle-and-failure-propagation\n");stop(0);
}
