#include "ring.h"
#include "bringup.h"
#ifndef EN_MUTANT
#define EN_MUTANT 0
#endif
#define OWN UINT32_C(0x80000000)
#define POLICY UINT32_C(0x02200000)
#define DMA_STATES UINT32_C(0x007e0000)
#define W1C UINT32_C(0x0001e7ff)
enum {MACCR=0,MACFFR=4,MACFCR=0x18,MACIMR=0x3c,A0HR=0x40,A0LR=0x44,BMR=0x1000,TPDR=0x1004,RPDR=0x1008,RDLAR=0x100c,TDLAR=0x1010,SR=0x1014,OMR=0x1018,IER=0x101c};
static int valid(const struct en_ring *r){return r && r->initialized && r->memory;}
static int rd(struct en_ring *r,unsigned o,uint32_t *v){if(r->bus.read(r->bus.context,o,v)){r->phase=EN_FAULT;return ED_IO;}return ED_OK;}
static int wr(struct en_ring *r,unsigned o,uint32_t v){if(r->bus.settle(r->bus.context) || r->bus.write(r->bus.context,o,v)){r->phase=EN_FAULT;return ED_IO;}return ED_OK;}
static void fence(struct en_ring *r){r->bus.barrier(r->bus.context);}
static int tick(struct en_ring *r,uint32_t now){
 if(!r->remaining || now<r->last || now-r->start>=r->deadline)return ED_TIMEOUT;
 r->remaining--;r->last=now;return ED_OK;
}
static int held(const struct en_ring *r){for(unsigned i=0;i<EN_RX_COUNT;i++)if(r->rx_held[i])return 1;return 0;}
static int ready(struct en_ring *r){if(r->phase!=EN_RUNNING)return ED_BUSY;return EN_MUTANT==7?ED_OK:ep_validate(r->phy,&r->link);}
int en_range(uint32_t base,size_t length){
 if((base&3u) || length>UINT32_MAX)return ED_INVALID;
 return bu_f439_dma((struct bu_range){base,(uint32_t)length})?ED_OK:ED_INVALID;
}
int en_init(struct en_ring *r,struct en_memory *m,uint32_t base,const struct en_bus *b,struct ep_phy *phy,const unsigned char mac[6]){
 if(!r || !m || !b || !b->read || !b->write || !b->barrier || !b->settle || !phy || !mac || en_range(base,sizeof(*m)) || (mac[0]&1u))return ED_INVALID;
 unsigned any=0;for(unsigned i=0;i<6;i++)any|=mac[i];
 if(!any)return ED_INVALID;
 if(r->initialized)return ED_BUSY;
 struct en_ring next={0};next.memory=m;next.base=base;next.bus=*b;next.phy=phy;next.initialized=1;for(unsigned i=0;i<6;i++)next.mac[i]=mac[i];*r=next;return ED_OK;
}
int en_reset_begin(struct en_ring *r,uint32_t now,unsigned polls,uint32_t timeout){
 if(!valid(r) || !polls || polls>1024 || !timeout)return ED_INVALID;
 /* Explicit fresh finite observation window; do not rewrite a busy core. */
 if(r->reset_pending){r->start=r->last=now;r->deadline=timeout;r->remaining=polls;r->phase=EN_RESETTING;return ED_BUSY;}
 if((r->phase!=EN_BOUND && r->phase!=EN_STOPPED) || held(r))return ED_BUSY;
 if(r->epoch==UINT32_MAX)return ED_FULL;
 uint32_t status;
 int e=rd(r,SR,&status);
 if(e)return e;
 
 if(status&DMA_STATES)return ED_BUSY;
 r->start=r->last=now;r->deadline=timeout;r->remaining=polls;r->epoch++;
 e=wr(r,BMR,1);
 if(e)return e;
 r->reset_pending=1;r->phase=EN_RESETTING;return ED_BUSY;
}
int en_reset_step(struct en_ring *r,uint32_t now){
 if(!valid(r))return ED_INVALID;
 if(r->phase!=EN_RESETTING)return ED_STALE;
 int e=tick(r,now);
 if(e)return e;
 uint32_t v;e=rd(r,BMR,&v);
 if(e)return e;
 
 if((v&1u) && EN_MUTANT!=5)return ED_BUSY;
 r->reset_pending=0;r->phase=EN_STOPPED;return ED_OK;
}
int en_start(struct en_ring *r){
 if(!valid(r))return ED_INVALID;
 if(r->phase!=EN_STOPPED || held(r))return ED_BUSY;
 struct ep_link link;
 int e=ep_snapshot(r->phy,&link);
 if(e)return e;
 
 uint32_t state;e=rd(r,SR,&state);
 if(e)return e;
 
 if(state&DMA_STATES)return ED_BUSY;
 struct en_memory *m=r->memory;
 for(unsigned i=0;i<EN_RX_COUNT;i++){
  m->rx[i].word[0]=0;m->rx[i].word[1]=EN_BUFFER|(i==EN_RX_COUNT-1?0x8000u:0);
  m->rx[i].word[2]=r->base+(uint32_t)offsetof(struct en_memory,rx_bytes)+i*EN_BUFFER;m->rx[i].word[3]=0;r->rx_held[i]=0;
 }
 for(unsigned i=0;i<EN_TX_COUNT;i++){
  for(unsigned j=0;j<4;j++)m->tx[i].word[j]=0;
  m->tx[i].word[2]=r->base+(uint32_t)offsetof(struct en_memory,tx_bytes)+i*EN_BUFFER;r->tx_phase[i]=0;r->tx_length[i]=0;
 }
 uint32_t cr=UINT32_C(0x8000)|0x800u|(link.mode.speed==100?0x4000u:0);
 uint32_t lo=(uint32_t)r->mac[0]|((uint32_t)r->mac[1]<<8)|((uint32_t)r->mac[2]<<16)|((uint32_t)r->mac[3]<<24);
 uint32_t hi=OWN|(uint32_t)r->mac[4]|((uint32_t)r->mac[5]<<8);
 unsigned regs[]={BMR,MACCR,MACFFR,MACFCR,MACIMR,A0HR,A0LR,IER,RDLAR,TDLAR,SR};
 uint32_t vals[]={0x100u,cr,0,0,0x208u,hi,lo,0,r->base+(uint32_t)offsetof(struct en_memory,rx),r->base+(uint32_t)offsetof(struct en_memory,tx),state&W1C};
 for(unsigned i=0;i<sizeof(regs)/sizeof(regs[0]);i++){e=wr(r,regs[i],vals[i]);
 if(e)return e;
 }
 fence(r);for(unsigned i=0;i<EN_RX_COUNT;i++)m->rx[i].word[0]=OWN;
 fence(r);
 e=wr(r,MACCR,cr|12u);
 if(e)return e;
 
 e=wr(r,OMR,POLICY|0x2002u);
 if(e)return e;
 
 r->link=link;r->rx_next=r->tx_next=r->dropping=0;r->phase=EN_RUNNING;return ED_OK;
}
int en_tx_copy(struct en_ring *r,const void *bytes,size_t n,struct en_ticket *out){
 if(!valid(r) || !bytes || !out || n<14 || n>1514)return ED_INVALID;
 int e=ready(r);
 if(e)return e;
 
 unsigned i=r->tx_next;
 if(r->tx_phase[i] || (r->memory->tx[i].word[0]&OWN))return ED_BUSY;
 if(r->serial==UINT32_MAX)return ED_FULL;
 const unsigned char *p=bytes;for(size_t j=0;j<n;j++)r->memory->tx_bytes[i][j]=p[j];
 r->tx_length[i]=(uint16_t)n;r->tx_serial[i]=++r->serial;r->tx_phase[i]=1;r->tx_next=(i+1)%EN_TX_COUNT;
 struct en_ticket t={r,r->epoch,r->tx_serial[i],i};*out=t;return ED_OK;
}
static int tx_ticket(const struct en_ring *r,struct en_ticket t){return t.owner==r && t.slot<EN_TX_COUNT && t.epoch==r->epoch && t.serial && t.serial==r->tx_serial[t.slot];}
int en_tx_publish(struct en_ring *r,struct en_ticket t){
 if(!valid(r))return ED_INVALID;
 if(!tx_ticket(r,t))return ED_STALE;
 int e=ready(r);
 if(e)return e;
 
 if(r->tx_phase[t.slot]!=1)return ED_BUSY;
 struct en_descriptor *d=&r->memory->tx[t.slot];
 if(d->word[0]&OWN)return ED_BUSY;
 uint32_t control=UINT32_C(0x70000000)|(t.slot==EN_TX_COUNT-1?0x200000u:0);
 d->word[1]=r->tx_length[t.slot];d->word[2]=r->base+(uint32_t)offsetof(struct en_memory,tx_bytes)+t.slot*EN_BUFFER;d->word[3]=0;
 if(EN_MUTANT!=1)d->word[0]=control;
 fence(r);d->word[0]|=OWN;
 fence(r);r->tx_phase[t.slot]=2;
 return wr(r,TPDR,0);
}
int en_tx_reap(struct en_ring *r,struct en_ticket t,uint32_t *out){
 if(!valid(r) || !out)return ED_INVALID;
 if(!tx_ticket(r,t))return ED_STALE;
 if(r->phase!=EN_RUNNING || r->tx_phase[t.slot]!=2)return ED_BUSY;
 uint32_t status=r->memory->tx[t.slot].word[0];
 if(status&OWN)return ED_BUSY;
 fence(r);r->tx_phase[t.slot]=0;*out=status;
 if(status&0x8000u){r->failed++;return ED_IO;}r->transmitted++;return ED_OK;
}
static int recycle(struct en_ring *r,unsigned i){
 fence(r);r->memory->rx[i].word[0]=OWN;
 fence(r);r->rx_next=(i+1)%EN_RX_COUNT;
 return wr(r,RPDR,0);
}
int en_rx_take(struct en_ring *r,struct en_frame *out){
 if(!valid(r) || !out)return ED_INVALID;
 int e=ready(r);
 if(e)return e;
 
 unsigned i=r->rx_next;
 if(r->rx_held[i] && EN_MUTANT!=3)return ED_BUSY;
 uint32_t s=r->memory->rx[i].word[0];
 if(s&OWN)return ED_BUSY;
 fence(r);
 unsigned fs=(s>>9)&1u,ls=(s>>8)&1u;
 uint32_t length=(s>>16)&0x3fffu;
 if(fs)r->dropping=0;
 if(!fs || !ls || (s&0xc000u) || length<64 || length>1518 || r->dropping){
  r->dropping=!ls;if(r->dropped!=UINT32_MAX)r->dropped++;e=recycle(r,i);return e?e:ED_IO;
 }
 if(r->serial==UINT32_MAX)return ED_FULL;
 r->rx_serial[i]=++r->serial;r->rx_held[i]=1;r->accepted++;r->rx_next=(i+1)%EN_RX_COUNT;
 struct en_frame f={r->memory->rx_bytes[i],length-(EN_MUTANT==2?8u:4u),{r,r->epoch,r->rx_serial[i],i}};*out=f;return ED_OK;
}
int en_rx_release(struct en_ring *r,struct en_ticket t){
 if(!valid(r))return ED_INVALID;
 if(t.owner!=r || t.slot>=EN_RX_COUNT || t.epoch!=r->epoch || !t.serial || !r->rx_held[t.slot] || t.serial!=r->rx_serial[t.slot])return ED_STALE;
 r->rx_held[t.slot]=0;r->released++;
 /* A stopping/faulted engine is never given a new buffer. */
 if(r->phase!=EN_RUNNING)return ED_OK;
 fence(r);r->memory->rx[t.slot].word[0]=OWN;
 fence(r);return wr(r,RPDR,0);
}
int en_status(struct en_ring *r,uint32_t *out){
 if(!valid(r) || !out)return ED_INVALID;
 uint32_t s;
 int e=rd(r,SR,&s);
 if(e)return e;
 r->raw_status=s;*out=s;
 if(s&0x2000u){r->phase=EN_FAULT;return ED_IO;}
 return wr(r,SR,EN_MUTANT==6?UINT32_MAX:s&W1C);
}
int en_stop_begin(struct en_ring *r,uint32_t now,unsigned polls,uint32_t timeout){
 if(!valid(r) || !polls || polls>1024 || !timeout)return ED_INVALID;
 if(r->reset_pending || (r->phase!=EN_RUNNING && r->phase!=EN_FAULT && r->phase!=EN_STOPPING))return ED_BUSY;
 r->start=r->last=now;r->deadline=timeout;r->remaining=polls;
 int e=wr(r,OMR,POLICY);
 if(e)return e;
 r->phase=EN_STOPPING;return ED_BUSY;
}
int en_stop_step(struct en_ring *r,uint32_t now){
 if(!valid(r))return ED_INVALID;
 if(r->phase!=EN_STOPPING)return ED_STALE;
 int e=tick(r,now);
 if(e)return e;
 
 uint32_t s;e=rd(r,SR,&s);
 if(e)return e;
 r->raw_status=s;
 if(((s&DMA_STATES) && EN_MUTANT!=4) || held(r))return ED_BUSY;
 uint32_t cr;e=rd(r,MACCR,&cr);
 if(e)return e;
 e=wr(r,MACCR,cr&~12u);
 if(e)return e;
 
 /* Device process state, not an interrupt or host timeout, is the memory
  * quiescence witness. A new reset epoch is still required before reuse. */
 r->phase=EN_BOUND;return ED_OK;
}
