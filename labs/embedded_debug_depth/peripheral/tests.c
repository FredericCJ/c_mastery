#include "peripheral.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { printf("DP_CHECK:%u:%s\n",__LINE__,#x);return 1;} } while (0)
int main(void) {
  unsigned r=99;
  CHECK(dp_predict(31,&r)==DP_OK&&r==15);
  CHECK(dp_predict(31^DP_ROUTE,&r)==DP_OK&&r==3);
  CHECK(dp_predict(31^DP_MODE,&r)==DP_OK&&r==3);
  CHECK(dp_predict(31^DP_GATE,&r)==DP_OK&&r==0);
  CHECK(dp_predict(31^DP_SEQUENCE,&r)==DP_OK&&r==1);
  CHECK(dp_predict(31^DP_TRIGGER,&r)==DP_OK&&r==7);
  CHECK(dp_predict(32,&r)==DP_INVALID&&r==7);
  struct dp_calibration c={1000,100,10,2,DP_MODEL};struct dp_correlation o={9,9,9,9},before=o;
  CHECK(dp_correlate(&c,105,1049,1051,&o)==DP_OK&&o.earliest_ns==1048&&o.latest_ns==1052&&o.compatible&&o.provenance==DP_MODEL);
  CHECK(dp_correlate(&c,105,1053,1060,&o)==DP_OK&&!o.compatible);
  o=before;c.reference_ns=UINT64_MAX;
  CHECK(dp_correlate(&c,101,0,1,&o)==DP_RANGE&&!memcmp(&o,&before,sizeof o));
  c.reference_ns=0;c.ns_per_tick=UINT64_MAX;
  CHECK(dp_correlate(&c,102,0,1,&o)==DP_RANGE&&!memcmp(&o,&before,sizeof o));
  c.ns_per_tick=1;CHECK(dp_correlate(&c,100,0,1,&o)==DP_RANGE);
  CHECK(dp_correlate(&c,99,0,1,&o)==DP_INVALID);
  CHECK(dp_spi_end(DP_MASTER_DUPLEX,1,1,1,1,0,0)==DP_WAIT);
  CHECK(dp_spi_end(DP_MASTER_DUPLEX,1,0,1,1,0,0)==DP_OK);
  CHECK(dp_spi_end(DP_SLAVE_RECEIVE,1,1,1,0,0,0)==DP_OK);
  CHECK(dp_spi_end(DP_SLAVE_TRANSMIT,1,1,0,1,7,8)==DP_WAIT);
  CHECK(dp_spi_end(DP_SLAVE_TRANSMIT,1,1,0,1,8,8)==DP_OK);
  CHECK(dp_spi_end(DP_SLAVE_TRANSMIT,1,0,0,0,8,8)==DP_WAIT);
  CHECK(dp_spi_end(DP_SLAVE_TRANSMIT,1,1,0,1,8,0)==DP_INVALID);
  struct dp_progress p={0};
  CHECK(dp_progress(12,127,0,3,6,&p)==DP_OK&&p.reset_at==3&&p.remaining==3&&!p.observed_at);
  CHECK(dp_progress(12,127,1,3,6,&p)==DP_OK&&p.dma_complete_at==6&&p.observed_at==8&&!p.reset_at&&p.fed==5);
  CHECK(dp_progress(12,0,0,3,6,&p)==DP_OK&&p.observed_at==6&&!p.reset_at);
  puts("DP_PASS:core");return 0;
}
