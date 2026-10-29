#include "mcu.h"
#include "order.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"VARIANT:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static _Alignas(8) uint32_t stack[3][160];
static int transitions(void){
 for(unsigned reverse=0;reverse<2;reverse++){
  struct mf_mf m;mf_handle_id a,b;C(mf_init(&m,stack[0],160,CMX_RR)==CMX_OK);
  C(mf_add(&m,stack[1],160,0x101,0x201,1,1,2,&a)==CMX_OK&&mf_add(&m,stack[2],160,0x101,0x201,1,1,2,&b)==CMX_OK);
  for(unsigned n=0;n<20;n++){int id=reverse?mf_schedule_reverse(&m):mf_schedule(&m);C(id==(int)(reverse?2-n%2:1+n%2));}
 }
 for(unsigned sequence=0;sequence<7776;sequence++){
  struct mf_mf m;C(mf_init(&m,stack[0],160,CMX_RR)==CMX_OK);mf_handle_id h;
  C(mf_add(&m,stack[1],160,0x101,0x201,1,1,2,&h)==CMX_OK);
  unsigned code=sequence;
  for(unsigned step=0;step<5;step++){
   struct mf_mf before=m;unsigned op=code%6;code/=6;enum cmx_result r=CMX_OK;
   if(op==0){int picked=mf_schedule(&m);C(picked==0||picked==1);C(m.kernel.task[picked].state==CMX_RUNNING);}
   else if(op==1)r=cmx_sleep(&m.kernel,1,cmx_at(m.kernel.now+1));
   else if(op==2)cmx_tick(&m.kernel,m.kernel.now+1);
   else if(op==3)r=cmx_cancel(&m.kernel,1);
   else if(op==4)r=mf_exit(&m,h,3);
   else r=mf_reap(&m,h);
   if(r==CMX_INVALID)C(memcmp(&m,&before,sizeof m)==0);
   C(mf_invariant(&m));
  }
 }
 puts("MF_VARIANT:7776 sequences,38880 post-operation invariant checks");return 0;
}
static int orders(void){
 for(unsigned reverse=0;reverse<2;reverse++)for(unsigned seed=0;seed<64;seed++){
  struct mf_order list,heap;uint32_t epoch=UINT32_MAX-8;mf_order_init(&list,epoch,0,reverse);mf_order_init(&heap,epoch,1,reverse);
  unsigned lcg=seed+1;struct mf_order_entry expected[16];
  for(unsigned i=0;i<16;i++){lcg=lcg*1664525u+1013904223u;struct mf_order_entry e={epoch+(lcg>>28),i};expected[i]=e;
   C(mf_order_push(&list,e)&&mf_order_push(&heap,e));}
  struct mf_order snapshot=heap;C(!mf_order_push(&heap,(struct mf_order_entry){0,99})&&!memcmp(&heap,&snapshot,sizeof heap));
  uint64_t li=list.comparisons,lm=list.moves,hi=heap.comparisons,hm=heap.moves;
  /* Independent selection oracle over immutable input, not the queue algorithm. */
  unsigned used=0;for(unsigned n=0;n<16;n++){int best=-1;for(unsigned i=0;i<16;i++)if(!(used&(1u<<i))){
   if(best<0||(uint32_t)(expected[i].at-epoch)<(uint32_t)(expected[best].at-epoch)||
    (expected[i].at==expected[best].at&&(reverse?expected[i].identity>expected[best].identity:expected[i].identity<expected[best].identity)))best=(int)i;}
   C(best>=0);used|=1u<<(unsigned)best;struct mf_order_entry a,b;C(mf_order_pop(&list,&a)&&mf_order_pop(&heap,&b));
   C(a.at==expected[best].at&&a.identity==expected[best].identity&&b.at==a.at&&b.identity==a.identity);}
  struct mf_order_entry out;C(!mf_order_pop(&list,&out)&&!mf_order_pop(&heap,&out));
  if(seed==0)printf("MF_COST:reverse=%u,list_insert=%llu/%llu,heap_insert=%llu/%llu,list_expire=%llu/%llu,heap_expire=%llu/%llu\n",reverse,(unsigned long long)li,(unsigned long long)lm,(unsigned long long)hi,(unsigned long long)hm,(unsigned long long)(list.comparisons-li),(unsigned long long)(list.moves-lm),(unsigned long long)(heap.comparisons-hi),(unsigned long long)(heap.moves-hm));
 }
 puts("MF_VARIANT:128 queue histories,2048 independently ordered expiries");return 0;
}
static int publication(void){
 /* Event-loop exploration: irq publication is indivisible to a task because
  * no dispatch occurs inside the masked ISR. Pending is coalesced; count is
  * authoritative. The incorrect publish-before-data prefix is observable. */
 for(unsigned arrival=0;arrival<7;arrival++){
  unsigned payload=0,count=0,pending=0,seen=0,masked=0,irq_pending=0;
  for(unsigned point=0;point<8;point++){
   if(point==1)masked=1;
   if(point==6)masked=0;
   if(point==arrival)irq_pending=1;
   if(irq_pending&&!masked){payload=0x1234;count++;pending=1;irq_pending=0;}
   if(!masked&&pending){C(count==1&&payload==0x1234);seen++;pending=0;count--;}
  }
  C(seen==1&&!count&&!irq_pending);
 }
 unsigned payload=0,published=1;C(published&&payload!=0x1234);payload=0x1234;
 /* Single producer reservation fails when nested entry reads the same head:
  * B publishes slot0, outer A then overwrites it and republishes head1. */
 unsigned head=0,slotA=head,slotB=head,data[2]={0};data[slotB]=22;head=slotB+1;data[slotA]=11;head=slotA+1;
 C(head==1&&data[0]==11);puts("MF_VARIANT:7 publication boundaries; early-publication and nested-reservation counterexamples");return 0;
}
static int cancellation(void){
 /* A protected pair sums10. Cancellation may be requested at every phase,
  * but publishing or releasing ownership is legal only after commit/rollback. */
 for(unsigned arrival=0;arrival<5;arrival++){
  int a=4,b=6,old_a=a,old_b=b;unsigned cancel=0,owned=1,cleanup=0,committed=0;
  for(unsigned phase=0;phase<5;phase++){
   if(arrival==phase)cancel=1;
   if(phase==1)a=7;
   if(phase==2)b=3;
   if(phase==3){if(cancel){a=old_a;b=old_b;}else committed=1;C(a+b==10);owned=0;cleanup++;}
   if(!owned)C(a+b==10);
  }
  C(cleanup==1&&(!committed||a==7));
 }
 /* Borrowed asynchronous storage is retained after cancellation request.
  * Only device completion/reset acknowledgement permits release. */
 for(unsigned cancel_at=0;cancel_at<4;cancel_at++){
  unsigned dma=1,caller=1,abandoned=0,release=0;
  for(unsigned phase=0;phase<4;phase++){
   if(phase==cancel_at)abandoned=1;
   if(phase==2)dma=0;
   if(abandoned&&!dma){caller=0;release=1;}
   C(!release||!dma);if(phase<2)C(caller);
  }
  C(!caller&&!dma&&release);
 }
 puts("MF_VARIANT:5 mutation cancellation points;4 borrowed-storage abandon/completion orders");return 0;
}
int main(void){if(transitions()||orders()||publication()||cancellation())return 1;puts("MF_PASS:variants");return 0;}
