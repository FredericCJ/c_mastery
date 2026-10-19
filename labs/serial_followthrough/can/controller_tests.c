#include "can.h"
#include <limits.h>
void cn_note(const char *);
void cn_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){cn_note("CN_ASSERT: " #x);return 41;}}while(0)
/* Executable register-effects fixture. It is an original teaching model, not
   an emulator claim about bxCAN bit-level transmission or electrical timing. */
typedef struct {uint32_t words[256];CnRx fifo[3];unsigned count,writes,releases,bad,block_init,block_sync,abort_requests,addresses;} Model;
static Model mapped;
static uint32_t model_read(void *p,unsigned reg){
 Model *m=p;if((reg&3u)||reg>=sizeof m->words){++m->bad;return 0;}
 if(reg==CN_RF0R)return (m->words[reg/4]&24u)|m->count;
 if(reg>=CN_RI0R&&reg<=CN_RDH0R){
  if(!m->count){++m->bad;return 0;}CnRx *r=&m->fifo[0];
  if(reg==CN_RI0R)return (r->frame.extended?(r->frame.identifier<<3)|4u:r->frame.identifier<<21)|(r->frame.remote<<1);
  if(reg==CN_RDT0R)return (r->timestamp<<16)|(r->filter<<8)|r->frame.length;
  unsigned first=reg==CN_RDL0R?0:4;uint32_t v=0;for(unsigned i=0;i<4;++i)v|=(uint32_t)r->frame.data[first+i]<<(8*i);return v;
 }
 return m->words[reg/4];
}
static void model_write(void *p,unsigned reg,uint32_t value){
 Model *m=p;++m->writes;if((reg&3u)||reg>=sizeof m->words){++m->bad;return;}
 if(reg==CN_TSR){
  if(value&~129u)++m->bad;
  if(value&1u)m->words[reg/4]&=~15u;
  if(value&128u)++m->abort_requests;
  return;
 }
 if(reg==CN_RF0R){
  if(value&~56u)++m->bad;
  if(value&24u)m->words[reg/4]&=~(value&24u);
  if(value&32u){if(!m->count){++m->bad;return;}for(unsigned i=1;i<m->count;++i)m->fifo[i-1]=m->fifo[i];--m->count;++m->releases;}
  return;
 }
 if(reg>=CN_TI0R&&reg<=CN_TDH0R&&!(m->words[CN_TSR/4]&(1u<<26))){++m->bad;return;}
 if(reg==CN_BTR&&!(m->words[CN_MSR/4]&1u))++m->bad;
 if(reg>=CN_FM1R&&reg<=CN_FA1R&&!(m->words[CN_FMR/4]&1u))++m->bad;
 m->words[reg/4]=value;
 if(reg==CN_MCR){if(value&1u){if(!m->block_init)m->words[CN_MSR/4]=1;}else if(!m->block_sync)m->words[CN_MSR/4]=0;}
 if(reg==CN_TI0R&&value&1u)m->words[CN_TSR/4]&=~((1u<<26)|15u);
}
static void model_init(Model *m){*m=(Model){0};m->words[CN_TSR/4]=0x1c000000;m->words[CN_FMR/4]=0x2a1c0e01;m->words[CN_MSR/4]=2;}
static int start(CnController *c,Model *m,unsigned nart,unsigned locked){
 SDTiming t;CnIO io={m,model_read,model_write};if(sd_can_timing(42000000,6,11,2,1,&t)!=SD_OK)return 1;
 return cn_begin(c,io,&t,nart,locked,3)!=CN_MORE||cn_control(c)!=CN_MORE||cn_control(c)!=CN_OK;
}
static int inject(Model *m,unsigned id,unsigned value){
 uint32_t encoded=id<<21;
 if(!(m->words[CN_FA1R/4]&1u)||((encoded^m->words[CN_F0R1/4])&m->words[CN_F0R2/4]))return 0;
 CnRx r={0};r.frame.identifier=id;r.frame.length=1;r.frame.data[0]=(uint8_t)value;r.timestamp=0x1234;
 if(m->count==3){m->words[CN_RF0R/4]|=16;if(!(m->words[CN_MCR/4]&8u))m->fifo[2]=r;}
 else{m->fifo[m->count++]=r;if(m->count==3)m->words[CN_RF0R/4]|=8;}
 return 1;
}
uint32_t cn_mapped_read(uintptr_t address){
 if(address<UINT32_C(0x40006400)||address>=UINT32_C(0x40006800)){++mapped.bad;return 0;}
 ++mapped.addresses;return model_read(&mapped,(unsigned)(address-UINT32_C(0x40006400)));
}
void cn_mapped_write(uintptr_t address,uint32_t value){
 if(address<UINT32_C(0x40006400)||address>=UINT32_C(0x40006800)){++mapped.bad;return;}
 ++mapped.addresses;model_write(&mapped,(unsigned)(address-UINT32_C(0x40006400)),value);
}
static int fifo(void){
 unsigned values[2][3]={{10,20,40},{10,20,30}},releases=0;
 for(unsigned locked=0;locked<2;++locked){
  Model m;model_init(&m);CnController c={0};CHECK(!start(&c,&m,0,locked));
  CHECK(m.words[CN_BTR/4]==0x1a0005&&m.words[CN_F0R2/4]==0xfe000006);
  CHECK(inject(&m,0x120,10)&&inject(&m,0x123,20)&&inject(&m,0x12f,30));
  CHECK(!inject(&m,0x130,99));CHECK(inject(&m,0x123,40));
  CnRx out={0};for(unsigned i=0;i<3;++i){CHECK(cn_receive(&c,&out)==CN_FRAME&&out.frame.data[0]==values[locked][i]&&out.overrun==1&&out.timestamp==0x1234);}
  CHECK(m.releases==3&&!m.bad&&(m.words[CN_RF0R/4]&16u));
  CHECK(cn_receive(&c,&out)==CN_MORE&&m.releases==3);releases+=m.releases;
  CHECK(cn_ack_overrun(&c)==CN_OK&&!(m.words[CN_RF0R/4]&16u));
  CHECK(inject(&m,0x123,7));m.fifo[0].frame.length=15;out.frame.identifier=0x555;
  CHECK(cn_receive(&c,&out)==CN_ERROR&&out.frame.identifier==0x555&&m.releases==4);
 }
 CnQueue q={0};CnRx r={0};for(unsigned i=1;i<=4;++i){r.frame.data[0]=(uint8_t)(i*10);CHECK(cn_queue_put(&q,&r)==CN_OK);}
 CHECK(q.loss==1);for(unsigned i=2;i<=4;++i)CHECK(cn_queue_get(&q,&r)==CN_FRAME&&r.frame.data[0]==i*10);
 CHECK(cn_queue_get(&q,&r)==CN_MORE);
 cn_values("fifo",releases,30,40,1);return 0;
}
static int mailbox(void){
 Model m;model_init(&m);CnController c={0};CHECK(!start(&c,&m,0,1));
 SDCan f={0x123,0,0,8,{0,1,2,3,4,5,6,7}};uint32_t token=0;unsigned result=99;
 CHECK(cn_send(&c,&f,&token)==CN_OK&&token==1);
 CHECK(m.words[CN_TI0R/4]==0x24600001&&m.words[CN_TDL0R/4]==0x03020100&&m.words[CN_TDH0R/4]==0x07060504);
 m.words[CN_TSR/4]|=4;CHECK(cn_tx_poll(&c,token)==CN_MORE&&cn_take(&c,token,&result)==CN_BUSY);
 CHECK(cn_send(&c,&f,&token)==CN_BUSY&&token==1);
 m.words[CN_TSR/4]|=(1u<<26)|3u;CHECK(cn_tx_poll(&c,token)==CN_OK);
 CHECK(cn_take(&c,token,&result)==CN_OK&&result==CN_SENT);
 CHECK(cn_take(&c,token,&result)==CN_INVALID);
 CHECK(cn_send(&c,&f,&token)==CN_OK&&token==2);
 CHECK(cn_tx_poll(&c,1)==CN_INVALID&&c.tx_state==1);
 CHECK(cn_cancel(&c,token)==CN_MORE&&m.abort_requests==1);
 CHECK(cn_take(&c,token,&result)==CN_BUSY&&cn_recover(&c,3)==CN_BUSY);
 CHECK(cn_cancel(&c,token)==CN_MORE&&m.abort_requests==1);
 m.words[CN_TSR/4]|=(1u<<26)|3u;CHECK(cn_tx_poll(&c,token)==CN_OK);
 CHECK(cn_take(&c,token,&result)==CN_OK&&result==CN_SENT); /* success won abort */
 CHECK(cn_send(&c,&f,&token)==CN_OK);CHECK(cn_cancel(&c,token)==CN_MORE);
 m.words[CN_TSR/4]|=(1u<<26)|1u;CHECK(cn_tx_poll(&c,token)==CN_OK);
 CHECK(cn_take(&c,token,&result)==CN_OK&&result==CN_ABORTED);
 c.last_token=UINT32_MAX;unsigned before=m.writes;CHECK(cn_send(&c,&f,&token)==CN_UNAVAILABLE&&m.writes==before&&!m.bad);
 cn_values("mailbox",1,2,m.abort_requests,UINT32_MAX);return 0;
}
static int recovery(void){
 Model m;model_init(&m);CnController c={0};CHECK(!start(&c,&m,1,1));
 SDCan f={0x123,0,0,1,{1}};uint32_t token;unsigned result;
 CHECK(cn_send(&c,&f,&token)==CN_OK);m.words[CN_TSR/4]=(1u<<26)|5u;
 CHECK(cn_tx_poll(&c,token)==CN_OK&&cn_take(&c,token,&result)==CN_OK&&result==CN_LOST);
 CHECK(cn_send(&c,&f,&token)==CN_OK);m.words[CN_ESR/4]=4;
 CHECK(cn_tx_poll(&c,token)==CN_UNAVAILABLE&&c.tx_state==1);
 CHECK(cn_recover(&c,3)==CN_BUSY&&cn_cancel(&c,token)==CN_MORE);
 m.words[CN_TSR/4]=(1u<<26)|1u;CHECK(cn_tx_poll(&c,token)==CN_OK&&cn_take(&c,token,&result)==CN_OK&&result==CN_ABORTED);
 CHECK(cn_send(&c,&f,&token)==CN_UNAVAILABLE);
 CHECK(cn_recover(&c,3)==CN_MORE&&cn_control(&c)==CN_MORE);
 CHECK(cn_control(&c)==CN_MORE&&cn_control(&c)==CN_MORE&&cn_control(&c)==CN_UNAVAILABLE);
 CHECK(cn_control(&c)==CN_UNAVAILABLE&&cn_send(&c,&f,&token)==CN_UNAVAILABLE);
 CHECK(cn_recover(&c,3)==CN_MORE&&cn_control(&c)==CN_MORE);
 m.words[CN_ESR/4]=0;CHECK(cn_control(&c)==CN_OK);
 CHECK(cn_send(&c,&f,&token)==CN_OK&&token==3&&!m.bad);
 Model blocked;model_init(&blocked);blocked.block_init=1;CnController d={0};CnIO io={&blocked,model_read,model_write};SDTiming t;
 CHECK(sd_can_timing(42000000,6,11,2,1,&t)==SD_OK&&cn_begin(&d,io,&t,0,0,1)==CN_MORE);
 CHECK(cn_control(&d)==CN_UNAVAILABLE&&blocked.words[CN_BTR/4]==0);
 cn_values("recovery",3,1,128,11);return 0;
}
static int port(void){
 model_init(&mapped);CnController c={0};SDTiming t;CHECK(sd_can_timing(42000000,6,11,2,1,&t)==SD_OK);
 CHECK(cn_begin(&c,cn_f439_io(),&t,0,1,3)==CN_MORE&&cn_control(&c)==CN_MORE&&cn_control(&c)==CN_OK);
 CHECK(inject(&mapped,0x123,0xa5));CnRx out={0};CHECK(cn_receive(&c,&out)==CN_FRAME&&out.frame.data[0]==0xa5);
 SDCan f={0x123,0,0,1,{0x5a}};uint32_t token;
 CHECK(cn_send(&c,&f,&token)==CN_OK&&cn_cancel(&c,token)==CN_MORE&&mapped.abort_requests==1&&!mapped.bad);
 cn_values("port",0x40006400,mapped.addresses,mapped.releases,0x1a0005);return 0;
}
int cn_controller_test(unsigned n){switch(n){case 5:return fifo();case 6:return mailbox();case 7:return recovery();case 8:return port();default:return 42;}}
