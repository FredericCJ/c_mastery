#include "port.h"
#include "compare.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"G4_MODEL_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
struct g4_event {enum g4_reg reg;uint32_t value;char op;};
struct g4_model {uint32_t reg[G4_REG_COUNT];struct g4_event events[100];unsigned count,cr,ack,ready_after,ack_after;};
static uint32_t rd(void *v,enum g4_reg r){struct g4_model *m=v;if(r==G4_RCC_CR && ++m->cr==m->ready_after)m->reg[r]|=1u<<10;if(r==G4_USART_ISR && ++m->ack==m->ack_after)m->reg[r]|=1u<<21;if(m->count<100)m->events[m->count++]=(struct g4_event){r,m->reg[r],'r'};return m->reg[r];}
static void wr(void *v,enum g4_reg r,uint32_t x){struct g4_model *m=v;m->reg[r]=x;if(m->count<100)m->events[m->count++]=(struct g4_event){r,x,'w'};}
static uint32_t tick(void *v){uint32_t *n=v;return (*n)++;}
static void init(struct g4_model *m,unsigned ready,unsigned ack){memset(m,0,sizeof *m);m->ready_after=ready;m->ack_after=ack;m->reg[G4_RCC_CCIPR]=0xa5a5a5a5u;m->reg[G4_GPIO_MODER]=0x55555555u;m->reg[G4_GPIO_OTYPER]=0xaaaau;m->reg[G4_GPIO_OSPEEDR]=0x33333333u;m->reg[G4_GPIO_PUPDR]=0x55555555u;m->reg[G4_GPIO_AFRL]=0xabcdef01u;m->reg[G4_RCC_APB1RSTR1]=1u<<8;}
int main(void){
 /* Independent integer table: nearest divisor, with exact-rational error
    checked by host oracle. This model records MMIO order, not a G474 emulator. */
 const uint32_t rates[]={245,300,1200,9600,57600,115200,230400,460800,921600,1000000};
 const uint32_t divisors[]={65306,53333,13333,1667,278,139,69,35,17,16};
 unsigned cases=0;
 for(unsigned n=0;n<sizeof rates/sizeof rates[0];n++)for(unsigned ready=2;ready<=5;ready++)for(unsigned ack=1;ack<=4;ack++){
  struct g4_model m;init(&m,ready,ack);struct g4_model before=m;struct g4_init out;struct g4_io io={&m,rd,wr};
  CHECK(g4_initialize(io,rates[n],4,&out)==G4_OK);CHECK(out.stage==5 && out.brr==divisors[n] && out.polls==ready-1+ack);
  CHECK(m.reg[G4_RCC_CCIPR]==((before.reg[G4_RCC_CCIPR]&~12u)|8u));
  CHECK(m.reg[G4_GPIO_MODER]==((before.reg[G4_GPIO_MODER]&~48u)|32u));
  CHECK(m.reg[G4_GPIO_OTYPER]==(before.reg[G4_GPIO_OTYPER]&~4u));
  CHECK(m.reg[G4_GPIO_OSPEEDR]==((before.reg[G4_GPIO_OSPEEDR]&~48u)|32u));
  CHECK(m.reg[G4_GPIO_PUPDR]==(before.reg[G4_GPIO_PUPDR]&~48u));
  CHECK(m.reg[G4_GPIO_AFRL]==((before.reg[G4_GPIO_AFRL]&~0xf00u)|0x700u));
  CHECK(m.reg[G4_RCC_APB1RSTR1]==before.reg[G4_RCC_APB1RSTR1]);
  CHECK(m.reg[G4_USART_CR1]==9 && !m.reg[G4_USART_CR2] && !m.reg[G4_USART_CR3] && !m.reg[G4_USART_PRESC]);
  int gpio_clock=-1,gpio_read=-1,usart_clock=-1,usart_read=-1,reset_high=-1,reset_low=-1,clock_select=-1,first_gpio=-1,disable=-1,brr=-1,enable=-1,ack_read=-1;
  for(unsigned j=0;j<m.count;j++){struct g4_event e=m.events[j];
   if(e.reg==G4_RCC_AHB2ENR){if(e.op=='w')gpio_clock=(int)j;else if(gpio_clock>=0)gpio_read=(int)j;}
   if(e.reg==G4_RCC_APB1ENR1){if(e.op=='w')usart_clock=(int)j;else if(usart_clock>=0)usart_read=(int)j;}
   if(e.op=='w' && e.reg==G4_RCC_APB1RSTR1){if(e.value&(1u<<17))reset_high=(int)j;else reset_low=(int)j;}
   if(e.op=='w' && e.reg==G4_RCC_CCIPR)clock_select=(int)j;
   if(e.op=='w' && e.reg==G4_GPIO_OTYPER)first_gpio=(int)j;
   if(e.op=='w' && e.reg==G4_USART_CR1){if(e.value==0)disable=(int)j;else enable=(int)j;}
   if(e.op=='w' && e.reg==G4_USART_BRR)brr=(int)j;
   if(e.op=='r' && e.reg==G4_USART_ISR)ack_read=(int)j;
  }
  CHECK(gpio_clock>=0 && gpio_read>gpio_clock && first_gpio>gpio_read);
  CHECK(usart_clock>=0 && usart_read>usart_clock && reset_high>usart_read && reset_low>reset_high);
  CHECK(clock_select>reset_low && first_gpio>clock_select && brr>disable && enable>brr && ack_read>enable);
  ++cases;
 }
 for(unsigned ready=0;ready<=6;ready++)for(unsigned ack=0;ack<=5;ack++){
  struct g4_model m;init(&m,ready,ack);struct g4_init out;struct g4_io io={&m,rd,wr};int status=g4_initialize(io,115200,4,&out);
  if(ready==0 || ready>5){CHECK(status==G4_CLOCK_TIMEOUT && out.stage==1 && out.polls==4);CHECK(!m.reg[G4_RCC_AHB2ENR] && !m.reg[G4_RCC_APB1ENR1]);}
  else if(ack==0 || ack>4){CHECK(status==G4_ENABLE_TIMEOUT && out.stage==4);CHECK(m.reg[G4_USART_CR1]==0 && (m.reg[G4_RCC_APB1ENR1]&(1u<<17)));}
  else CHECK(status==G4_OK);
  ++cases;
 }
 for(unsigned i=0;i<8;i++){
  struct g4_model m;init(&m,2,1);struct g4_model before=m;struct g4_init out={66,77,88},old=out;struct g4_io io={&m,rd,wr};uint32_t rate=115200;unsigned limit=4;struct g4_init *dst=&out;
  if(i==0)rate=0;else if(i==1)rate=244;else if(i==2)rate=1000001;else if(i==3)limit=0;else if(i==4)limit=1000001;else if(i==5)io.read=0;else if(i==6)io.write=0;else dst=0;
  CHECK(g4_initialize(io,rate,limit,dst)==G4_INVALID);CHECK(!memcmp(&m,&before,sizeof m) && !memcmp(&out,&old,sizeof out));++cases;
 }
 for(unsigned failure=0;failure<2;failure++){
  struct g4_model m;init(&m,failure?2:0,0);struct g4_init out;struct g4_io io={&m,rd,wr};
  CHECK(g4_initialize(io,115200,4,&out)==(failure?G4_ENABLE_TIMEOUT:G4_CLOCK_TIMEOUT));
  m.ready_after=m.cr+2;m.ack_after=m.ack+1;
  CHECK(g4_initialize(io,115200,4,&out)==G4_OK && out.stage==5 && m.reg[G4_USART_CR1]==9);
  CHECK((m.reg[G4_GPIO_AFRL]&~0xf00u)==(0xabcdef01u&~0xf00u));++cases;
 }
 /* Import the unchanged earlier byte mechanism using actual generated G474
    offsets. A memory array is explicitly a register model, not physical MMIO. */
 volatile uint32_t regs[12]={0};uint32_t counter=0;regs[7]=0xc0;struct pl_byte_port p=pl_generated(regs,1,tick,&counter);
 CHECK(p.status==regs+7 && p.data==regs+10);CHECK(pl_byte(p,0xa5,4)==0 && regs[10]==0xa5 && regs[1]==0);
 regs[7]=0;counter=0;CHECK(pl_byte(p,0x5a,3)==PL_TIMEOUT && regs[10]==0xa5);
 regs[7]=128;counter=0;CHECK(pl_byte(p,0x5a,3)==PL_TIMEOUT && regs[10]==0x5a);
 struct g4_model m;init(&m,0,0);struct g4_io io={&m,rd,wr};CHECK(g4_clock_probe(io)==G4_CLOCK_TIMEOUT && m.count==1);
 m.reg[G4_RCC_CR]|=1u<<10;m.reg[G4_RCC_CFGR]=0x81001234u;m.reg[G4_GPIO_AFRH]=0xabcdef0fu;struct g4_model before=m;
 CHECK(g4_clock_probe(io)==G4_OK);
 CHECK(m.reg[G4_RCC_CFGR]==0xc3001234u && m.reg[G4_GPIO_AFRH]==0xabcdef00u);
 CHECK(m.reg[G4_GPIO_MODER]==((before.reg[G4_GPIO_MODER]&~(3u<<16))|(2u<<16)));
 CHECK(m.reg[G4_GPIO_OTYPER]==(before.reg[G4_GPIO_OTYPER]&~(1u<<8)));
 CHECK(m.reg[G4_GPIO_PUPDR]==(before.reg[G4_GPIO_PUPDR]&~(3u<<16)));
 CHECK(m.reg[G4_GPIO_OSPEEDR]==((before.reg[G4_GPIO_OSPEEDR]&~(3u<<16))|(2u<<16)));
 CHECK(g4_clock_probe((struct g4_io){&m,0,wr})==G4_INVALID);CHECK(g4_clock_probe((struct g4_io){&m,rd,0})==G4_INVALID);
 printf("G4_MODEL_PASS cases=%u transfer=3 probe=4\n",cases);return 0;
}
