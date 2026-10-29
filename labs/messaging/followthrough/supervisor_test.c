#include "supervisor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SC_CHECK: supervisor_follow:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
struct msg_resources{unsigned*objects[4][4];unsigned creates,destroys,fail_at,tries,drops;};
static bool open_resource(void*arg,unsigned g,unsigned c){struct msg_resources*r=arg;CHECK(!r->objects[g][c]);
  if(c==1)CHECK(r->objects[g][0]);
  if(++r->tries==r->fail_at)return false;
  unsigned*p=malloc(sizeof(*p));if(!p)return false;*p=4*g+c;r->objects[g][c]=p;r->creates++;return true;
}
static void close_resource(void*arg,unsigned g,unsigned c){struct msg_resources*r=arg;CHECK(r->objects[g][c]);
  free(r->objects[g][c]);r->objects[g][c]=NULL;r->destroys++;
}
static bool handler(void*arg,const struct mfo_value*v){(void)arg;return *(unsigned*)v->data!=99;}
static void dead(void*arg,uint64_t generation,struct mfo_value v){(void)arg;(void)generation;mfo_drop(&v);}
static void counted(void*arg){struct msg_resources*r=arg;r->drops++;}
static enum msg_status setup(struct mfs_tree*t,struct msg_resources*r,size_t parent,size_t child){void*contexts[4][4]={{0}};
  return mfs_init(t,parent,child,100,open_resource,close_resource,r,handler,contexts,dead,NULL);
}
static unsigned live(const struct msg_resources*r){unsigned n=0;for(unsigned g=0;g<4;g++)for(unsigned c=0;c<4;c++)n+=r->objects[g][c]!=NULL;return n;}
static void ordinary(void){
  for(unsigned strategy=0;strategy<3;strategy++)for(unsigned failed=0;failed<4;failed++){
    struct msg_resources r={0};struct mfs_tree t;struct mfs_report out;CHECK(setup(&t,&r,2,2)==MSG_OK&&live(&r)==16);
    CHECK(mfs_fault(&t,2,failed,0,(enum msg_strategy)strategy,0,&out)==MSG_OK);
    unsigned first=strategy==MSG_ALL?0:failed,last=strategy==MSG_ONE?failed:3,n=0;
    for(unsigned i=last+1;i>first;i--)CHECK(out.events[n++]==-(int)(8+i));
    for(unsigned i=first;i<=last;i++)CHECK(out.events[n++]==(int)(9+i));
    CHECK(out.count==n&&!out.parent_used&&!out.closed&&live(&r)==16);
    struct mfs_report old=out;t.actors[0][0].actor.stepping=true;
    CHECK(mfs_fault(&t,2,failed,1,(enum msg_strategy)strategy,0,&out)==MSG_BUSY&&out.count==old.count&&t.last==0);
    CHECK(mfs_destroy(&t)==MSG_BUSY);t.actors[0][0].actor.stepping=false;
    CHECK(mfs_fault(&t,0,0,1,(enum msg_strategy)-1,0,&out)==MSG_INVALID&&t.last==0);
    CHECK(mfs_destroy(&t)==MSG_OK&&live(&r)==0&&r.creates==r.destroys);
  }
}
static void hierarchy(void){
  struct msg_resources r={0};struct mfs_tree t;struct mfs_report out;CHECK(setup(&t,&r,1,1)==MSG_OK);
  /* Queued owned resource is dropped before its actor dependency is closed. */
  struct mfo_value owner={&r,counted};CHECK(mfa_send(&t.actors[1][3],2,&owner)==MSG_OK);
  CHECK(mfs_fault(&t,1,0,0,MSG_ALL,2,&out)==MSG_OK);
  CHECK(out.parent_used&&out.rollback&&!out.closed&&out.attempts==6&&r.drops==1&&live(&r)==16);
  const int expected[]={-8,-7,-6,-5,5,106,-5,5,6,7,8};
  CHECK(out.count==sizeof(expected)/sizeof(expected[0])&&!memcmp(out.events,expected,sizeof(expected)));
  CHECK(t.parent.count==1&&t.children[1].count==0);
  CHECK(mfs_fault(&t,1,2,1,MSG_REST,0,&out)==MSG_OK);
  CHECK(mfs_fault(&t,1,2,2,MSG_REST,0,&out)==MSG_FAILED&&out.closed&&live(&r)==0);
  CHECK(t.parent.escalated&&mfs_fault(&t,0,0,3,MSG_ALL,0,&out)==MSG_CLOSED);
  CHECK(r.creates==r.destroys&&mfs_destroy(&t)==MSG_OK);
  r=(struct msg_resources){0};CHECK(setup(&t,&r,2,2)==MSG_OK);
  /* Child2 (index1) fails locally and again during its parent's recovery. */
  CHECK(mfs_fault(&t,0,0,0,MSG_ALL,10,&out)==MSG_FAILED);
  CHECK(out.parent_used&&out.rollback&&out.closed&&live(&r)==0&&r.creates==r.destroys);
  CHECK(mfs_destroy(&t)==MSG_OK);
  for(unsigned fail=1;fail<=16;fail++){
    r=(struct msg_resources){.fail_at=fail};CHECK(setup(&t,&r,1,1)==MSG_FAILED&&t.closed&&live(&r)==0&&r.creates==r.destroys);
  }
}
static void handler_failure(void){struct msg_resources r={0};struct mfs_tree t;struct mfs_report out;CHECK(setup(&t,&r,2,2)==MSG_OK);
  unsigned bad=99;struct mfo_value borrowed_owner={&bad,NULL}; /* invalid ownership is refused */
  CHECK(mfa_send(&t.actors[0][2],2,&borrowed_owner)==MSG_INVALID);
  unsigned*heap=malloc(sizeof(*heap));CHECK(heap);*heap=99;struct mfo_value owner={heap,free};
  CHECK(mfa_send(&t.actors[0][2],2,&owner)==MSG_OK&&mfa_step(&t.actors[0][2])==MSG_FAILED);
  CHECK(!t.actors[0][2].actor.alive&&t.resources[0][2]);
  CHECK(mfs_fault(&t,0,2,10,MSG_REST,0,&out)==MSG_OK&&t.actors[0][2].actor.alive&&live(&r)==16);
  CHECK(mfs_fault(&t,0,2,9,MSG_REST,0,&out)==MSG_INVALID&&t.last==10);
  CHECK(mfs_destroy(&t)==MSG_OK&&r.creates==r.destroys);
}
static void timestamp_edges(void){
  struct msg_supervisor s;int actions[8];size_t count=99;
  CHECK(msg_supervisor_init(&s,1,10)==MSG_OK);
  CHECK(msg_restart_plan(&s,UINT64_MAX-10,MSG_ONE,0,actions,&count)==MSG_OK);
  CHECK(msg_restart_plan(&s,UINT64_MAX,MSG_ONE,0,actions,&count)==MSG_OK && s.count==1);
  CHECK(msg_restart_plan(&s,UINT64_MAX-1,MSG_ONE,0,actions,&count)==MSG_INVALID && s.last==UINT64_MAX);
  CHECK(msg_restart_plan(&s,UINT64_MAX,MSG_ONE,0,actions,&count)==MSG_FAILED && s.escalated);
  CHECK(msg_restart_plan(&s,UINT64_MAX,MSG_ALL,0,actions,&count)==MSG_CLOSED);
  CHECK(msg_supervisor_init(&s,2,UINT64_MAX)==MSG_OK);
  CHECK(msg_restart_plan(&s,0,MSG_ONE,0,actions,&count)==MSG_OK);
  CHECK(msg_restart_plan(&s,UINT64_MAX,MSG_ONE,1,actions,&count)==MSG_OK && s.count==1);
}
static void oracle(void){unsigned strategy,child,mask;
  while(scanf("%u%u%u",&strategy,&child,&mask)==3){
    struct msg_resources r={0};struct mfs_tree t;struct mfs_report out;CHECK(setup(&t,&r,2,2)==MSG_OK);
    enum msg_status status=mfs_fault(&t,0,child,0,(enum msg_strategy)strategy,mask,&out);
    printf("%u %u %u %u %u %u %zu %zu |",(unsigned)status,(unsigned)out.closed,(unsigned)out.parent_used,
           (unsigned)out.rollback,out.attempts,live(&r),t.parent.count,t.children[0].count);
    for(size_t i=0;i<out.count;i++)printf("%d,",out.events[i]);
    puts("");
    CHECK(mfs_destroy(&t)==MSG_OK&&r.creates==r.destroys);
  }
}
int main(int argc,char**argv){
  if(argc==2&&!strcmp(argv[1],"oracle")){oracle();return 0;}
  if(argc!=1)return 2;
  ordinary();hierarchy();handler_failure();timestamp_edges();puts("actual actor supervision, rollback and distinct budgets passed");return 0;
}
