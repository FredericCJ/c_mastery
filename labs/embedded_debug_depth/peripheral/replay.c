#include "peripheral.h"
#include <inttypes.h>
#include <stdio.h>
int main(void) {
  for(unsigned c=0;c<32;c++){unsigned r=99;int s=dp_predict(c,&r);printf("P %u %d %u\n",c,s,r);}
  for(unsigned h=0;h<256;h++)for(unsigned f=0;f<2;f++)for(unsigned b=1;b<=4;b++){
    struct dp_progress r={0};int s=dp_progress(8,h,f,b,6,&r);
    printf("H %u %u %u %d %u %u %u %u %u\n",h,f,b,s,r.dma_complete_at,r.reset_at,r.observed_at,r.fed,r.remaining);
  }
  for(unsigned i=0;i<1024;i++){
    struct dp_calibration c={1000,100,(i%7)+1,i%5,i%3};struct dp_correlation r={0};uint64_t tick=100+(i%11),begin=1000+(i%83),end=begin+3;
    int s=dp_correlate(&c,tick,begin,end,&r);
    printf("C %u %d %"PRIu64" %"PRIu64" %u %u\n",i,s,r.earliest_ns,r.latest_ns,r.compatible,r.provenance);
  }
  for(unsigned mode=0;mode<3;mode++)for(unsigned bits=0;bits<16;bits++)for(unsigned elapsed=0;elapsed<10;elapsed++)
    printf("S %u %u %u %d\n",mode,bits,elapsed,dp_spi_end(mode,bits&1,(bits>>1)&1,(bits>>2)&1,(bits>>3)&1,elapsed,8));
  return 0;
}
