#include "i2c_depth.h"
#ifndef IA_MUTANT
#define IA_MUTANT 0
#endif
/* Fixed 64-step restoring division, explicit target mechanism, d is positive. */
static uint64_t divide(uint64_t n,uint64_t d,unsigned up){
 uint64_t q=0,r=0;
 for(unsigned bit=64;bit>0;){
  --bit;unsigned carry=(unsigned)(r>>63);r=(r<<1)|((n>>bit)&1u);
  if(carry || r>=d){r-=d;q|=UINT64_C(1)<<bit;}
 }
 return q+(up && r!=0);
}
static uint64_t ceil_div(uint64_t n,uint64_t d){return divide(n,d,1);}
static int interval(uint64_t lo,uint64_t hi,IaInterval *out){
 IaInterval r={(uint32_t)lo,(uint32_t)hi};*out=r;return lo<=hi?IA_OK:IA_EMPTY;
}
int ia_nominal(uint32_t v,uint32_t low,uint32_t i,uint32_t t,uint32_t c,IaInterval *out){
 if(!out || !v || v>6000 || low>=v || !i || i>20000 || !t || t>1000000 || !c || c>10000)return IA_INVALID;
 uint64_t n=(uint64_t)(v-low)*1000;
 uint64_t lo=IA_MUTANT==1?divide(n,i,0):ceil_div(n,i);
 uint64_t hi=divide((uint64_t)t*10000000,(uint64_t)8473*c,0);
 return interval(lo,hi,out);
}
int ia_guaranteed(const IaEnvelope *e,IaInterval *out){
 if(!e || !out || !e->vmin_mv || e->vmax_mv<e->vmin_mv || e->vmax_mv>6000 || e->vol_mv>=e->vmin_mv ||
    !e->iol_ua || e->iol_ua>20000 || !e->rise_ns || e->rise_ns>1000000 || !e->cap_pf || e->cap_pf>10000 ||
    e->source_leak_ua>20000 || e->sink_leak_ua>20000 || e->tolerance_ppm>250000)return IA_INVALID;
 if(e->source_leak_ua>=e->iol_ua)return interval(1,0,out);
 uint64_t lo=ceil_div((uint64_t)(e->vmax_mv-e->vol_mv)*1000*1000000,
                      (uint64_t)(e->iol_ua-e->source_leak_ua)*(1000000-e->tolerance_ppm));
 uint64_t hi=divide((uint64_t)3000*e->vmin_mv*e->rise_ns*1000000,
   ((uint64_t)4*e->vmin_mv*e->cap_pf+(IA_MUTANT==2?0:(uint64_t)10*e->sink_leak_ua*e->rise_ns))*(1000000+e->tolerance_ppm),0);
 return interval(lo,hi,out);
}
int ia_arbitrate(const uint8_t bits[3][32],unsigned n,unsigned count,
                 const unsigned low[3],const unsigned high[3],IaTrace *out){
 if(!bits || !out || !low || !high || !n || n>3 || !count || count>32)return IA_INVALID;
 for(unsigned j=0;j<n;j++){
  if(!low[j] || !high[j])return IA_INVALID;
  for(unsigned k=0;k<count;k++){
   if(bits[j][k]>3)return IA_INVALID;
   if(bits[j][k]>1 && IA_MUTANT!=3)return IA_UNSUPPORTED;
  }
 }
 IaTrace r={0};unsigned active=(1u<<n)-1;r.high=high[0];
 for(unsigned j=0;j<n;j++){if(low[j]>r.low)r.low=low[j];if(high[j]<r.high)r.high=high[j];}
 for(unsigned k=0;k<count;k++){
  unsigned bus=1;
  for(unsigned j=0;j<n;j++)if(active&(1u<<j))bus&=bits[j][k]&1;
  r.bus[k]=(uint8_t)bus;
  for(unsigned j=0;j<n;j++)if((active&(1u<<j)) && bits[j][k]!=bus){r.lost[k]|=(uint8_t)(1u<<j);if(IA_MUTANT!=4)active&=~(1u<<j);}
  r.active[k]=(uint8_t)active;
 }
 r.survivors=(uint8_t)active;*out=r;return IA_OK;
}
