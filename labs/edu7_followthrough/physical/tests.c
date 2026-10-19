#include "physical.h"
#include <limits.h>
int puts(const char *);
#define CHECK(x) do{if(!(x)){puts("EP_ASSERT:" #x);return 41;}}while(0)
typedef struct {uint32_t r[16];unsigned de,edges,writes;uint8_t bytes[128];} Model;
static uint32_t read_io(void *p,unsigned k){Model *m=p;uint32_t r=k==PU_SOURCE?1:m->r[k];if(k==PU_DR)m->r[PU_SR]&=~47u;return r;}
static void write_io(void *p,unsigned k,uintptr_t v){Model *m=p;if(k==PU_SR)m->r[k]&=(uint32_t)v;else m->r[k]=(uint32_t)v;
 if(k==PU_DR){if(m->writes<128)m->bytes[m->writes]=(uint8_t)v;m->writes++;m->r[PU_SR]&=~(PU_TXE|PU_TC);}}
static void drive(void *p,unsigned on){Model *m=p;m->de=on;m->edges++;}
static int open_tx(EpTx *s,Model *m,unsigned baud,unsigned device){PuClock c={42000000,1,1,1,1,0};R5Timing t={device,0,1000,200000};R5Io io={{m,read_io,write_io},m,drive};return ep_open(s,io,&c,baud,&t,0);}
static int physical(void){
 EpTx a={0},b={0};Model ma={0},mb={0};unsigned fits=99;
 CHECK(open_tx(&a,&ma,115200,R5_THVD1400)==EP_OK);
 CHECK(a.divisor==365 && a.delays.enable==1 && a.delays.disable==1 && a.delays.guard==200);
 CHECK(ep_fits(&a,96,10000,&fits)==EP_OK && fits==1);
 CHECK(open_tx(&b,&mb,57600,R5_THVD1410)==EP_OK);
 CHECK(b.divisor==729 && ep_fits(&b,96,10000,&fits)==EP_OK && fits==0);
 uint8_t data[96]={0};CHECK(ep_send(&b,1,data,96,0,10000)==EP_TIMEOUT && mb.de==0 && b.token==0);
 CHECK(ep_fits(&a,97,10000,&fits)==EP_INVALID && fits==0);
 R5Timing t={R5_THVD1400,0,1,200000};R5Delays d;
 CHECK(r5_delays(&t,&d)==R5_OK && d.enable==650 && d.disable==200);
 t.device=R5_THVD1410;CHECK(r5_delays(&t,&d)==R5_OK && d.enable==600);
 t.receiver_disabled=1;CHECK(r5_delays(&t,&d)==R5_OK && d.enable==11000);
 puts("EP_PHYSICAL_VALUES 42000000 365 729 96 10000 650 600 11000");
 puts("EP_PHYSICAL_PASS model_does_not_measure_two_terminated_segments");return 0;
}
static int tx(void){
 EpTx s={0};Model m={0};uint8_t data[96];for(unsigned i=0;i<96;i++)data[i]=(uint8_t)i;
 CHECK(open_tx(&s,&m,115200,R5_THVD1400)==EP_OK);
 CHECK(ep_send(&s,1,data,96,0,10000)==EP_OK&&m.de);
 for(unsigned i=0;i<96;i++)data[i]=255;
 m.r[PU_SR]=PU_TXE|PU_TC;
 CHECK(ep_step(&s,1,199)==EP_BUSY && m.writes==0);
 CHECK(ep_send(&s,2,data,1,199,10000)==EP_BUSY && s.token==1);
 for(unsigned i=0;i<96;i++){
  uint32_t now=200+i*90;m.r[PU_SR]=PU_TXE|PU_TC;
  CHECK(ep_step(&s,1,now)==EP_BUSY && m.de && m.writes==i+1 && m.bytes[i]==i);
  if(i%8==7){m.r[PU_SR]=PU_TXE;CHECK(ep_step(&s,1,now+1)==EP_BUSY && m.de);
   m.r[PU_SR]=PU_TC;CHECK(ep_step(&s,1,now+86)==EP_BUSY);
   CHECK(m.de==(i!=95));}
 }
 CHECK(m.edges==3 && s.byte.token==12 && s.phase==EP_GUARD);
 CHECK(ep_step(&s,1,9035)==EP_BUSY && ep_step(&s,1,9036)==EP_OK && s.phase==EP_IDLE);
 CHECK(ep_step(&s,1,9037)==EP_STALE && ep_send(&s,1,data,1,9038,10000)==EP_STALE);
 EpTx c={0};Model mc={0};CHECK(open_tx(&c,&mc,115200,R5_THVD1400)==EP_OK);
 CHECK(ep_send(&c,1,data,2,0,10000)==EP_OK);mc.r[PU_SR]=PU_TXE;
 CHECK(ep_step(&c,1,200)==EP_BUSY && mc.writes==1);
 CHECK(ep_cancel(&c,1,201)==EP_BUSY && mc.de);
 CHECK(ep_step(&c,1,20000)==EP_BUSY && mc.de && c.byte.line_owned);
 CHECK(ep_send(&c,2,data,1,20001,10000)==EP_BUSY);
 mc.r[PU_SR]=PU_TC;CHECK(ep_step(&c,1,20002)==EP_BUSY && !mc.de);
 CHECK(ep_step(&c,1,20203)==EP_DROP && mc.writes==1);
 EpTx early={0};Model me={0};CHECK(open_tx(&early,&me,115200,R5_THVD1400)==EP_OK);
 CHECK(ep_send(&early,1,data,2,0,10000)==EP_OK && ep_cancel(&early,1,0)==EP_BUSY);
 CHECK(ep_step(&early,1,201)==EP_DROP && me.writes==0 && !me.de);
 EpTx late={0};Model ml={0};CHECK(open_tx(&late,&ml,115200,R5_THVD1400)==EP_OK);
 CHECK(ep_send(&late,1,data,1,0,10000)==EP_OK);ml.r[PU_SR]=PU_TXE;
 CHECK(ep_step(&late,1,200)==EP_BUSY);
 CHECK(ep_step(&late,1,10200)==EP_BUSY && ml.de);
 ml.r[PU_SR]=PU_TC;CHECK(ep_step(&late,1,10201)==EP_BUSY && !ml.de);
 CHECK(ep_step(&late,1,10401)==EP_TIMEOUT);
 puts("EP_TX_VALUES 96 12 3 9036");puts("EP_TX_PASS copied_frame_retained_until_final_TC");return 0;
}
static int begin_poll(EpMaster *s,Model *m,uint32_t *origin){
 CHECK(open_tx(&s->tx,m,115200,R5_THVD1400)==EP_OK);
 CHECK(ep_poll(s,1,0)==EP_OK && s->grant.phase==0);
 for(uint32_t now=1;now<4000;now+=100){m->r[PU_SR]=PU_TXE|PU_TC;int r=ep_tick(s,now);
  if(!m->de&&s->tx.phase==EP_GUARD){
   CHECK(r==EP_BUSY && s->grant.phase==1 && s->grant.start_earliest==now+200);
   EduFrame untouched={.length=39};
   CHECK(ep_receive(s,PU_RXNE,0x7e,now+199,&untouched)==EP_DROP && untouched.length==39 && s->grant.phase==1);
   CHECK(ep_tick(s,now+200)==EP_OK && s->tx.phase==EP_IDLE);
   *origin=now;return 0;
  }
  CHECK(r==EP_BUSY);}
 CHECK(0);return 41;
}
static int grant(void){
 EpSlave slave={0};Model sm={0};CHECK(open_tx(&slave.tx,&sm,115200,R5_THVD1400)==EP_OK);
 EduFrame reply={.source=2,.destination=1,.kind=2,.length=0};
 CHECK(ep_reply(&slave,1,&reply,300)==EP_STALE && !sm.de);
 EduFrame poll={.source=1,.destination=2,.kind=1,.length=0};uint8_t pollbytes[96];size_t pn;
 CHECK(edu_frame_encode(&poll,pollbytes,sizeof pollbytes,&pn)==0);
 for(size_t i=0;i<pn;i++){sm.r[PU_SR]=PU_RXNE;sm.r[PU_DR]=pollbytes[i];int r=ep_slave_poll_rx(&slave,500+(uint32_t)i);CHECK(r==(i+1==pn?EP_OK:EP_BUSY) && !(sm.r[PU_SR]&PU_RXNE));}
 uint32_t origin_slave=slave.origin;
 CHECK(ep_reply(&slave,1,&reply,origin_slave+199)==EP_BUSY && !sm.de);
 CHECK(ep_reply(&slave,1,&reply,origin_slave+200)==EP_OK && sm.de);
 /* Scheduler stall past first-byte admission cannot emit a late first byte. */
 sm.r[PU_SR]=PU_TXE|PU_TC;CHECK(ep_step(&slave.tx,1,origin_slave+2201)==EP_BUSY && sm.writes==0);
 CHECK(ep_step(&slave.tx,1,origin_slave+2402)==EP_TIMEOUT && !sm.de);
 EpMaster s={0};Model m={0};uint32_t origin=0;CHECK(begin_poll(&s,&m,&origin)==0);
 CHECK(s.grant.start_earliest==origin+200 && s.grant.start_deadline==origin+2200);
 EduFrame f={.source=2,.destination=1,.kind=0,.length=1,.payload={42}},out={.length=39};uint8_t bytes[96];size_t n;
 CHECK(edu_frame_encode(&f,bytes,sizeof bytes,&n)==0);
 CHECK(ep_receive(&s,PU_RXNE,bytes[0],origin+1500,&out)==EP_BUSY);
 CHECK(ep_tick(&s,origin+2300)==EP_OK && s.grant.phase==2);
 for(size_t i=1;i<n;i++){uint32_t now=origin+2300+(uint32_t)i*7200u/(uint32_t)(n-1);int r=ep_receive(&s,PU_RXNE,bytes[i],now,&out);CHECK(r==(i+1==n?EP_FRAME:EP_BUSY));}
 CHECK(out.payload[0]==42 && s.delivered==1 && !s.grant.phase);
 CHECK(ep_poll(&s,2,origin+24999)==EP_BUSY);
 EpMaster missing={0};Model mm={0};uint32_t second=0;CHECK(begin_poll(&missing,&mm,&second)==0);
 CHECK(ep_tick(&missing,second+2200)==EP_OK && ep_tick(&missing,second+2201)==EP_TIMEOUT);
 CHECK(ep_receive(&missing,PU_RXNE,0x7e,second+2202,&out)==EP_DROP);
 EpMaster echo={0};Model em={0};CHECK(open_tx(&echo.tx,&em,115200,R5_THVD1400)==EP_OK);
 CHECK(ep_poll(&echo,1,0)==EP_OK);
 em.r[PU_SR]=PU_RXNE|PU_FE|PU_ORE;em.r[PU_DR]=0x7e;
 CHECK(ep_master_poll_rx(&echo,0,&out)==EP_DROP && !(em.r[PU_SR]&PU_RXNE));
 CHECK(echo.echoes==1 && echo.faults==1 && echo.delivered==0 && echo.parser.legacy.active==0);
 CHECK(ep_disconnect(&echo,0)==EP_BUSY && !em.de && echo.faults==2);
 CHECK(ep_tick(&echo,201)==EP_DROP && !echo.grant.phase && em.writes==0);
 EpMaster fault={0};Model fm={0};uint32_t fo=0;CHECK(begin_poll(&fault,&fm,&fo)==0);
 CHECK(ep_receive(&fault,PU_RXNE,bytes[0],fo+300,&out)==EP_BUSY);
 CHECK(ep_receive(&fault,PU_RXNE|PU_FE,bytes[1],fo+301,&out)==EP_DROP && fault.faults==1);
 for(size_t i=2;i<n;i++)CHECK(ep_receive(&fault,PU_RXNE,bytes[i],fo+300+(uint32_t)i,&out)!=EP_FRAME);
 CHECK(fault.delivered==0);
 for(size_t i=0;i<n;i++){int r=ep_receive(&fault,PU_RXNE,bytes[i],fo+1000+(uint32_t)i,&out);CHECK(r==(i+1==n?EP_FRAME:EP_BUSY));}
 CHECK(fault.delivered==1 && fault.faults==1);
 puts("EP_GRANT_VALUES 1500 9500 2200 10000");puts("EP_GRANT_PASS actual_poll_TC_guard_fault_and_echo_boundaries");return 0;
}
int ep_tests(unsigned n){return n==0?physical():n==1?tx():n==2?grant():n==3?ep_mapped():41;}
