#include "model.h"
#ifndef CB_GUEST
#include <stdio.h>
#include <stdlib.h>
#endif
#define CHECK(x) do{if(!(x))return __LINE__;}while(0)
static CBModel m;
int cb_suite(void){
 const unsigned sizes[5]={32,64,128,256,512};
 for(unsigned z=0;z<5;++z){
  cb_model_init(&m,sizes[z]);CBIO x=cb_model_io(&m);
  CHECK(cb_boot(x,7));CHECK(!m.bad&&!m.enabled_before_invalidation);
  CHECK(m.attrs[7]==CB_M7_RASR&&m.bases[7]==CB_LINE);
  for(unsigned i=0;i<7;++i)CHECK(!m.attrs[i]);
  CHECK(m.dc_writes==sizes[z]*4u&&m.clean_writes==1&&m.line_writes==8);
  CHECK(m.release_writes==1&&m.release_generation==7&&!m.dirty);
  CHECK(m.memory[0]==7);for(unsigned i=1;i<8;++i)CHECK(!m.memory[i]);
  uint32_t old=m.writes;CHECK(!cb_boot(x,7)&&old==m.writes);
  /* M4 has private system registers, but observes the same RAM and RCC. */
  cb_model_set(&m,0xE000ED14u,0);cb_model_set(&m,0xE000ED94u,0);
  uint32_t line=m.line_writes;CHECK(cb_peer(x,7));CHECK(m.line_writes==line);
  CHECK(m.attrs[7]==CB_M4_RASR);
  cb_model_set(&m,0xE000ED94u,0);CHECK(!cb_peer(x,8));CHECK(m.memory[0]==7&&m.line_writes==line);
 }
 /* Every unsupported geometry/ownership premise refuses before release. */
 const uint32_t badaddr[]={0xE000ED14,0xE000ED94,0x580244A0,0xE000ED90,0xE000ED80,0x58026400,0x58000704,0x52002048,0xE000ED90};
 const uint32_t badval[]={0x10000,1,8,7u<<8,0xF00FE018,0x80000000,1,0x820,9u<<8};
 for(unsigned i=0;i<9;++i){cb_model_init(&m,128);cb_model_set(&m,badaddr[i],badval[i]);CHECK(!cb_boot(cb_model_io(&m),7));CHECK(!m.release_writes&&!m.line_writes);}
 cb_model_init(&m,128);cb_model_set(&m,0xE000ED90u,16u<<8);CHECK(cb_boot(cb_model_io(&m),UINT32_MAX));
 CHECK(m.release_generation==UINT32_MAX);for(unsigned i=8;i<16;++i)CHECK(!m.attrs[i]);
 cb_model_init(&m,128);CHECK(!cb_boot(cb_model_io(&m),0));CHECK(!m.writes);
 CBIO missing=cb_model_io(&m);missing.isb=0;CHECK(!cb_boot(missing,7));CHECK(!m.writes);
 /* Actual imported peer state: health is neither reservation return nor persistence. */
 for(unsigned first=0;first<2;++first){
  FWPeer p;p.generation=7;p.health=0;p.failed=0;p.outstanding[0]=0;p.outstanding[1]=0;p.confirmed=0;p.reserved[0]=0;p.reserved[1]=0;
  CHECK(fw_peer_step(&p,first,6,FW_BEGIN)==H7_STALE);
  CHECK(fw_peer_step(&p,first,7,FW_BEGIN)==H7_OK);
  CHECK(fw_peer_step(&p,first,7,FW_HEALTHY)==H7_BUSY);
  CHECK(fw_peer_step(&p,1u-first,7,FW_HEALTHY)==H7_OK);
  CHECK(fw_peer_step(&p,first,7,FW_CONFIRM)==H7_BUSY);
  CHECK(fw_peer_step(&p,first,7,FW_END)==H7_OK);
  CHECK(fw_peer_step(&p,first,7,FW_HEALTHY)==H7_OK);
  CHECK(fw_peer_step(&p,first,7,FW_CONFIRM)==H7_OK);
  CHECK(fw_peer_step(&p,first,7,FW_BEGIN)==H7_BUSY);
  CHECK(fw_peer_step(&p,first,7,FW_FAIL_PEER)==H7_OK&&!p.confirmed);
 }
 return 0;
}
#ifndef CB_GUEST
int main(int argc,char**argv){if(argc==2){unsigned sets=(unsigned)strtoul(argv[1],0,10);if(sets!=32&&sets!=64&&sets!=128&&sets!=256&&sets!=512)return 2;cb_model_init(&m,sets);if(!cb_boot(cb_model_io(&m),7))return 3;for(unsigned s=0;s<sets;++s)for(unsigned w=0;w<4;++w)if(m.invalid[s][w])printf("%u\n",(w<<30)|(s<<5));return 0;}int r=cb_suite();if(r){fprintf(stderr,"CB_CHECK:%d\n",r);return 41;}puts("CB_PASS:geometry,release,shared-line,peer");return 0;}
#endif
