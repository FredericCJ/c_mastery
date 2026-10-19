#include "model.h"
#include "../phy/model.h"
void en_note(const char *);
#define CHECK(x) do{if(!(x)){en_note("EN_ASSERT: " #x);return 41;}}while(0)
#ifdef EN_TARGET_IMAGE
static EnMemory memory __attribute__((section(".eth"),aligned(4)));
#else
static EnMemory memory;
#endif
static EnModel model;
static EnRing ring;
static EpModel phy_model;
static EpPhy phy;
static int boot(void){
 EnRing rz={0};EpPhy pz={0};ring=rz;phy=pz;
 ep_model_setup(&phy_model,0);EDBus b=ep_model_bus(&phy_model);
 CHECK(ep_init(&phy,&b,168000000,0,4,100)==ED_OK && ep_begin(&phy,16,1000)==ED_BUSY);
 int e=ED_BUSY;for(unsigned i=0;i<16 && e==ED_BUSY;i++)e=ep_step(&phy);CHECK(e==ED_OK);
 en_model_init(&model,&memory,0x20001000);EnBus bus=en_model_bus(&model);unsigned char mac[]={2,0,0,0,0,1};
 CHECK(en_init(&ring,&memory,0x20001000,&bus,&phy,mac)==ED_OK);
 CHECK(en_reset_begin(&ring,0,4,100)==ED_BUSY && en_reset_step(&ring,1)==ED_BUSY && en_reset_step(&ring,2)==ED_BUSY && en_reset_step(&ring,3)==ED_OK);
 CHECK(en_start(&ring)==ED_OK && !model.violation);return 0;
}
int en_initialization(void){
#ifdef EN_TARGET_IMAGE
 CHECK((uintptr_t)&memory==UINT32_C(0x20001000));
#endif
 CHECK(en_range(0x20000100,64)==ED_OK && en_range(0x10000100,64)==ED_INVALID && en_range(0x2002fff0,32)==ED_INVALID && en_range(0x20000101,64)==ED_INVALID && en_range(0x20000000,SIZE_MAX)==ED_INVALID);
 CHECK(boot()==0 && sizeof(memory)==12384);
 CHECK(model.reg[0x1000/4]==0x100 && model.reg[0x1018/4]==0x02202002 && model.reg[0]==0xc80c);
 CHECK(model.reg[0x40/4]==0x80000100u && model.reg[0x44/4]==2 && model.reg[0x100c/4]==0x20001000 && model.reg[0x1010/4]==0x20001040);
 for(unsigned i=0;i<4;i++)CHECK(memory.rx[i].word[0]==0x80000000u && memory.rx[i].word[1]==(2048u|(i==3?0x8000u:0)) && memory.rx[i].word[3]==0);
 CHECK(en_reset_begin(&ring,1,4,100)==ED_BUSY && ring.epoch==1);
 en_note("OBS ring_init 12384 35659778 51212 1");return 0;
}
int en_transmit(void){
 CHECK(boot()==0);unsigned char bytes[100];for(unsigned i=0;i<100;i++)bytes[i]=(unsigned char)i;
 EnTicket a={0},b={0},out={0};CHECK(en_tx_copy(&ring,bytes,100,&a)==ED_OK);bytes[0]=99;
 CHECK(memory.tx_bytes[a.slot][0]==0 && memory.tx[a.slot].word[0]==0 && en_tx_copy(&ring,bytes,14,&b)==ED_OK);
 CHECK(en_tx_copy(&ring,bytes,64,&out)==ED_BUSY && !out.owner && !out.serial);
 CHECK(en_tx_publish(&ring,a)==ED_OK && en_tx_publish(&ring,b)==ED_OK);
 uint32_t status=99;CHECK(en_tx_reap(&ring,a,&status)==ED_BUSY && status==99);
 CHECK(en_model_tx(&model,0)==ED_OK && model.wire_length==100 && model.wire[0]==0 && en_tx_reap(&ring,a,&status)==ED_OK && status==0);
 CHECK(en_model_tx(&model,1)==ED_OK && model.wire_length==60 && en_tx_reap(&ring,b,&status)==ED_IO && status==0x8002);
 for(unsigned k=0;k<32;k++){
  CHECK(en_tx_copy(&ring,bytes,64,&out)==ED_OK && en_tx_publish(&ring,out)==ED_OK);
  CHECK(en_model_tx(&model,0)==ED_OK && en_tx_reap(&ring,out,&status)==ED_OK);
 }
 CHECK(en_tx_publish(&ring,a)==ED_STALE && ring.transmitted==33 && ring.failed==1 && !model.violation);
 en_note("OBS ring_tx 34 33 1 60");return 0;
}
int en_receive(void){
 CHECK(boot()==0);unsigned char frame[1518]={0};EnFrame out={0};EnTicket held[4];
 for(unsigned i=0;i<4;i++){frame[0]=(unsigned char)i;CHECK(en_model_rx(&model,frame,64,(64u<<16)|0x300u)==ED_OK && en_rx_take(&ring,&out)==ED_OK && out.length==60 && out.bytes[0]==i);held[i]=out.ticket;}
 EnFrame unchanged=out;CHECK(en_rx_take(&ring,&out)==ED_BUSY && out.ticket.serial==unchanged.ticket.serial && out.bytes==unchanged.bytes);
 frame[0]=99;CHECK(en_model_rx(&model,frame,64,(64u<<16)|0x300u)==ED_BUSY && memory.rx_bytes[0][0]==0);
 CHECK(en_rx_release(&ring,held[0])==ED_OK && en_rx_release(&ring,held[0])==ED_STALE);
 CHECK(en_model_rx(&model,frame,1518,(1518u<<16)|0x300u)==ED_OK && en_rx_take(&ring,&out)==ED_OK && out.length==1514);
 CHECK(en_rx_release(&ring,held[0])==ED_STALE && en_rx_release(&ring,out.ticket)==ED_OK);
 for(unsigned i=1;i<4;i++)CHECK(en_rx_release(&ring,held[i])==ED_OK);
 for(unsigned k=0;k<16;k++){
  CHECK(en_model_rx(&model,frame,64,(64u<<16)|0x300u)==ED_OK && en_rx_take(&ring,&out)==ED_OK && en_rx_release(&ring,out.ticket)==ED_OK);
 }
 CHECK(ring.accepted==21 && ring.released==21 && !model.violation);en_note("OBS ring_rx 21 60 1514 21");return 0;
}
int en_recovery(void){
 CHECK(boot()==0);unsigned char bytes[64]={0};EnTicket ticket;EnFrame frame={0};
 CHECK(en_tx_copy(&ring,bytes,64,&ticket)==ED_OK && en_tx_publish(&ring,ticket)==ED_OK);
 CHECK(en_model_rx(&model,bytes,64,(64u<<16)|0x300u)==ED_OK && en_rx_take(&ring,&frame)==ED_OK);
 phy_model.phy[0][1]=0x20;CHECK(ep_poll(&phy)==ED_LINK_DOWN);
 EnTicket absent={0};CHECK(en_tx_copy(&ring,bytes,64,&absent)==ED_STALE && !absent.owner && ring.tx_phase[ticket.slot]==2);
 model.stop_stuck=1;CHECK(en_stop_begin(&ring,0,3,100)==ED_BUSY && en_stop_step(&ring,1)==ED_BUSY);
 CHECK(en_tx_copy(&ring,bytes,64,&absent)==ED_BUSY && (memory.tx[ticket.slot].word[0]&0x80000000u));
 model.reg[0x1014/4]&=~0x7e0000u;CHECK(en_stop_step(&ring,2)==ED_BUSY);
 CHECK(en_rx_release(&ring,frame.ticket)==ED_OK && memory.rx[frame.ticket.slot].word[0]==((64u<<16)|0x300u));
 CHECK(en_stop_step(&ring,3)==ED_OK && ring.phase==EN_BOUND && en_start(&ring)==ED_BUSY);
 CHECK(en_reset_begin(&ring,4,4,100)==ED_BUSY && ring.epoch==2);
 CHECK(en_tx_reap(&ring,ticket,&model.reg[900])==ED_STALE);
 CHECK(en_reset_step(&ring,5)==ED_BUSY && en_reset_step(&ring,6)==ED_BUSY && en_reset_step(&ring,7)==ED_OK && en_start(&ring)==ED_STALE);
 CHECK(!model.violation);en_note("OBS ring_recovery 2 1 0 0");return 0;
}
int en_drop_status(void){
 CHECK(boot()==0);unsigned char frame[2048]={0};EnFrame out={0};
 uint32_t bad[]={0x200u,(64u<<16)|0x100u,(64u<<16)|0x8300u,(64u<<16)|0x4300u,(63u<<16)|0x300u,(1519u<<16)|0x300u};
 for(unsigned i=0;i<6;i++)CHECK(en_model_rx(&model,frame,64,bad[i])==ED_OK && en_rx_take(&ring,&out)==ED_IO && !out.bytes);
 CHECK(ring.dropped==6 && en_model_rx(&model,frame,64,(64u<<16)|0x300u)==ED_OK && en_rx_take(&ring,&out)==ED_OK && en_rx_release(&ring,out.ticket)==ED_OK);
 model.reg[0x1014/4]|=0x18041u;uint32_t s=0;CHECK(en_status(&ring,&s)==ED_OK && (s&0x18041u)==0x18041u && !(model.reg[0x1014/4]&0x18041u));
 model.reg[0x1014/4]|=0x2000u;CHECK(en_status(&ring,&s)==ED_IO && ring.phase==EN_FAULT && (model.reg[0x1014/4]&0x2000u));
 en_note("OBS ring_drop 6 1 8192 0");return 0;
}
static int stop_reset(void){
 CHECK(en_stop_begin(&ring,0,4,100)==ED_BUSY && en_stop_step(&ring,1)==ED_OK);
 CHECK(en_reset_begin(&ring,2,4,100)==ED_BUSY && en_reset_step(&ring,3)==ED_BUSY && en_reset_step(&ring,4)==ED_BUSY && en_reset_step(&ring,5)==ED_OK);return 0;
}
int en_boundaries(void){
 CHECK(boot()==0);model.stop_stuck=1;
 CHECK(en_stop_begin(&ring,0,2,100)==ED_BUSY && en_stop_step(&ring,0)==ED_BUSY && en_stop_step(&ring,0)==ED_BUSY && en_stop_step(&ring,0)==ED_TIMEOUT && ring.phase==EN_STOPPING);
 model.stop_stuck=0;CHECK(en_stop_begin(&ring,1,2,100)==ED_BUSY && en_stop_step(&ring,2)==ED_OK);
 model.reset_stuck=1;CHECK(en_reset_begin(&ring,3,2,100)==ED_BUSY);unsigned writes=model.writes;
 CHECK(en_reset_step(&ring,3)==ED_BUSY && en_reset_step(&ring,3)==ED_BUSY && en_reset_step(&ring,3)==ED_TIMEOUT && ring.reset_pending);
 CHECK(en_start(&ring)==ED_BUSY && en_stop_begin(&ring,4,2,100)==ED_BUSY && model.writes==writes);
 model.reset_stuck=0;CHECK(en_reset_begin(&ring,4,4,100)==ED_BUSY && model.writes==writes && ring.epoch==2);
 CHECK(en_reset_step(&ring,5)==ED_BUSY && en_reset_step(&ring,6)==ED_BUSY && en_reset_step(&ring,7)==ED_OK && !ring.reset_pending && en_start(&ring)==ED_OK);
 unsigned char bytes[64]={0};EnTicket ticket={0};ring.serial=UINT32_MAX;
 CHECK(en_tx_copy(&ring,bytes,64,&ticket)==ED_FULL && !ticket.owner);
 CHECK(en_stop_begin(&ring,8,2,100)==ED_BUSY && en_stop_step(&ring,9)==ED_OK);ring.epoch=UINT32_MAX;
 CHECK(en_reset_begin(&ring,10,2,100)==ED_FULL && ring.epoch==UINT32_MAX);
 en_note("OBS ring_bounds 2 2 4294967295 0");return 0;
}
int en_failures(void){
 for(unsigned fault=1;fault<=27;fault++){
  CHECK(boot()==0 && stop_reset()==0);model.calls=0;model.fail_at=fault;
  CHECK(en_start(&ring)==ED_IO && ring.phase==EN_FAULT && !(model.reg[0x1018/4]&0x2002u));
 }
 CHECK(boot()==0);unsigned char bytes[64]={0};EnTicket ticket;
 CHECK(en_tx_copy(&ring,bytes,64,&ticket)==ED_OK);model.calls=0;model.fail_at=2;
 CHECK(en_tx_publish(&ring,ticket)==ED_IO && ring.phase==EN_FAULT && ring.tx_phase[ticket.slot]==2 && (memory.tx[ticket.slot].word[0]&0x80000000u));
 uint32_t out=99;CHECK(en_tx_reap(&ring,ticket,&out)==ED_BUSY && out==99);
 en_note("OBS ring_failures 27 27 2 1");return 0;
}
int en_consumer(void){
 CHECK(boot()==0);unsigned char bytes[64]={0};bytes[12]=8;bytes[14]=0x65;EnFrame frame;
 CHECK(en_model_rx(&model,bytes,64,(64u<<16)|0x300u)==ED_OK && en_rx_take(&ring,&frame)==ED_OK);
 /* Original bounded upper-layer version discriminator, not a full IP parser. */
 unsigned upper_rejected=(frame.length>=34 && (frame.bytes[14]>>4)!=4);
 CHECK(upper_rejected==1 && ring.accepted==1 && ring.dropped==0 && ring.released==0);
 CHECK(en_rx_release(&ring,frame.ticket)==ED_OK && ring.released==1 && en_rx_release(&ring,frame.ticket)==ED_STALE && ring.released==1);
 en_note("OBS ring_consumer 1 0 1 1");return 0;
}
