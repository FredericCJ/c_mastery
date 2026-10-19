#include "release.h"
#include <inttypes.h>
#include <stdio.h>
int main(void){
 for(unsigned word=0;word<4096;word++){
  DtTrace q={0};q.next=10;unsigned bits=word;uint32_t last=0;
  for(unsigned i=0;i<12;i++){
   unsigned op=bits&1;bits>>=1;DtRecord r={99,0,0,0};int s=op?dt_emit(&q,1,i,i):dt_take(&q,&r);if(!op&&s==DT_OK)last=r.sequence;
   printf("Q %u %u %d %u %u %u %u\n",word,i,s,q.next,q.dropped,q.count,last);
  }
 }
 for(unsigned i=0;i<1024;i++){
  DtInterval v[2]={{i%256,(i*3)%256,(i%7)+1,8},{(i*7)%256,(i*13)%256,(i%13)+1,8}};DtDuration r={0};int s=dt_duration(v,2,&r);
  printf("D %u %d %"PRIu64" %"PRIu64"\n",i,s,r.lower_ns,r.upper_ns);
 }
 return 0;
}
