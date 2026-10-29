#include "interval.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
 unsigned op,bits,other;int32_t al,ah,bl,bh;
 while(scanf("%u %u %"SCNd32" %"SCNd32" %"SCNd32" %"SCNd32" %u",&op,&bits,&al,&ah,&bl,&bh,&other)==7) {
  struct ni_interval a={al,ah,bits},b={bl,bh,other},out={123,456,7};int rc;
  if(op<=5)rc=ni_binary(op,a,b,&out);
  else if(op==6)rc=ni_square(a,&out);
  else if(op==7)rc=ni_rescale(a,other,&out);
  else rc=ni_ratio(al,ah,bits,&out);
  printf("%d %"PRId32" %"PRId32" %u\n",rc,out.lo,out.hi,out.bits);
 }
 return 0;
}
