#include "port.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"G4_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
struct g4_model {uint32_t reg[G4_REG_COUNT];unsigned reads[G4_REG_COUNT],writes[G4_REG_COUNT],ready,ack;};
static uint32_t rd(void *v,enum g4_reg r){struct g4_model *m=v;++m->reads[r];if(r==G4_RCC_CR && m->ready && m->reads[r]>=m->ready)m->reg[r]|=1u<<10;if(r==G4_USART_ISR && m->ack && m->reads[r]>=m->ack)m->reg[r]|=1u<<21;return m->reg[r];}
static void wr(void *v,enum g4_reg r,uint32_t x){struct g4_model *m=v;++m->writes[r];m->reg[r]=x;}
int main(void){
 struct g4_model m={0};m.ready=2;m.ack=3;struct g4_io io={&m,rd,wr};struct g4_init out={99,99,99};
 CHECK(g4_initialize(io,115200,4,&out)==G4_OK);CHECK(out.stage==5 && out.brr==139 && out.polls==4);
 CHECK(m.reg[G4_USART_CR1]==9 && m.reg[G4_USART_BRR]==139 && m.reg[G4_RCC_CCIPR]==8);
 CHECK(m.reg[G4_GPIO_AFRL]==7u<<8 && m.reg[G4_GPIO_MODER]==2u<<4);
 struct g4_model before=m;struct g4_init old=out;CHECK(g4_initialize(io,0,4,&out)==G4_INVALID);CHECK(!memcmp(&m,&before,sizeof m) && !memcmp(&out,&old,sizeof out));
 memset(&m,0,sizeof m);m.ready=2;CHECK(g4_initialize(io,115200,4,&out)==G4_ENABLE_TIMEOUT);CHECK(out.stage==4 && out.polls==5 && m.reg[G4_USART_CR1]==0);
 puts("G4_PASS admission initialization failure");return 0;
}
