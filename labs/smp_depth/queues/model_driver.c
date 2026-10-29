/* Hosted verification driver, never linked into the freestanding guest. */
#include "queues.h"
#include <stdio.h>
#include <inttypes.h>
int main(void) {
 unsigned coarse;if(scanf("%u",&coarse)!=1)return 2;
 struct sq_world w;struct sq_handle ids[3],running;struct sq_transfer tickets[3]={0};struct sq_event e;struct sl_stats stats={0};
 if(sq_init(&w,2,2,coarse)!=SQ_OK)return 3;
 for(unsigned i=0;i<3;i++)if(sq_create(&w,0,i/2,i,&ids[i],&e)!=SQ_OK)return 4;
 unsigned op,a,b,slot,choice;
 while(scanf("%u%u%u%u%u",&op,&a,&b,&slot,&choice)==5) {
  if(a>1||b>1||slot>2)return 5;
  int r;
  if(op==0)r=sq_dispatch(&w,a,&running,&e,&stats);
  else if(op==1)r=sq_finish(&w,a,ids[slot],choice,&e,&stats);
  else if(op==2)r=sq_wake(&w,a,b,ids[slot],&e,&stats);
  else if(op==3)r=sq_transfer_begin(&w,a,a,b,ids[slot],&tickets[slot],&e,&stats);
  else if(op==4)r=sq_transfer_end(&w,a,tickets[slot],(int)choice,&e,&stats);
  else return 6;
  printf("%d %d",r,sq_invariant(&w));
  for(unsigned c=0;c<2;c++){printf(" %u %u %u",w.queue[c].count,w.queue[c].reserved,w.queue[c].current);for(unsigned j=0;j<w.queue[c].count;j++)printf(" %u",w.queue[c].slots[j]);}
  for(unsigned i=0;i<3;i++)printf(" %u %u %"PRIu64,atomic_load(&w.task[i].state),atomic_load(&w.task[i].home),w.task[i].revision);
  putchar('\n');
 }
 return ferror(stdin)?7:0;
}
