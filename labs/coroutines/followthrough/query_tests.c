#include "query_follow.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"CQ_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static size_t oracle(const CORow*r,size_t n,unsigned*k,uint32_t*s){size_t out=0,i=0;while(i<n){unsigned key=r[i].key,kept=0;uint64_t total=0;do{if(r[i].value>=0){kept=1;total+=(uint32_t)r[i].value;}i++;}while(i<n&&r[i].key==key);if(kept){k[out]=key;s[out++]=(uint32_t)total;}}return out;}
int main(void){
 const CORow alphabet[]={{0,-1},{0,1},{1,-1},{1,2}};size_t power=1,total_cases=0;
 for(size_t n=0;n<=6;n++,power*=4)for(size_t code=0;code<power;code++){
  CORow rows[6];size_t v=code;for(size_t i=0;i<n;i++){rows[i]=alphabet[v%4];v/=4;}
  unsigned keys[6];uint32_t sums[6];size_t expected=oracle(rows,n,keys,sums);CFVolcano q;COQuery fused;C(cf_volcano_open(&q,rows,n)==0&&co_query_open(&fused,rows,n)==0);
  for(size_t i=0;i<=expected;i++){unsigned k=91,k2=91;uint32_t s=92,s2=92;int r=cf_volcano_next(&q,&k,&s),r2=co_query_next(&fused,&k2,&s2);C(r==r2&&k==k2&&s==s2&&q.scan.index==fused.index);C(r==(i<expected?CO_OK:CO_DONE));if(i<expected)C(k==keys[i]&&s==sums[i]);else C(k==91&&s==92);}
  C(cf_volcano_close(&q)==0&&cf_volcano_close(&q)==0);unsigned k=91;uint32_t s=92;C(cf_volcano_next(&q,&k,&s)==CO_STATE);
  for(size_t split=0;split<=n;split++){
   CFBatch b;C(cf_batch_open(&b,rows,n)==0);size_t out=0,avail=split;
   for(unsigned guard=0;guard<100;guard++){
    k=91;s=92;int blocked=cf_batch_poll(&b,avail,avail==n,0,&k,&s);C(blocked==CO_PENDING||blocked==CO_DONE);C(k==91&&s==92);
    int r=cf_batch_poll(&b,avail,avail==n,1,&k,&s);if(r==CO_DONE)break;
    if(r==CO_OK){C(out<expected&&k==keys[out]&&s==sums[out]);out++;}
    else {C(r==CO_PENDING&&k==91&&s==92);if(avail<n)avail++;}
   }C(out==expected&&b.committed==n);C(cf_batch_cancel(&b)==0&&cf_batch_cancel(&b)==0);C(cf_batch_poll(&b,n,1,1,&k,&s)==CO_STATE);
  }
  total_cases++;
 }
 CORow overflow[]={{1,-1},{2,INT32_MAX},{2,INT32_MAX},{2,2},{3,1}};CFVolcano q;C(cf_volcano_open(&q,overflow,5)==0);CFVolcano before=q;unsigned k=91;uint32_t s=92;C(cf_volcano_next(&q,&k,&s)==CO_RANGE&&!memcmp(&q,&before,sizeof q)&&k==91&&s==92);
 CFBatch b;C(cf_batch_open(&b,overflow+1,3)==0);C(cf_batch_poll(&b,1,0,1,&k,&s)==CO_PENDING);CFBatch old=b;C(cf_batch_poll(&b,3,1,1,&k,&s)==CO_RANGE&&!memcmp(&b,&old,sizeof b)&&k==91&&s==92);
 CORow in[]={{2,7},{0,9},{2,8},{1,3}},out[4]={{99,99}};size_t used=71;
 C(cf_order(in,4,0,out,4,&used)==CO_PENDING&&out[0].key==99&&used==71);C(cf_order(in,4,1,out,3,&used)==CO_RANGE&&out[0].key==99&&used==71);
 C(cf_order(in,4,1,out,4,&used)==CO_OK&&used==4&&out[0].value==9&&out[1].value==3&&out[2].value==7&&out[3].value==8);
 printf("CQ_CASES:%zu\n",total_cases);return 0;
}
