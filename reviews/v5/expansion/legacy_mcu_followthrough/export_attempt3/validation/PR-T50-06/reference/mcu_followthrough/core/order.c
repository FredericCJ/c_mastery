#include "order.h"
void mf_order_init(MfOrder *q,uint32_t epoch,unsigned heap,unsigned reverse){*q=(MfOrder){0};q->epoch=epoch;q->heap=heap;q->reverse_ties=reverse;}
static int before(MfOrder *q,MfOrderEntry a,MfOrderEntry b){q->comparisons++;uint32_t da=a.at-q->epoch,db=b.at-q->epoch;
 return da!=db?da<db:q->reverse_ties?a.identity>b.identity:a.identity<b.identity;}
int mf_order_push(MfOrder *q,MfOrderEntry e){if(q->count==16||(uint32_t)(e.at-q->epoch)>=0x80000000u)return 0;
 unsigned i=q->count++;if(q->heap){while(i){unsigned parent=(i-1)/2;if(!before(q,e,q->entries[parent]))break;q->entries[i]=q->entries[parent];q->moves++;i=parent;}}
 else{while(i&&before(q,e,q->entries[i-1])){q->entries[i]=q->entries[i-1];q->moves++;i--;}}
 q->entries[i]=e;q->moves++;return 1;}
int mf_order_pop(MfOrder *q,MfOrderEntry *out){if(!q->count)return 0;*out=q->entries[0];MfOrderEntry last=q->entries[--q->count];
 if(q->heap){unsigned i=0;while(i*2+1<q->count){unsigned c=i*2+1;if(c+1<q->count&&before(q,q->entries[c+1],q->entries[c]))c++;
  if(!before(q,q->entries[c],last))break;
  q->entries[i]=q->entries[c];q->moves++;i=c;}
  if(q->count){q->entries[i]=last;q->moves++;}}
 else{for(unsigned i=0;i<q->count;i++){q->entries[i]=q->entries[i+1];q->moves++;}}
 return 1;}
