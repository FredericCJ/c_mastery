#include "wakeup.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SW_CHECK:%s:%d\n",#x,__LINE__);return 1;}}while(0)
typedef struct {SwWorld *world;unsigned rings,acks,early,fail;unsigned level[4];} Port;
static int ring(void *p,unsigned c){Port *x=p;x->rings++;if(!atomic_load_explicit(&x->world->work[c],memory_order_acquire))x->early++;if(x->fail)return 0;x->level[c]=1;return 1;}
static void ack(void *p,unsigned c){Port *x=p;x->acks++;x->level[c]=0;}
int main(void){
 SwWorld w;Port p={.world=&w};SlStats stats={0};SqHandle ids[4],run;SqEvent event={0},old;SqTransfer ticket;
 CHECK(sw_init(&w,2,4,(SwPort){ring,ack,&p})==SQ_OK);
 for(unsigned i=0;i<4;i++)CHECK(sw_create(&w,0,1,i,&ids[i],&event)==SQ_OK);
 for(unsigned i=0;i<4;i++){CHECK(sw_dispatch(&w,1,&run,&event,&stats)==SQ_OK);CHECK(sw_finish(&w,1,run,SQ_BLOCKED,&event,&stats)==SQ_OK);}
 CHECK(sw_prepare_idle(&w,1,0,&stats)==SQ_INVALID&&sw_prepare_idle(&w,1,1,&stats)==SQ_IDLE&&p.level[1]==0);
 for(unsigned i=0;i<4;i++)CHECK(sw_wake(&w,0,1,ids[i],&event,&stats)==SQ_OK);
 CHECK(p.rings==1&&p.early==0&&p.level[1]==1&&atomic_load(&w.work[1])==4);
 old=event;CHECK(sw_wake(&w,0,1,ids[0],&event,&stats)==SQ_DUPLICATE&&memcmp(&old,&event,sizeof old)==0&&w.queues.queue[1].count==4);
 CHECK(sw_prepare_idle(&w,1,1,&stats)==SW_BUSY&&p.level[1]==1&&p.rings==2);
 for(unsigned i=0;i<4;i++){
  CHECK(sw_dispatch(&w,1,&run,&event,&stats)==SQ_OK&&run.slot==i);
  CHECK(sw_wake(&w,0,1,run,&event,&stats)==SQ_DUPLICATE);
  CHECK(sw_finish(&w,1,run,SQ_BLOCKED,&event,&stats)==SQ_OK);
 }
 CHECK(sw_prepare_idle(&w,1,1,&stats)==SQ_IDLE&&p.level[1]==0&&sq_invariant(&w.queues));
 p.fail=1;CHECK(sw_wake(&w,0,1,ids[0],&event,&stats)==SW_SIGNAL&&w.queues.queue[1].count==1&&w.bell[1]==1);
 p.fail=0;CHECK(sw_retry(&w,0,1,&stats)==SQ_OK&&p.level[1]==1&&w.queues.queue[1].count==1);
 CHECK(sw_transfer_begin(&w,0,1,0,ids[0],&ticket,&event,&stats)==SQ_OK);
 CHECK(sw_wake(&w,1,1,ids[0],&event,&stats)==SW_MOVING);
 p.fail=1;CHECK(sw_transfer_end(&w,0,ticket,1,&event,&stats)==SW_SIGNAL&&w.queues.queue[0].count==1);
 CHECK(sw_transfer_end(&w,0,ticket,1,&event,&stats)==SQ_STALE);
 p.fail=0;CHECK(sw_retry(&w,1,0,&stats)==SQ_OK&&sw_wake(&w,1,1,ids[0],&event,&stats)==SQ_STALE&&sw_wake(&w,1,0,ids[0],&event,&stats)==SQ_DUPLICATE);
 CHECK(sw_dispatch(&w,0,&run,&event,&stats)==SQ_OK&&sw_finish(&w,0,run,SQ_EXITED,&event,&stats)==SQ_OK);
 CHECK(sw_create(&w,0,0,0,&run,&event)==SQ_OK&&run.generation==2);
 CHECK(sw_wake(&w,1,0,ids[0],&event,&stats)==SQ_STALE&&sq_invariant(&w.queues));
 puts("SW_PASS:publication-coalescing-ack-recheck-identity-retry");return 0;
}
