#include "interrupts.h"
#include <inttypes.h>
#include <stdio.h>
struct sa_seen {uint64_t sum;unsigned calls;};
static void add(void *context,uint32_t item){struct sa_seen *s=context;s->sum+=item;s->calls++;}
int main(void) {
 unsigned history;
 while(scanf("%u",&history)==1) {
  struct si_box box;si_init(&box);struct sa_seen seen={0,0};unsigned actions=history;
  for(unsigned step=0;step<6;step++) {
   unsigned action=actions%4;actions/=4;int result;uint64_t out=999;uint32_t items[]={3,5,7,11};
   if(action==0)result=si_publish(&box,items,1,&out);
   else if(action==1)result=si_publish(&box,items,4,&out);
   else if(action==2)result=si_drain(&box,box.sequence,add,&seen);
   else result=si_timeout(&box,box.sequence);
   printf("%u %u %u %d %"PRIu64" %"PRIu64" %"PRIu64" %u %u %"PRIu64" %u\n",history,step,action,result,out,box.sequence,atomic_load(&box.acknowledged),box.count,box.timed_out,seen.sum,seen.calls);
  }
 }
 return ferror(stdin)?2:0;
}
