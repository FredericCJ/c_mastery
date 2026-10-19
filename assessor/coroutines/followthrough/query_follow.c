#include "query_follow.h"
#ifndef CQ_MUTANT
#define CQ_MUTANT 0
#endif
/* Separate scan/mark/group operators retain group boundaries on rejected rows. */
static int scan_peek(CFVolcano*q,CORow*out){q->scan_calls++;if(q->scan.index==q->scan.count)return CO_DONE;*out=q->scan.rows[q->scan.index];return CO_OK;}
static int mark(CFVolcano*q,CORow r){q->predicate_calls++;return CQ_MUTANT==1||r.value>=0;}
int cf_volcano_open(CFVolcano*q,const CORow*r,size_t n){if(!q||(!r&&n))return CO_INVALID;if(n>64)return CO_RANGE;*q=(CFVolcano){{r,n,0,0},0,0};return CO_OK;}
int cf_volcano_next(CFVolcano*q,unsigned*key,uint32_t*sum){
 if(!q||!key||!sum)return CO_INVALID;if(q->scan.closed)return CO_STATE;CFVolcano t=*q;CORow row;
 while(scan_peek(&t,&row)==CO_OK){unsigned k=row.key,kept=0;uint32_t total=0;
  do{if(mark(&t,row)){if((uint32_t)row.value>UINT32_MAX-total)return CO_RANGE;total+=(uint32_t)row.value;kept=1;}t.scan.index++;}while(scan_peek(&t,&row)==CO_OK&&row.key==k);
  if(kept){*q=t;*key=k;*sum=total;return CO_OK;}
 }
 *q=t;return CO_DONE;
}
int cf_volcano_close(CFVolcano*q){if(!q)return CO_INVALID;q->scan.rows=NULL;q->scan.closed=1;return CO_OK;}
int cf_batch_open(CFBatch*q,const CORow*r,size_t capacity){if(!q||(!r&&capacity))return CO_INVALID;if(capacity>64)return CO_RANGE;*q=(CFBatch){.rows=r,.capacity=capacity};return CO_OK;}
int cf_batch_poll(CFBatch*q,size_t available,unsigned eof,unsigned ready,unsigned*key,uint32_t*sum){
 if(!q||!key||!sum||eof>1||ready>1||available>q->capacity||available<q->available||(q->eof&&(!eof||available!=q->available)))return CO_INVALID;
 if(q->closed)return CO_STATE;CFBatch t=*q;t.available=available;t.eof=eof;
 for(;;){
  if(!t.active){if(t.scan==available){*q=t;return eof?CO_DONE:CO_PENDING;}t.key=t.rows[t.scan].key;t.active=1;t.kept=0;t.sum=0;}
  while(t.scan<available){CORow r=t.rows[t.scan];if(r.key!=t.key){t.complete=1;break;}
   if(r.value>=0){if((uint32_t)r.value>UINT32_MAX-t.sum)return CO_RANGE;t.sum+=(uint32_t)r.value;t.kept=1;}t.scan++;
  }
  if(t.scan==available&&eof)t.complete=1;
  if(!t.complete){*q=t;return CO_PENDING;}
  if(t.kept){if(!ready&&CQ_MUTANT!=2){*q=t;return CO_PENDING;}*key=t.key;*sum=t.sum;t.committed=t.scan;t.active=t.complete=0;*q=t;return CO_OK;}
  t.committed=t.scan;t.active=t.complete=0;
 }
}
int cf_batch_cancel(CFBatch*q){if(!q)return CO_INVALID;q->rows=NULL;q->closed=1;q->active=q->complete=0;return CO_OK;}
int cf_order(const CORow*in,size_t n,unsigned eof,CORow*out,size_t cap,size_t*used){
 if((!in&&n)||!out||!used||eof>1)return CO_INVALID;if(n>64||cap<n)return CO_RANGE;if(!eof)return CO_PENDING;
 for(size_t i=0;i<n;i++)out[i]=in[i];
 for(size_t i=1;i<n;i++){CORow value=out[i];size_t j=i;while(j&&out[j-1].key>value.key){out[j]=out[j-1];j--;}out[j]=value;}
 *used=n;return CO_OK;
}
