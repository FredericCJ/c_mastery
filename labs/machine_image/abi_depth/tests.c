#include "api.h"
#include "machine.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x,s) do{if(!(x)){fprintf(stderr,"AD_CHECK:%s\n",s);return 1;}}while(0)
static volatile unsigned trace_calls;
__attribute__((noinline)) void ad_trace_leaf(void){trace_calls++;}
int main(int argc,char **argv){
 CHECK(argc==2,"case required");
 if(!strcmp(argv[1],"values")){
  const uint32_t a[]={1,UINT32_MAX,3};CHECK(ad_sum64(a,3)==UINT64_C(4294967299),"zero extending sum");
  CHECK(ad_sum32(a,3)==3,"modulo accumulator");CHECK(ad_sum64(NULL,0)==0&&ad_sum32(NULL,0)==0,"empty no access");
  const uint32_t singleton=UINT32_MAX;CHECK(ad_sum64(&singleton,1)==UINT32_MAX,"singleton maximum zero extension");
  CHECK(sizeof(struct ad_cell)==8&&offsetof(struct ad_cell,y)==4,"measured cell layout");
  CHECK(0x1000+3*sizeof(struct ad_cell)+offsetof(struct ad_cell,y)==0x101c,"synthetic member address");
 }else if(!strcmp(argv[1],"aggregate")){
  struct ad_pair p=ad_pair(UINT64_MAX,7);struct ad_triple t=ad_triple(2,3,5);struct ad_mixed m=ad_mixed(1.5,UINT64_MAX);
  CHECK(p.a==UINT64_MAX&&p.b==7,"integer pair return");CHECK(t.a==2&&t.b==3&&t.c==5,"caller owned triple result");
  CHECK(m.real==2.0&&m.integer==0,"mixed register class return");
  CHECK(ad_nine(1,2,3,4,5,6,7,8,9,0.5)==285,"nine integer and separate float resources");
 }else if(!strcmp(argv[1],"hostile")){
  uint32_t a[]={1,UINT32_MAX,3};
  for(unsigned selected=0;selected<6;selected++)for(unsigned pass=0;pass<64;pass++){
   uint64_t seeds[6];for(unsigned k=0;k<6;k++)seeds[k]=(UINT64_C(0x9123456789abcdef)>>(k+1))^(k==selected?(UINT64_C(1)<<pass):0)^(k*37u);
   for(unsigned n=0;n<=3;n++){
    struct ad_observation o={0};ad_hostile(n?a:NULL,n,seeds,&o);
    uint64_t expected=n==0?0:n==1?1:n==2?UINT64_C(4294967296):UINT64_C(4294967299);
    CHECK(o.result==expected,"arithmetic independent of preservation");CHECK(o.preserved==1&&o.mask==0,"callee preserved sentinels");
    CHECK(o.before==0&&o.entry==8,"nested call alignment");
   }
  }
 }else if(!strcmp(argv[1],"call8")){
  CHECK(mi_mix(UINT64_MAX,0,0,0,0,0,0,0)==UINT64_MAX&&mi_mix(0,UINT64_MAX,0,0,0,0,0,0)==UINT64_MAX-1,"high bit weighted modulo arguments");
  for(unsigned i=0;i<128;i++){CHECK(ad_call8()==204,"eight argument sum");CHECK(ad_before8==0&&ad_entry8==8,"precall versus entry alignment");CHECK(ad_seventh==7&&ad_eighth==8,"return address shifts stack arguments");}
 }else if(!strcmp(argv[1],"inline")){
  uint32_t values[]={0,1,2,7,UINT32_MAX,UINT32_C(0x80000000)};
  for(unsigned a=0;a<6;a++)for(unsigned b=0;b<6;b++)CHECK(ad_inline_add(values[a],values[b])==(uint32_t)((uint64_t)values[a]+values[b]),"early clobber additive contract");
  for(unsigned i=0;i<256;i++)CHECK(ad_inline_add(i,i)==2*i,"equal valued loop allocation");
  uint32_t v=UINT32_MAX;CHECK(ad_named_memory(&v)==0&&v==0,"precise memory operand");CHECK(ad_opaque_memory(&v)==1&&v==1,"opaque pointer clobber");
 }else if(!strcmp(argv[1],"cfi")){ad_cfi(1);ad_cfi(0);CHECK(trace_calls==1,"normal call and early return paths");}
 else return 2;
 puts("AD_PASS");return 0;
}
