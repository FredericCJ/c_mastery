#include "balance.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SB_CHECK:%u:%s\n",__LINE__,#x);return 1;}}while(0)
struct sa_port {unsigned rings[4],fail;};
static int ring(void *ctx,unsigned cpu){struct sa_port *p=ctx;p->rings[cpu]++;return !p->fail;}
static void ack(void *ctx,unsigned cpu){(void)ctx;(void)cpu;}
static int contract(void){
 struct sb_world w;struct sa_port p={{0},0};struct sq_handle a,b,c,run;struct sq_event e={0},before={0},end;struct sq_transfer t={0},old;struct sl_stats s={0};struct sb_timer timer;
 CHECK(sb_init(&w,3,2,(struct sw_port){ring,ack,&p})==SQ_OK);
 CHECK(sb_create(&w,0,0,0,7,&a,&e)==SQ_OK);CHECK(sb_create(&w,0,0,1,1,&b,&e)==SQ_OK);
 CHECK(sb_affinity(&w,0,a,0,&s)==SQ_INVALID);CHECK(sb_affinity(&w,0,a,8,&s)==SQ_INVALID);
 CHECK(sb_dispatch(&w,0,&run,&e,&s)==SQ_OK&&run.slot==0);
 CHECK(sb_affinity(&w,1,a,6,&s)==SB_DEFERRED&&w.mask[0]==7&&w.pending[0]==6&&sb_invariant(&w));
 CHECK(sb_move_begin(&w,1,2,a,0,&t,&e,&s)==SQ_STATE);
 CHECK(sb_finish(&w,0,a,SQ_READY,&e,&s)==SQ_OK&&w.mask[0]==6&&!w.pending[0]);
 CHECK(sb_dispatch(&w,0,&run,&e,&s)==SQ_OK&&run.slot==1);CHECK(sb_finish(&w,0,b,SQ_BLOCKED,&e,&s)==SQ_OK);
 CHECK(sb_dispatch(&w,0,&run,&e,&s)==SB_AFFINITY);
 before=e;old=t;CHECK(sb_move_begin(&w,0,2,a,1,&t,&e,&s)==SB_INJECTED&&memcmp(&e,&before,sizeof e)==0&&memcmp(&t,&old,sizeof t)==0&&sb_invariant(&w));
 CHECK(sb_move_begin(&w,0,2,a,2,&t,&e,&s)==SB_INJECTED&&memcmp(&e,&before,sizeof e)==0&&memcmp(&t,&old,sizeof t)==0&&sb_invariant(&w));
 CHECK(atomic_load(&w.wake.queues.task[0].home)==0&&w.wake.queues.queue[0].count==1&&!w.wake.queues.queue[2].reserved);
 CHECK(sb_timer_arm(&w,1,a,&timer,&s)==SQ_OK&&timer.old_home==0);
 CHECK(sb_move_begin(&w,0,2,a,0,&t,&e,&s)==SQ_OK&&sb_invariant(&w));
 CHECK(sb_wake(&w,1,a,&end,&s)==SW_MOVING);CHECK(sb_timer_fire(&w,1,timer,&end,&s)==SW_MOVING&&w.timer_live[0]);
 CHECK(sb_affinity(&w,1,a,1,&s)==SW_MOVING);
 CHECK(sb_move_end(&w,0,t,1,&e,&s)==SQ_OK&&w.migrations[0]==1&&sb_invariant(&w));
 CHECK(sb_move_end(&w,0,t,1,&e,&s)==SQ_STALE);
 CHECK(sb_timer_fire(&w,1,timer,&e,&s)==SQ_DUPLICATE&&!w.timer_live[0]);
 CHECK(sb_timer_fire(&w,1,timer,&e,&s)==SQ_STALE);
 CHECK(sb_dispatch(&w,2,&run,&e,&s)==SQ_OK&&run.slot==0);CHECK(sb_finish(&w,2,a,SQ_BLOCKED,&e,&s)==SQ_OK);
 CHECK(sb_prepare_idle(&w,2,1,&s)==SQ_IDLE);CHECK(sb_timer_arm(&w,0,a,&timer,&s)==SQ_OK);p.fail=1;
 CHECK(sb_timer_fire(&w,0,timer,&e,&s)==SW_SIGNAL&&!w.timer_live[0]&&w.wake.queues.queue[2].count==1);
 p.fail=0;CHECK(sb_retry(&w,0,2,&s)==SQ_OK);CHECK(sb_invariant(&w));
 CHECK(sb_create(&w,0,1,2,2,&c,&e)==SQ_OK);
 CHECK(sb_timer_arm(&w,0,c,&timer,&s)==SQ_OK);CHECK(sb_dispatch(&w,1,&run,&e,&s)==SQ_OK);CHECK(sb_finish(&w,1,c,SQ_EXITED,&e,&s)==SQ_OK);
 CHECK(sb_create(&w,0,1,2,2,&run,&e)==SW_BUSY);CHECK(sb_timer_cancel(&w,0,timer,&s)==SQ_OK);CHECK(sb_create(&w,0,1,2,2,&run,&e)==SQ_OK&&run.generation==c.generation+1);
 CHECK(sb_timer_fire(&w,0,timer,&e,&s)==SQ_STALE);CHECK(sb_wake(&w,0,c,&e,&s)==SQ_STALE);
 return 0;
}
static int balance(void){
 struct sb_world w;struct sa_port p={{0},0};struct sq_handle id[4],run;struct sq_event first,last;struct sl_stats stats={0};
 CHECK(sb_init(&w,2,4,(struct sw_port){ring,ack,&p})==SQ_OK);
 for(unsigned i=0;i<4;i++)CHECK(sb_create(&w,0,0,i,i?3:1,&id[i],&first)==SQ_OK);
 CHECK(sb_balance(&w,0,&first,&last,&stats)==SQ_OK&&first.slot==1&&last.destination==1);
 CHECK(sb_balance(&w,0,&first,&last,&stats)==SQ_OK&&first.slot==2&&last.destination==1);
 CHECK(sb_balance(&w,0,&first,&last,&stats)==SQ_IDLE&&w.wake.queues.queue[0].count==2&&w.wake.queues.queue[1].count==2&&atomic_load(&w.wake.queues.task[0].home)==0);
 for(unsigned cpu=0;cpu<2;cpu++)for(unsigned i=0;i<2;i++){CHECK(sb_dispatch(&w,cpu,&run,&first,&stats)==SQ_OK);CHECK(sb_finish(&w,cpu,run,SQ_EXITED,&first,&stats)==SQ_OK);}
 CHECK(sb_invariant(&w));return 0;
}
static int limits(void){
 struct sb_world w;struct sa_port p={{0},0};struct sq_handle id,run;struct sq_event e;struct sq_transfer t;struct sb_timer timer;struct sl_stats s={0};
 CHECK(sb_init(&w,2,1,(struct sw_port){ring,ack,&p})==SQ_OK);CHECK(sb_create(&w,0,0,0,3,&id,&e)==SQ_OK);
 w.migrations[0]=UINT64_MAX-1;CHECK(sb_move_begin(&w,0,1,id,0,&t,&e,&s)==SQ_OK);CHECK(sb_move_end(&w,0,t,1,&e,&s)==SQ_OK&&w.migrations[0]==UINT64_MAX);CHECK(sb_move_begin(&w,0,0,id,0,&t,&e,&s)==SQ_FULL);
 w.timer_serial[0]=UINT64_MAX-1;CHECK(sb_timer_arm(&w,0,id,&timer,&s)==SQ_OK&&timer.serial==UINT64_MAX);CHECK(sb_timer_cancel(&w,0,timer,&s)==SQ_OK);CHECK(sb_timer_arm(&w,0,id,&timer,&s)==SQ_FULL);
 CHECK(sb_dispatch(&w,1,&run,&e,&s)==SQ_OK);CHECK(sb_finish(&w,1,run,SQ_EXITED,&e,&s)==SQ_OK&&sb_invariant(&w));return 0;
}
static int capacity(void){
 struct sb_world w;struct sa_port p={{0},0};struct sq_handle a,b,run;struct sq_event e;struct sq_transfer t;struct sl_stats s={0};
 CHECK(sb_init(&w,2,1,(struct sw_port){ring,ack,&p})==SQ_OK);CHECK(sb_create(&w,0,0,0,3,&a,&e)==SQ_OK);CHECK(sb_create(&w,0,1,1,2,&b,&e)==SQ_OK);
 CHECK(sb_move_begin(&w,0,1,a,0,&t,&e,&s)==SQ_FULL&&sb_invariant(&w)&&w.wake.queues.queue[0].count==1);
 CHECK(sb_dispatch(&w,1,&run,&e,&s)==SQ_OK);CHECK(sb_move_begin(&w,0,1,a,0,&t,&e,&s)==SQ_FULL);CHECK(sb_finish(&w,1,b,SQ_BLOCKED,&e,&s)==SQ_OK);
 CHECK(sb_move_begin(&w,0,1,a,0,&t,&e,&s)==SQ_OK);CHECK(sb_wake(&w,1,b,&e,&s)==SQ_FULL);
 CHECK(sb_move_end(&w,0,t,0,&e,&s)==SQ_OK&&sb_invariant(&w)&&w.wake.queues.queue[0].count==1&&!w.wake.queues.queue[1].reserved);
 CHECK(sb_wake(&w,1,b,&e,&s)==SQ_OK);return 0;
}
int main(void){if(contract()||balance()||limits()||capacity())return 1;puts("SB_PASS:affinity-safe-boundary-transfer-rollback-timer-routing-pinning");return 0;}
