#include "channel.h"
#include <limits.h>
void r5_note(const char *);
void r5_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){r5_note("R5_ASSERT: " #x);return 41;}}while(0)
struct mc_model {uint32_t r[16];uintptr_t address;unsigned de,edges,writes,reads;};
static uint32_t read_io(void *p,unsigned k){struct mc_model *m=p;if(k==PU_SOURCE)return 1;if(k==PU_DR){++m->reads;m->r[PU_SR]&=~47u;}return m->r[k];}
static void write_io(void *p,unsigned k,uintptr_t v){struct mc_model *m=p;
 if(k==PU_DMA_ADDRESS)m->address=v;
 else if(k==PU_SR)m->r[k]&=(uint32_t)v;
 else if(k==PU_DMA_CLEAR)m->r[PU_DMA_FLAGS]=0;
 else m->r[k]=(uint32_t)v;
 if(k==PU_DR){++m->writes;m->r[PU_SR]&=~(PU_TXE|PU_TC);}
 if(k==PU_DMA_CONTROL&&v)m->r[PU_DMA_BUSY]=1;
}
static void drive(void *p,unsigned enabled){struct mc_model *m=p;m->de=enabled;++m->edges;}
static int open_channel(struct r5_channel *s,struct mc_model *m,unsigned rx,unsigned events,uint32_t now){
 struct pu_clock c={42000000,1,1,1,1,0};struct r5_timing timing={R5_THVD1410,0,1,2000};
 struct r5_io io={{m,read_io,write_io},m,drive};return r5_open(s,io,&c,100000,&timing,rx,events,now);
}
static int config(void){
 struct r5_timing t={R5_THVD1410,0,1,2000};struct r5_delays d={99,99,99};unsigned load=99;
 CHECK(r5_load(16,16,&load)==R5_OK&&load==32);
 CHECK(r5_load(256,1,&load)==R5_INVALID&&load==32);
 CHECK(r5_delays(&t,&d)==R5_OK&&d.enable==600&&d.disable==200&&d.guard==2000);
 t.receiver_disabled=1;CHECK(r5_delays(&t,&d)==R5_OK&&d.enable==11000);
 t.device=R5_THVD1400;CHECK(r5_delays(&t,&d)==R5_OK&&d.enable==10000);
 t.receiver_disabled=0;CHECK(r5_delays(&t,&d)==R5_OK&&d.enable==650);
 t.tick_ns=1000;CHECK(r5_delays(&t,&d)==R5_OK&&d.enable==1&&d.disable==1&&d.guard==2);
 t.tick_ns=0;d.enable=99;CHECK(r5_delays(&t,&d)==R5_INVALID&&d.enable==99);
 t.tick_ns=1;t.guard_ns=0x7fffffffu;CHECK(r5_delays(&t,&d)==R5_INVALID);
 r5_values("config",32,11000,650,4);return 0;
}
static int one_turn(uint32_t base){
 struct r5_channel s={0};struct mc_model m={0};uint8_t bytes[]={17,34};size_t count=99;struct r5_event e={0};
 CHECK(open_channel(&s,&m,16,32,base)==R5_OK);
 CHECK(r5_send(&s,1,bytes,2,PU_POLL,base,500000,&count)==R5_OK&&count==2&&m.de);
 bytes[0]=99;bytes[1]=99;
 CHECK(r5_step(&s,1,base+599)==R5_BUSY&&m.writes==0);
 CHECK(r5_step(&s,1,base+600)==R5_BUSY);
 m.r[PU_SR]=PU_TXE|PU_TC;CHECK(r5_step(&s,1,base+600)==R5_BUSY&&m.r[PU_DR]==17&&m.de);
 m.r[PU_SR]=PU_TXE;CHECK(r5_step(&s,1,base+100600)==R5_BUSY&&m.r[PU_DR]==34&&m.de);
 CHECK(r5_step(&s,1,base+100601)==R5_BUSY&&m.de==1);
 m.r[PU_SR]=PU_TC;CHECK(r5_step(&s,1,base+200600)==R5_BUSY&&!m.de);
 CHECK(r5_step(&s,1,base+202799)==R5_BUSY);
 CHECK(r5_step(&s,1,base+202800)==R5_OK&&s.phase==R5_IDLE);
 CHECK(r5_take(&s,&e,&count)==R5_OK&&count==1&&e.kind==R5_TX_DONE&&e.token==1);
 CHECK(r5_take(&s,&e,&count)==R5_BUSY&&count==0);
 CHECK(r5_step(&s,1,base+202801)==R5_STALE);
 CHECK(r5_send(&s,1,bytes,2,PU_POLL,base+202801,500000,&count)==R5_STALE);
 CHECK(r5_send(&s,2,bytes,2,PU_POLL,base+202801,0x80000000u,&count)==R5_INVALID);
 return 0;
}
static int timing(void){CHECK(one_turn(0)==0);CHECK(one_turn(UINT32_MAX-300u)==0);r5_values("timing",200600,202800,2,0x80000000u);return 0;}
static int events(void){
 struct r5_channel s={0};struct mc_model m={0};size_t consumed=99;struct r5_event e={0};
 CHECK(open_channel(&s,&m,2,8,0)==R5_OK);
 CHECK(r5_receive(&s,PU_RXNE,17,&consumed)==R5_OK&&consumed==1);
 CHECK(r5_receive(&s,PU_RXNE,34,&consumed)==R5_OK&&consumed==1);
 consumed=99;CHECK(r5_receive(&s,PU_RXNE,51,&consumed)==R5_FULL&&consumed==99&&s.rx_count==2);
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_BYTE&&e.value==17);
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_BYTE&&e.value==34);
 CHECK(r5_receive(&s,PU_RXNE|PU_FE|PU_ORE,33,&consumed)==R5_OK);
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_BYTE&&e.value==33&&e.error==(PU_FE|PU_ORE));
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_FRAMING);
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_OVERRUN);
 CHECK(r5_take(&s,&e,&consumed)==R5_BUSY&&consumed==0);
 m.r[PU_SR]=PU_RXNE|PU_FE;m.r[PU_DR]=0x55;
 CHECK(r5_receive_poll(&s)==R5_OK&&m.reads==1);
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_BYTE&&e.value==0x55&&e.error==PU_FE);
 CHECK(r5_take(&s,&e,&consumed)==R5_OK&&e.kind==R5_FRAMING);
 for(unsigned i=0;i<8;++i)CHECK(r5_loss(&s)==R5_OK);
 m.r[PU_SR]=PU_RXNE;m.r[PU_DR]=0x66;
 CHECK(r5_receive_poll(&s)==R5_FULL&&m.reads==1&&(m.r[PU_SR]&PU_RXNE));
 r5_values("events",2,3,m.reads,s.queued);return 0;
}
static int cancellation(void){
 struct r5_channel s={0};struct mc_model m={0};uint8_t bytes[16];for(unsigned i=0;i<16;++i)bytes[i]=(uint8_t)i;
 size_t count=0;struct r5_event e={0};CHECK(open_channel(&s,&m,16,8,0)==R5_OK);
 CHECK(r5_send(&s,1,bytes,16,PU_DMA,0,500000,&count)==R5_OK&&count==16);
 CHECK(r5_step(&s,1,600)==R5_BUSY&&s.tx.byte.memory_owned&&m.r[PU_DMA_BUSY]);
 CHECK(m.address==(uintptr_t)s.tx_bytes&&m.r[PU_DMA_COUNT]==8);
 CHECK(r5_cancel(&s,1,601)==R5_OK&&s.tx.byte.memory_owned&&m.de);
 CHECK(r5_step(&s,1,602)==R5_UNAVAILABLE&&s.tx.byte.memory_owned&&m.de);
 CHECK(r5_take(&s,&e,&count)==R5_OK&&e.kind==R5_DEVICE_UNAVAILABLE&&e.token==1);
 CHECK(r5_step(&s,1,603)==R5_UNAVAILABLE);
 CHECK(r5_take(&s,&e,&count)==R5_BUSY&&count==0);
 m.r[PU_SR]=PU_TC;CHECK(r5_step(&s,1,604)==R5_UNAVAILABLE&&m.de);
 m.r[PU_DMA_BUSY]=0;
 CHECK(r5_step(&s,1,605)==R5_BUSY&&!m.de&&!s.tx.byte.memory_owned&&!s.tx.byte.line_owned);
 CHECK(r5_step(&s,1,2805)==R5_OK);
 CHECK(r5_take(&s,&e,&count)==R5_OK&&e.kind==R5_ABORTED);
 CHECK(r5_send(&s,2,bytes,16,PU_DMA,2806,500000,&count)==R5_OK);
 CHECK(r5_step(&s,1,3406)==R5_STALE&&s.phase==R5_ENABLING);
 CHECK(r5_step(&s,2,3406)==R5_BUSY);
 m.r[PU_DMA_BUSY]=0;m.r[PU_DMA_FLAGS]=1;m.r[PU_DMA_COUNT]=0;m.r[PU_SR]=PU_TC;
 CHECK(r5_step(&s,2,4000)==R5_BUSY&&s.offset==8&&m.address==(uintptr_t)(s.tx_bytes+8));
 m.r[PU_DMA_BUSY]=0;m.r[PU_DMA_FLAGS]=1;m.r[PU_DMA_COUNT]=0;m.r[PU_SR]=PU_TC;
 CHECK(r5_step(&s,2,5000)==R5_BUSY&&s.offset==16&&!m.de);
 CHECK(r5_step(&s,2,7200)==R5_OK);
 CHECK(r5_take(&s,&e,&count)==R5_OK&&e.kind==R5_TX_DONE&&e.token==2);
 CHECK(r5_send(&s,3,bytes,1,PU_POLL,7201,1000,&count)==R5_OK);
 CHECK(r5_step(&s,3,7801)==R5_BUSY);
 m.r[PU_SR]=PU_TXE;CHECK(r5_step(&s,3,7801)==R5_BUSY);
 CHECK(r5_step(&s,3,8201)==R5_UNAVAILABLE&&m.de&&s.tx.byte.line_owned);
 CHECK(r5_take(&s,&e,&count)==R5_OK&&e.kind==R5_DEVICE_UNAVAILABLE);
 m.r[PU_SR]=PU_TC;CHECK(r5_step(&s,3,8202)==R5_BUSY&&!m.de);
 CHECK(r5_step(&s,3,10402)==R5_OK);
 CHECK(r5_take(&s,&e,&count)==R5_OK&&e.kind==R5_ABORTED&&e.token==3);
 /* A full unreserved queue cannot steal either eventual control event. */
 struct r5_channel q={0};struct mc_model qm={0};CHECK(open_channel(&q,&qm,1,3,0)==R5_OK);
 CHECK(r5_loss(&q)==R5_OK);
 CHECK(r5_send(&q,1,bytes,1,PU_DMA,0,1000,&count)==R5_OK&&q.reserved==2);
 CHECK(r5_loss(&q)==R5_FULL&&q.queued==1&&q.reserved==2);
 CHECK(r5_step(&q,1,600)==R5_BUSY);
 CHECK(r5_cancel(&q,1,601)==R5_OK);
 CHECK(r5_step(&q,1,602)==R5_UNAVAILABLE&&q.queued==2&&q.reserved==1&&qm.de);
 CHECK(r5_step(&q,1,0x8000025au)==R5_INVALID&&q.last==602);
 qm.r[PU_DMA_BUSY]=0;qm.r[PU_SR]=PU_TC;
 CHECK(r5_step(&q,1,603)==R5_BUSY&&!qm.de);
 CHECK(r5_step(&q,1,2803)==R5_OK&&q.queued==3&&!q.reserved);
 CHECK(r5_take(&q,&e,&count)==R5_OK&&e.kind==R5_LOSS);
 CHECK(r5_take(&q,&e,&count)==R5_OK&&e.kind==R5_DEVICE_UNAVAILABLE);
 CHECK(r5_take(&q,&e,&count)==R5_OK&&e.kind==R5_ABORTED);
 CHECK(r5_send(&q,2,bytes,1,PU_POLL,2804,1000,&count)==R5_OK);
 CHECK(r5_cancel(&q,1,2805)==R5_STALE&&q.phase==R5_ENABLING&&qm.de);
 CHECK(r5_cancel(&q,2,2805)==R5_OK&&!qm.de&&qm.writes==0);
 CHECK(r5_step(&q,2,5005)==R5_OK);
 r5_values("cancellation",1,16,2,7200);return 0;
}
static int link_case(void){
 struct edu_frame source={0,1,2,4,{1,0x7e,0x7d,2}},out={0};uint8_t wire[96];size_t n=0;
 CHECK(edu_frame_encode(&source,wire,sizeof wire,&n)==0&&n<=96);
 unsigned runs=0;
 for(size_t split=0;split<=n;++split){
  struct r5_link link={0};link.local=2;link.grant=1;
  struct r5_channel channel={0};struct mc_model model={0};size_t consumed=99;
  CHECK(open_channel(&channel,&model,16,32,0)==R5_OK);
  /* A channel transport lost an event, then a complete fresh frame arrives. */
  struct r5_event e={R5_BYTE,0,0,0};
  for(size_t i=0;i<n;++i){
   if(i==split){CHECK(r5_loss(&channel)==R5_OK);CHECK(r5_take(&channel,&e,&consumed)==R5_OK&&e.kind==R5_LOSS);CHECK(r5_link(&link,&e,&out)==R5_OK);}
   CHECK(r5_receive(&channel,PU_RXNE,wire[i],&consumed)==R5_OK&&consumed==1);
   CHECK(r5_take(&channel,&e,&consumed)==R5_OK&&consumed==1&&e.kind==R5_BYTE);
   int r=r5_link(&link,&e,&out);CHECK(r==R5_OK||r==R5_BUSY||r==R5_FRAME);
  }
  /* Loss at boundary0 precedes opening delimiter and need not lose the frame.
     A loss notice after the last byte cannot undo prior valid publication. */
  if(split==n){CHECK(r5_loss(&channel)==R5_OK);CHECK(r5_take(&channel,&e,&consumed)==R5_OK&&e.kind==R5_LOSS);CHECK(r5_link(&link,&e,&out)==R5_OK);}
  unsigned before=link.delivered;
  CHECK(before==(split==0||split==n?1u:0u));
  for(size_t i=0;i<n;++i){CHECK(r5_receive(&channel,PU_RXNE,wire[i],&consumed)==R5_OK);CHECK(r5_take(&channel,&e,&consumed)==R5_OK);int r=r5_link(&link,&e,&out);CHECK(r==R5_OK||r==R5_BUSY||r==R5_FRAME);}
  CHECK(link.delivered==before+1&&out.length==4&&out.payload[1]==0x7e&&out.payload[2]==0x7d&&link.faults==1);
  ++runs;
 }
 struct r5_link foreign={0};foreign.local=1;foreign.grant=1;struct r5_event e={R5_BYTE,0,0,0};
 out.length=99;for(size_t i=0;i<n;++i){e.value=wire[i];(void)r5_link(&foreign,&e,&out);}
 CHECK(!foreign.delivered&&foreign.rejected==1&&out.length==99);
 foreign=(struct r5_link){0};foreign.local=2;out.length=99;
 for(size_t i=0;i<n;++i){e.value=wire[i];(void)r5_link(&foreign,&e,&out);}
 CHECK(!foreign.delivered&&foreign.rejected==1&&out.length==99);
 r5_values("link",(unsigned)n,runs,1,0);return 0;
}
int r5_test(unsigned c){switch(c){case 0:return config();case 1:return timing();case 2:return events();case 3:return cancellation();case 4:return link_case();case 5:return r5_mapped();default:return 42;}}
