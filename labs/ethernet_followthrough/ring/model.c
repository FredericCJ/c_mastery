#include "model.h"
#define OWN UINT32_C(0x80000000)
static int fault(struct en_model *m){return ++m->calls==m->fail_at;}
static int read_register(void *p,unsigned offset,uint32_t *out){
 struct en_model *m=p;if((offset&3u) || offset/4>=1032 || !out || fault(m))return 1;
 if(offset==0x1000 && (m->reg[offset/4]&1u) && !m->reset_stuck){
  if(m->reset_reads)m->reset_reads--;else m->reg[offset/4]&=~1u;
 }
 *out=m->reg[offset/4];return 0;
}
static int write_register(void *p,unsigned offset,uint32_t value){
 struct en_model *m=p;if((offset&3u) || offset/4>=1032 || fault(m))return 1;
 if(!m->settled){m->violation++;return 1;}m->settled=0;m->writes++;
 if((offset==0x100c || offset==0x1010) && ((m->reg[0x1014/4]&0x7e0000u) || (m->reg[0x1000/4]&1u))){m->violation++;return 1;}
 if(offset==0x1000 && (value&1u)){
  for(unsigned i=0;i<1032;i++)m->reg[i]=0;
  m->reset_reads=2;m->tx_current=m->rx_current=0;
 }
 if(offset==0x1014){if(value&~0x1e7ffu){m->violation++;return 1;}m->reg[offset/4]&=~value;return 0;}
 if(offset==0x1018){
  if((value&0x02200000u)!=0x02200000u || (value&0xc0u)){m->violation++;return 1;}
  if(value&0x2002u){
   if(m->reg[0x100c/4]!=m->base || m->reg[0x1010/4]!=m->base+64 || (m->reg[0]&12u)!=12u || (m->reg[0x1000/4]&0xfcu)){m->violation++;return 1;}
   m->reg[0x1014/4]=(m->reg[0x1014/4]&~0x7e0000u)|0x160000u;
  }else if(!m->stop_stuck)m->reg[0x1014/4]&=~0x7e0000u;
 }
 if(offset==0x1004)m->tx_demands++;
 if(offset==0x1008)m->rx_demands++;
 m->reg[offset/4]=value;return 0;
}
static void barrier(void *p){struct en_model *m=p;m->fences++;}
static int settle(void *p){struct en_model *m=p;if(fault(m))return 1;m->settled=1;return 0;}
void en_model_init(struct en_model *m,struct en_memory *memory,uint32_t base){struct en_model zero={0};*m=zero;m->memory=memory;m->base=base;}
struct en_bus en_model_bus(struct en_model *m){struct en_bus b={m,read_register,write_register,barrier,settle};return b;}
int en_model_tx(struct en_model *m,unsigned error){
 if(!m || error>1 || !(m->reg[0x1018/4]&0x2000u))return ED_INVALID;
 unsigned i=m->tx_current;struct en_descriptor *d=&m->memory->tx[i];uint32_t s=d->word[0];
 if(!(s&OWN)){m->reg[0x1014/4]=(m->reg[0x1014/4]&~0x700000u)|0x600004u;return ED_BUSY;}
 uint32_t expected=0xf0000000u|(i==1?0x200000u:0);
 size_t n=d->word[1]&0x1fffu;
 if(s!=expected || d->word[1]!=n || n<14 || n>1514 || d->word[2]!=m->base+(uint32_t)offsetof(struct en_memory,tx_bytes)+i*EN_BUFFER || d->word[3]){m->violation++;return ED_INVALID;}
 size_t padded=n<60?60:n;for(size_t j=0;j<padded;j++)m->wire[j]=j<n?m->memory->tx_bytes[i][j]:0;
 /* The model captures bytes before MAC FCS insertion; no wire CRC claim. */
 m->wire_length=(unsigned)padded;d->word[0]=error?0x8002u:0;m->reg[0x1014/4]|=1u;
 m->tx_current=(s&0x200000u)?0:i+1;return ED_OK;
}
int en_model_rx(struct en_model *m,const void *bytes,size_t n,uint32_t status){
 if(!m || !bytes || !n || n>EN_BUFFER || (status&OWN) || !(m->reg[0x1018/4]&2u))return ED_INVALID;
 unsigned i=m->rx_current;struct en_descriptor *d=&m->memory->rx[i];
 if(!(d->word[0]&OWN)){m->reg[0x1014/4]=(m->reg[0x1014/4]&~0xe0000u)|0x80080u;return ED_BUSY;}
 if(d->word[1]!=(EN_BUFFER|(i==3?0x8000u:0)) || d->word[2]!=m->base+(uint32_t)offsetof(struct en_memory,rx_bytes)+i*EN_BUFFER || d->word[3]){m->violation++;return ED_INVALID;}
 const unsigned char *p=bytes;for(size_t j=0;j<n;j++)m->memory->rx_bytes[i][j]=p[j];
 d->word[0]=status;m->reg[0x1014/4]|=64u;m->rx_current=i==3?0:i+1;return ED_OK;
}
