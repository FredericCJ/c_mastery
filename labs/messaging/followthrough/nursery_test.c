#define _POSIX_C_SOURCE 200809L
#include "nursery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"SC_CHECK: nursery_follow:%d:%s\n",__LINE__,#x); exit(1); } } while(0)
typedef struct {
  unsigned index, order;
  atomic_uint *ready, *finished, *drops;
  atomic_bool *go;
  bool cancelled;
  int code;
} Task;
static int work(void *arg, const atomic_bool *cancel) {
  Task *p = arg;
  atomic_fetch_or(p->ready, 1u << p->index);
  while (!atomic_load(p->go)) sched_yield();
  if (p->order && p->index < 2) {
    unsigned earlier = p->order == 1 ? 1 : 0;
    if (p->index != earlier)
      while (!(atomic_load(p->finished) & (1u << earlier))) sched_yield();
  }
  p->cancelled = atomic_load(cancel);
  atomic_fetch_or(p->finished, 1u << p->index);
  return p->code;
}
static void drop(void *arg) {
  Task *p = arg;
  /* A running task must have published its final context access. */
  unsigned ready = atomic_load(p->ready), done = atomic_load(p->finished);
  CHECK(!(ready & (1u << p->index)) || (done & (1u << p->index)));
  CHECK(!(atomic_fetch_or(p->drops, 1u << p->index) & (1u << p->index)));
}
static long long deadline(void) {
  struct timespec t; CHECK(!clock_gettime(CLOCK_MONOTONIC, &t));
  return (long long)t.tv_sec*1000000000 + t.tv_nsec + 5000000000LL;
}
typedef struct { MfnTree *tree; atomic_bool *go; } Release;
static void *release_after_cancel(void *arg) {
  Release *p = arg;
  while (!atomic_load(&p->tree->outer.cancel)) sched_yield();
  atomic_store(p->go, true);
  return NULL;
}
static void outcome(unsigned order, unsigned stop, size_t outer_fail, size_t inner0,
                    size_t inner1) {
  atomic_uint ready, finished, drops; atomic_bool go;
  atomic_init(&ready, 0); atomic_init(&finished, 0); atomic_init(&drops, 0);
  atomic_init(&go, false);
  Task data[4]; MfnTask tasks[4]; size_t fail[2] = {inner0, inner1};
  MfnTree tree; MfnOutcome result = {.primary = 1234}; int cancellation[2];
  CHECK(!pipe(cancellation));
  for (unsigned i=0;i<4;i++) {
    data[i] = (Task){.index=i,.order=order,.ready=&ready,.finished=&finished,
                    .drops=&drops,.go=&go,.code=order && i<2 ? (int)(11*(i+1)) : 0};
    tasks[i] = (MfnTask){work,drop,&data[i]};
  }
  CHECK(mfn_start(&tree,tasks,outer_fail,fail)==MSG_OK);
  pthread_t releaser; Release release = {&tree,&go};
  if (outer_fail || inner0 || inner1) atomic_store(&go,true);
  else {
    while (atomic_load(&ready) != 15) sched_yield();
    if (stop==1) { mfn_cancel(&tree); atomic_store(&go,true); }
    else if (stop==2 || stop==3) {
      CHECK(!pthread_create(&releaser,NULL,release_after_cancel,&release));
      if (stop==3) CHECK(write(cancellation[1],"x",1)==1);
    } else atomic_store(&go,true);
  }
  CHECK(mfn_wait(&tree,cancellation[0],stop==2 ? 0 : deadline(),&result)==MSG_OK);
  if (stop==2 || stop==3) CHECK(!pthread_join(releaser,NULL));
  CHECK(atomic_load(&drops)==15 && !tree.active);
  CHECK(result.spawn_failed == (outer_fail || inner0 || inner1));
  if (stop) {
    for (unsigned i=0;i<4;i++) CHECK(data[i].cancelled);
    CHECK(result.stop == (stop==2 ? MFN_DEADLINE : stop==3 ? MFN_EXTERNAL_CANCEL : MFN_COMPLETE));
  }
  if (order) CHECK(result.errors[0]==11 && result.errors[1]==22 && result.primary==11);
  else CHECK(result.primary==0);
  for (unsigned i=0;i<4;i++) CHECK(result.admitted[i] == ((atomic_load(&ready)&(1u<<i))!=0));
  unsigned char previous[sizeof(result)]; memcpy(previous,&result,sizeof(result));
  CHECK(mfn_wait(&tree,cancellation[0],deadline(),&result)==MSG_CLOSED);
  CHECK(!memcmp(previous,&result,sizeof(result)));
  close(cancellation[0]);close(cancellation[1]);
}
static void pool(void) {
  const int values[] = {3,-2,7,10,-6};
  MfnPool pools[4]; MfnTask tasks[4]; size_t fail[2]={0,0};
  MfnTree tree; MfnOutcome out; int cancellation[2]; CHECK(!pipe(cancellation));
  for(unsigned i=0;i<4;i++) {
    pools[i]=(MfnPool){values,5,i+1,999}; tasks[i]=(MfnTask){mfn_pool_task,NULL,&pools[i]};
  }
  CHECK(mfn_start(&tree,tasks,0,fail)==MSG_OK);
  CHECK(mfn_wait(&tree,cancellation[0],deadline(),&out)==MSG_OK && out.primary==0);
  for(unsigned i=0;i<4;i++) CHECK(pools[i].sum==12 && out.admitted[i]);
  close(cancellation[0]);close(cancellation[1]);
  atomic_bool cancelled; atomic_init(&cancelled,true); pools[0].sum=77;
  CHECK(mfn_pool_task(&pools[0],&cancelled)==-901 && pools[0].sum==77);
  atomic_store(&cancelled,false);pools[0].workers=0;
  CHECK(mfn_pool_task(&pools[0],&cancelled)==-902 && pools[0].sum==77);
}
static int fixed_result(void *arg, const atomic_bool *cancel) {
  (void)cancel; return *(int *)arg;
}
typedef struct {atomic_uint *ready,*ack;bool fails;} CancelChild;
static int cancellation_child(void *arg,const atomic_bool *cancel) {
  CancelChild *child=arg;atomic_fetch_add(child->ready,1);
  if(child->fails) {while(atomic_load(child->ready)!=8)sched_yield();return 7;}
  while(!atomic_load(cancel))sched_yield();
  atomic_fetch_add(child->ack,1);return 0;
}
static void core_admission_edges(void) {
  MsgNursery n;int result=91,zero=0;
  msg_nursery_init(&n,0);CHECK(msg_join(&n,&result)==MSG_OK && result==0 && n.count==0 && n.closed);
  for(size_t fail=1;fail<=8;fail++) {
    msg_nursery_init(&n,fail);
    for(size_t i=1;i<fail;i++)CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_OK);
    CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_FAILED && n.count==fail-1 && atomic_load(&n.cancel));
    CHECK(msg_join(&n,&result)==MSG_OK && result==0);
    for(size_t i=0;i<n.count;i++)CHECK(n.child[i].joined);
  }
  msg_nursery_init(&n,0);CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_OK);
  CHECK(msg_join(&n,&result)==MSG_OK && n.count==1 && n.child[0].joined);
  msg_nursery_init(&n,0);
  for(unsigned i=0;i<8;i++)CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_OK);
  CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_FULL && n.count==8 && n.attempts==8);
  CHECK(msg_join(&n,&result)==MSG_OK && result==0);
  CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_CLOSED);
  msg_nursery_init(&n,0);n.attempts=SIZE_MAX;
  CHECK(msg_spawn(&n,fixed_result,&zero)==MSG_OVERFLOW && n.count==0 && n.attempts==SIZE_MAX);
  atomic_uint ready,ack;atomic_init(&ready,0);atomic_init(&ack,0);CancelChild children[8];
  msg_nursery_init(&n,0);
  for(unsigned i=0;i<8;i++) {children[i]=(CancelChild){&ready,&ack,i==0};CHECK(msg_spawn(&n,cancellation_child,&children[i])==MSG_OK);}
  CHECK(msg_join(&n,&result)==MSG_OK && result==7 && atomic_load(&ack)==7 && atomic_load(&ready)==8);
}
static void oracle(void) {
  unsigned outer,a,b;int codes[4];
  while(scanf("%u%u%u%d%d%d%d",&outer,&a,&b,&codes[0],&codes[1],&codes[2],&codes[3])==7) {
    MfnTree tree;MfnTask tasks[4];MfnOutcome result;size_t fail[2]={a,b};int cancellation[2];
    CHECK(!pipe(cancellation));
    for(unsigned i=0;i<4;i++) tasks[i]=(MfnTask){fixed_result,NULL,&codes[i]};
    CHECK(mfn_start(&tree,tasks,outer,fail)==MSG_OK);
    CHECK(mfn_wait(&tree,cancellation[0],deadline(),&result)==MSG_OK);
    unsigned mask=0;for(unsigned i=0;i<4;i++) mask|=(unsigned)result.admitted[i]<<i;
    printf("%u %u %d %d %d %d %d\n",mask,(unsigned)result.spawn_failed,result.primary,
           result.errors[0],result.errors[1],result.errors[2],result.errors[3]);
    close(cancellation[0]);close(cancellation[1]);
  }
}
int main(int argc,char **argv) {
  if(argc!=2) return 2;
  if(!strcmp(argv[1],"oracle")) {oracle();return 0;}
  if(!strcmp(argv[1],"nested")) {
    core_admission_edges();outcome(0,0,0,0,0);outcome(0,1,0,0,0);outcome(0,2,0,0,0);outcome(0,3,0,0,0);pool();
  } else if(!strcmp(argv[1],"aggregate")) {
    outcome(1,0,0,0,0);outcome(2,0,0,0,0);
    for(size_t outer=0;outer<=2;outer++)for(size_t a=0;a<=2;a++)for(size_t b=0;b<=2;b++)
      if(outer||a||b) outcome(0,0,outer,a,b);
    MfnTree tree; memset(&tree,0x5a,sizeof(tree)); unsigned char before[sizeof(tree)];
    memcpy(before,&tree,sizeof(tree)); MfnTask invalid[4]={{0}};size_t fail[2]={0};
    CHECK(mfn_start(&tree,invalid,0,fail)==MSG_INVALID && !memcmp(before,&tree,sizeof(tree)));
  } else return 2;
  puts("nested nursery adapter checks passed");return 0;
}
