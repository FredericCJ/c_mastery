#include "scheduler.h"
#include "serial_depth.h"
void ss_note(const char *);void ss_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){ss_note("SS_ASSERT: " #x);return 41;}}while(0)
struct mc_model {uint32_t r[19];uintptr_t addr[2];unsigned calls,reads,writes,reset,stop;uint8_t wire[16];};
static uint32_t rd(void *p,unsigned reg){struct mc_model *m=p;m->calls++;if(reg==PS_DR){m->reads++;m->r[PS_SR]&=~PS_RXNE;}return m->r[reg];}
static void wr(void *p,unsigned reg,uintptr_t value){
 struct mc_model *m=p;m->calls++;
 if(reg==PS_DR){if(m->writes<16)m->wire[m->writes]=(uint8_t)value;m->writes++;m->r[PS_SR]=(m->r[PS_SR]&(PS_RXNE|PS_OVR))|PS_BSY;return;}
 if((reg==PS_TX_EN || reg==PS_RX_EN) && !value){m->stop++;return;}
 if(reg==PS_TX_ADDR || reg==PS_RX_ADDR){m->addr[reg==PS_RX_ADDR]=value;return;}
 m->r[reg]=(uint32_t)value;
 if(reg==PS_TX_START)m->r[PS_TX_EN]=1;
 if(reg==PS_RX_START)m->r[PS_RX_EN]=1;
 if(reg==PS_RESET){m->reset++;m->r[PS_SR]=PS_TXE;}
}
static struct ps_io io(struct mc_model *m){struct ps_io r={m,rd,wr};return r;}
static void receive(struct mc_model *m,uint8_t value){
 if(m->r[PS_SR]&PS_RXNE)m->r[PS_SR]|=PS_OVR;
 else m->r[PS_DR]=value;
 m->r[PS_SR]|=PS_TXE|PS_RXNE|PS_BSY;
}
int ss_edges(void){
 unsigned cases=0;uint8_t level[16],sample[16],mosi[16],rx;
 for(unsigned mode=0;mode<4;mode++)for(unsigned tx=0;tx<256;tx++)for(unsigned peer=0;peer<256;peer++){
  CHECK(sd_spi_edge(mode,(uint8_t)tx,(uint8_t)peer,level,sample,mosi,&rx)==SD_OK && rx==peer);
  for(unsigned edge=0;edge<16;edge++){
   CHECK(level[edge]==((mode>>1)^((edge+1)&1u)));
   CHECK(sample[edge]==((edge%2)==(mode%2)) && mosi[edge]==((tx>>(7-edge/2))&1u));
  }
  cases++;
 }
 uint8_t cmd[3],wire[3]={0xa6,0x34,0x12};uint16_t value=99;
 CHECK(ps_command(1,3,cmd)==PS_OK && cmd[0]==0xc3 && !cmd[1] && !cmd[2]);
 CHECK(ps_decode(1,wire,&value)==PS_OK && value==0x1234);
 wire[0]=0;CHECK(ps_decode(1,wire,&value)==PS_PROTOCOL && value==0x1234);
 ss_values("edges",cases,0x1234,0xc3,2);return 0;
}
int ss_queue(void){
 for(unsigned n=1;n<=16;n++){
  struct ss_scheduler q={0};struct mc_model m={0};struct pu_clock c={16000000,1,1,1,1,0};uint8_t tx[16],out[16];struct ss_result result={99,99,99};
  for(unsigned i=0;i<16;i++){tx[i]=(uint8_t)(0xa0+i);out[i]=0xcc;}
  CHECK(ss_open(&q,io(&m),&c,3,3)==PS_OK);
  CHECK(ss_enqueue(&q,1,tx,n,0,100,2,3)==PS_OK && ss_enqueue(&q,2,tx,1,0,100,0,0)==PS_OK);
  CHECK(ss_enqueue(&q,3,tx,1,0,100,0,0)==PS_BUSY && q.last_token==2);
  tx[0]=0;CHECK(ss_run(&q,io(&m),ss_portable_submit,UINT32_MAX-3u)==PS_BUSY);
  CHECK(ss_take(&q,1,&result,out)==PS_BUSY && result.status==99 && out[0]==0xcc);
  m.r[PS_SR]=PS_TXE;CHECK(ss_run(&q,io(&m),ss_portable_submit,UINT32_MAX-2u)==PS_BUSY && !m.writes);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,UINT32_MAX-1u)==PS_BUSY && m.writes==1 && m.wire[0]==0xa0);
  for(unsigned i=0;i<n;i++){receive(&m,(uint8_t)(0x30+i));CHECK(ss_run(&q,io(&m),ss_portable_submit,UINT32_MAX+i)==PS_BUSY);}
  CHECK(m.reads==n && m.writes==n && q.driver.cs_owned && !q.driver.rx_owned);
  m.r[PS_SR]=PS_TXE;uint32_t at=n;
  CHECK(ss_run(&q,io(&m),ss_portable_submit,at)==PS_BUSY);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,at+2)==PS_BUSY && q.driver.cs_owned);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,at+3)==PS_OK && !q.driver.cs_owned);
  CHECK(ss_take(&q,1,&result,out)==PS_OK && result.status==PS_OK && result.sent==n && result.received==n);
  for(unsigned i=0;i<16;i++)CHECK(out[i]==(i<n?0x30+i:0xcc));
  CHECK(q.pending.count==1 && ss_enqueue(&q,3,tx,1,0,100,0,0)==PS_OK);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,at+4)==PS_BUSY && q.driver.token==2);
  ss_values("queue",n,result.sent,result.received,q.driver.token);
  uint32_t tick=at+5;
  for(uint32_t token=2;token<=3;token++){
   if(token==3)CHECK(ss_run(&q,io(&m),ss_portable_submit,tick++)==PS_BUSY);
   m.r[PS_SR]=PS_TXE;CHECK(ss_event(&q,io(&m),token,tick++)==PS_BUSY);
   receive(&m,0x44);CHECK(ss_event(&q,io(&m),token,tick++)==PS_BUSY);
   m.r[PS_SR]=PS_TXE;CHECK(ss_event(&q,io(&m),token,tick++)==PS_OK);
   CHECK(ss_take(&q,token,&result,out)==PS_OK);
  }
  CHECK(!q.active && !q.pending.count);
 }
 /* Original flawed send-only path never drains RX and loses the later value. */
 struct mc_model flawed={0};
 /* Intentionally flawed original client: transmit on readiness, never read DR. */
 for(unsigned i=0;i<2;i++){flawed.r[PS_SR]|=PS_TXE;if(rd(&flawed,PS_SR)&PS_TXE)wr(&flawed,PS_DR,i);receive(&flawed,(uint8_t)(0x11*(i+1)));}
 CHECK((flawed.r[PS_SR]&PS_OVR) && flawed.r[PS_DR]==0x11 && flawed.reads==0 && flawed.writes==2);
 ss_values("send_only",flawed.r[PS_DR],flawed.r[PS_SR]&PS_OVR,flawed.reads,flawed.writes);return 0;
}
int ss_quiescence(void){
 for(unsigned order=0;order<2;order++){
  struct ss_scheduler q={0};struct mc_model m={0};struct pu_clock c={16000000,1,1,1,1,0};uint8_t tx[3]={0xc3,0,0},out[16]={0};struct ss_result r={99,99,99};
  CHECK(ss_open(&q,io(&m),&c,0,3)==PS_OK && ss_enqueue(&q,1,tx,3,1,8,1,3)==PS_OK);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,0)==PS_BUSY);
  m.r[PS_SR]=PS_TXE;CHECK(ss_run(&q,io(&m),ss_portable_submit,0)==PS_BUSY && !m.r[PS_TX_EN]);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,1)==PS_BUSY && m.r[PS_TX_EN] && m.r[PS_RX_EN]);
  CHECK(m.addr[0]==(uintptr_t)q.slots[0].tx_words && m.addr[1]==(uintptr_t)q.slots[0].rx_words);
  m.r[PS_SR]=PS_RXNE|PS_BSY;
  CHECK(ss_run(&q,io(&m),ss_portable_submit,8)==PS_BUSY && q.driver.result==PS_TIMEOUT && m.stop==2);
  CHECK(ss_take(&q,1,&r,out)==PS_BUSY && r.status==99 && q.slots[0].state==SS_RUNNING);
  CHECK(ss_reset(&q,io(&m),1,9)==PS_BUSY && !m.reset && q.driver.ready);
  for(unsigned i=0;i<3;i++)q.slots[0].rx_words[i]=(uint16_t)(0xa0+i);
  unsigned first=order?PS_RX_EN:PS_TX_EN,second=order?PS_TX_EN:PS_RX_EN;
  m.r[first]=0;CHECK(ss_event(&q,io(&m),1,10)==PS_BUSY && q.driver.cs_owned);
  CHECK(q.driver.rx_owned==(order?0:1) && q.driver.tx_owned==(order?1:0));
  CHECK(ss_reset(&q,io(&m),1,11)==PS_BUSY && !m.reset);
  m.r[second]=0;m.r[PS_TX_COUNT]=m.r[PS_RX_COUNT]=0;
  CHECK(ss_reset(&q,io(&m),1,12)==PS_BUSY && m.reset==1 && !q.driver.ready && q.driver.cs_owned);
  CHECK(ss_event(&q,io(&m),1,14)==PS_BUSY && ss_take(&q,1,&r,out)==PS_BUSY);
  CHECK(ss_event(&q,io(&m),1,15)==PS_TIMEOUT && ss_take(&q,1,&r,out)==PS_OK && r.status==PS_TIMEOUT && r.received==3 && out[2]==0xa2);
  CHECK(ss_open(&q,io(&m),&c,0,3)==PS_OK && ss_enqueue(&q,2,tx,3,0,100,0,0)==PS_OK);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,16)==PS_BUSY);unsigned calls=m.calls;
  CHECK(ss_event(&q,io(&m),1,17)==PS_STALE && m.calls==calls && q.driver.sent==0);
  ss_values("quiescence",order,r.status,r.received,m.reset);
  CHECK(ss_cancel(&q,io(&m),2)==PS_BUSY);m.r[PS_SR]=PS_TXE;
  CHECK(ss_event(&q,io(&m),2,18)==PS_CANCELLED && ss_take(&q,2,&r,out)==PS_OK);
 }
 return 0;
}
/* Public finite IRQ probe; target harness invokes its middle operation in IRQ0. */
static struct ss_scheduler irq_q;static struct mc_model irq_m;static unsigned irq_before;static int irq_status;
int ss_irq_prepare(void){
 struct ss_scheduler zero={0};struct mc_model model={0};irq_q=zero;irq_m=model;struct pu_clock c={16000000,1,1,1,1,0};uint8_t tx=0x55;
 uint8_t out[16]={0};struct ss_result result={99,99,99};
 CHECK(ss_open(&irq_q,io(&irq_m),&c,0,3)==PS_OK && ss_enqueue(&irq_q,1,&tx,1,1,100,0,2)==PS_OK);
 CHECK(ss_run(&irq_q,io(&irq_m),ss_portable_submit,0)==PS_BUSY);
 irq_m.r[PS_SR]=PS_TXE;CHECK(ss_event(&irq_q,io(&irq_m),1,1)==PS_BUSY);
 CHECK(ss_cancel(&irq_q,io(&irq_m),1)==PS_BUSY && irq_m.stop==2);
 CHECK(irq_q.driver.tx_owned && irq_q.driver.rx_owned && irq_q.driver.cs_owned);
 CHECK(ss_reset(&irq_q,io(&irq_m),1,2)==PS_BUSY && !irq_m.reset);
 irq_m.r[PS_TX_EN]=0;CHECK(ss_reset(&irq_q,io(&irq_m),1,3)==PS_BUSY && !irq_m.reset);
 irq_m.r[PS_RX_EN]=0;CHECK(ss_reset(&irq_q,io(&irq_m),1,4)==PS_BUSY && irq_m.reset==1);
 CHECK(ss_event(&irq_q,io(&irq_m),1,6)==PS_CANCELLED && ss_take(&irq_q,1,&result,out)==PS_OK && result.status==PS_CANCELLED);
 CHECK(ss_open(&irq_q,io(&irq_m),&c,0,3)==PS_OK && ss_enqueue(&irq_q,2,&tx,1,0,100,0,0)==PS_OK);
 CHECK(ss_run(&irq_q,io(&irq_m),ss_portable_submit,20)==PS_BUSY);irq_m.r[PS_SR]=PS_TXE;irq_before=irq_m.calls;irq_status=-1;return 0;
}
void ss_irq_old_event(void){irq_status=ss_event(&irq_q,io(&irq_m),1,21);}
int ss_irq_verify(void){
 CHECK(irq_status==PS_STALE && irq_m.calls==irq_before && irq_q.driver.sent==0 && irq_q.driver.tx_owned);
 CHECK(ss_event(&irq_q,io(&irq_m),2,21)==PS_BUSY && irq_q.driver.sent==1);
 ss_values("irq_stale",(unsigned)irq_status,irq_q.driver.sent,irq_q.driver.tx_owned,irq_q.driver.cs_owned);
 uint8_t out[16]={0};struct ss_result result={99,99,99};receive(&irq_m,0x22);
 CHECK(ss_event(&irq_q,io(&irq_m),2,22)==PS_BUSY);irq_m.r[PS_SR]=PS_TXE;
 CHECK(ss_event(&irq_q,io(&irq_m),2,23)==PS_OK && ss_take(&irq_q,2,&result,out)==PS_OK);
 return 0;
}
int ss_irq_native(void){int r=ss_irq_prepare();if(r)return r;ss_irq_old_event();return ss_irq_verify();}
int ss_device(void){
 for(unsigned n=1;n<=3;n++){
  struct ss_scheduler q={0};struct mc_model m={0};struct pu_clock c={16000000,1,1,1,1,0};uint8_t tx[3],reply[3]={0xa6,0x34,0x12},out[16]={0};struct ss_result r={99,99,99};uint16_t value=0x7777;
  CHECK(ps_command(1,3,tx)==PS_OK && ss_open(&q,io(&m),&c,1,3)==PS_OK && ss_enqueue(&q,1,tx,n,0,100,0,0)==PS_OK);
  CHECK(ss_run(&q,io(&m),ss_portable_submit,0)==PS_BUSY);m.r[PS_SR]=PS_TXE;
  CHECK(ss_event(&q,io(&m),1,1)==PS_BUSY);
  for(unsigned i=0;i<n;i++){receive(&m,reply[i]);CHECK(ss_event(&q,io(&m),1,2+i)==PS_BUSY);}
  m.r[PS_SR]=PS_TXE;CHECK(ss_event(&q,io(&m),1,6)==PS_OK && ss_take(&q,1,&r,out)==PS_OK);
  /* A short transport is incomplete device framing; do not decode zero padding. */
  unsigned complete=(r.received==3);
  if(complete)CHECK(ps_decode(1,out,&value)==PS_OK && value==0x1234);
  else CHECK(value==0x7777);
  CHECK(m.writes==n && m.reads==n && m.wire[0]==0xc3);
  for(unsigned i=1;i<n;i++)CHECK(m.wire[i]==0);
  ss_values("device",n,r.received,complete,value);
 }
 return 0;
}
