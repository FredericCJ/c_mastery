#include "model.h"
int ep_model_read(void *p,unsigned reg,uint32_t *out){
 EpModel *m=p;m->calls++;m->reads++;if(m->calls==m->fail_at)return 1;
 if(reg==1){*out=m->data;return 0;}
 if(reg!=0)return 1;
 if(m->ar&1){
  if(m->stuck || m->busy_reads){if(m->busy_reads)m->busy_reads--;*out=m->ar;return 0;}
  unsigned address=(m->ar>>11)&31,number=(m->ar>>6)&31;
  if(m->ar&2){
   m->phy[address][number]=m->data;
   if(number==0 && (m->data&0x8000u)){m->reset_value=m->data;m->reset_reads=2;}
   if(number==0 && (m->data&0x0200u)){m->restart_value=m->data;m->restart_reads=2;}
  }else{
   m->data=m->phy[address][number];
   if(number==0 && m->reset_reads){if(!--m->reset_reads)m->phy[address][0]&=~0x8000u;}
   if(number==0 && m->restart_reads){if(!--m->restart_reads)m->phy[address][0]&=~0x0200u;}
   if(number==1 && m->latch){m->data&=~4u;m->latch=0;}
  }
  m->ar&=~1u;m->transfers++;
 }
 *out=m->ar;return 0;
}
int ep_model_write(void *p,unsigned reg,uint32_t value){
 EpModel *m=p;m->calls++;if(m->calls==m->fail_at)return 1;
 if(m->ar&1){m->violation++;return 1;}
 m->writes++;
 if(reg==1){m->data=(uint16_t)value;return 0;}
 if(reg!=0)return 1;
 m->ar=value;if(value&1)m->busy_reads=m->delay;return 0;
}
uint32_t ep_model_clock(void *p){EpModel *m=p;if(!m->frozen)m->tick++;return m->tick;}
EDBus ep_model_bus(EpModel *m){EDBus b={m,ep_model_read,ep_model_write,ep_model_clock};return b;}
void ep_model_setup(EpModel *m,unsigned address){
 EpModel zero={0};*m=zero;m->delay=1;
 for(unsigned a=0;a<32;a++)for(unsigned r=0;r<32;r++)m->phy[a][r]=0xffff;
 m->phy[address][0]=0x1000;m->phy[address][1]=0x24;m->phy[address][2]=7;m->phy[address][3]=0xc13a;m->phy[address][31]=0x1018;
}
