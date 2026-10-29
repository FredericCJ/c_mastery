#include "reduction.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SR_CHECK:%u:%s\n",__LINE__,#x);return 1;}}while(0)
static int boundaries(void){
 for(size_t n=0;n<=260;n++)for(unsigned workers=1;workers<=4;workers++){
  struct sr_range range;size_t previous=0,small=n/workers,large=small+(n%workers!=0);
  for(unsigned i=0;i<workers;i++){CHECK(sr_partition(n,workers,i,&range)==SR_OK);CHECK(range.begin==previous&&range.end>=range.begin&&range.end-range.begin>=small&&range.end-range.begin<=large);previous=range.end;}
  CHECK(previous==n);
 }
 struct sr_range r={17,19};CHECK(sr_partition(SIZE_MAX,4,3,&r)==SR_OK&&r.end==SIZE_MAX);r=(struct sr_range){17,19};CHECK(sr_partition(10,0,0,&r)==SR_INVALID&&r.begin==17&&r.end==19);CHECK(sr_partition(10,3,3,&r)==SR_INVALID);return 0;
}
static int integers(void){
 uint64_t values[257];atomic_uint visits[257];for(unsigned i=0;i<257;i++)values[i]=(uint64_t)(i+1)*(i+1);
 {uint64_t original[10],partials[3]={10,18,27};for(unsigned i=0;i<10;i++)original[i]=i+1;struct sr_world w;struct sr_result r;CHECK(sr_init(&w,original,visits,10,3,SR_UNSIGNED)==SR_OK);for(unsigned i=0;i<3;i++){CHECK(sr_work(&w,i,SIZE_MAX)==SR_OK);CHECK(w.slot[i].value.unsigned_sum==partials[i]);}CHECK(sr_finish(&w,&r)==SR_OK&&r.unsigned_sum==55);}
 for(unsigned workers=1;workers<=4;workers++)for(unsigned repeat=0;repeat<3;repeat++){
  struct sr_world w;struct sr_result result={0};result.unsigned_sum=99;CHECK(sr_init(&w,values,visits,257,workers,SR_UNSIGNED)==SR_OK);CHECK(sr_finish(&w,&result)==SR_INCOMPLETE&&result.unsigned_sum==99);
  for(unsigned i=0;i<workers;i++){CHECK(sr_work(&w,i,SIZE_MAX)==SR_OK);CHECK(sr_work(&w,i,SIZE_MAX)==SR_BUSY);}
  CHECK(sr_finish(&w,&result)==SR_OK&&result.unsigned_sum==5691265&&result.count==257);for(unsigned i=0;i<257;i++)CHECK(atomic_load(visits+i)==1);
 }
 for(unsigned workers=1;workers<=4;workers++){struct sr_world w;struct sr_result r;CHECK(sr_init(&w,NULL,NULL,0,workers,SR_UNSIGNED)==SR_OK);for(unsigned i=0;i<workers;i++)CHECK(sr_work(&w,i,SIZE_MAX)==SR_OK);CHECK(sr_finish(&w,&r)==SR_OK&&r.count==0&&r.unsigned_sum==0);}
 uint64_t u[]={UINT64_MAX,1};int64_t a[]={INT64_MAX,1},b[]={INT64_MIN,-1};
 for(unsigned workers=1;workers<=2;workers++)for(unsigned kind=0;kind<3;kind++){
  struct sr_world w;struct sr_result r={0};r.unsigned_sum=71;CHECK(sr_init(&w,kind==0?(const void *)u:kind==1?(const void *)a:(const void *)b,visits,2,workers,kind==0?SR_UNSIGNED:SR_SIGNED)==SR_OK);for(unsigned i=0;i<workers;i++){int code=sr_work(&w,i,SIZE_MAX);CHECK(code==SR_OK||code==SR_OVERFLOW);}CHECK(sr_finish(&w,&r)==SR_OVERFLOW&&r.unsigned_sum==71);
 }
 struct sr_world w;struct sr_result r={0};r.unsigned_sum=83;CHECK(sr_init(&w,values,visits,257,4,SR_UNSIGNED)==SR_OK);for(unsigned i=0;i<4;i++)CHECK(sr_work(&w,i,i==2?3:SIZE_MAX)==(i==2?SR_FAILED:SR_OK));CHECK(sr_finish(&w,&r)==SR_FAILED&&r.unsigned_sum==83);
 int64_t good[]={INT64_MAX,-INT64_MAX,-7,2,5};CHECK(sr_init(&w,good,visits,5,1,SR_SIGNED)==SR_OK);CHECK(sr_work(&w,0,SIZE_MAX)==SR_OK);CHECK(sr_finish(&w,&r)==SR_OK&&r.signed_sum==0);return 0;
}
static int ordered(void){
 uint8_t bytes[257];atomic_uint visits[257];uint64_t expected=0,power=1,bins[16]={0};for(unsigned i=0;i<257;i++){bytes[i]=(uint8_t)((i*7+3)%16);expected=expected*257+bytes[i]+1;power*=257;bins[bytes[i]]++;}
 for(unsigned workers=1;workers<=4;workers++){struct sr_world w;struct sr_result r;CHECK(sr_init(&w,bytes,visits,257,workers,SR_BYTES)==SR_OK);for(unsigned i=workers;i;i--)CHECK(sr_work(&w,i-1,SIZE_MAX)==SR_OK);CHECK(sr_finish(&w,&r)==SR_OK&&r.hash==expected&&r.power==power&&r.count==257);CHECK(memcmp(r.bins,bins,sizeof bins)==0);}
 puts("SR_ORDER:noncommutative fixed-range order matches sequential bytes; histogram merges counts");return 0;
}
int main(void){if(boundaries()||integers()||ordered())return 1;puts("SR_PASS:partition-overflow-ownership-failure-order");return 0;}
