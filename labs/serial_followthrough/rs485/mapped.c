#include "channel.h"
void r5_note(const char *);
void r5_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){r5_note("R5_PORT_ASSERT: " #x);return 41;}}while(0)
struct ep_registers {volatile uint32_t sr,dr,brr,cr1,cr2,cr3,gtpr;};
static uint32_t rcc[64],gpio[16],dma[64];
static struct ep_registers usart;
static unsigned barriers,bad;
volatile uint32_t *pu_test_register(uint32_t a){
 if(a>=0x40023800u&&a<0x40023900u&&!(a&3))return &rcc[(a-0x40023800u)/4];
 if(a>=0x40020c00u&&a<0x40020c40u&&!(a&3))return &gpio[(a-0x40020c00u)/4];
 if(a>=0x40026000u&&a<0x40026100u&&!(a&3))return &dma[(a-0x40026000u)/4];
 ++bad;return &rcc[63];
}
/* Matches the producer's seven uint32_t register members. */
struct ep_registers *pu_test_usart(void){return &usart;}
volatile uint32_t *r5_test_register(uint32_t a){return pu_test_register(a);}
void r5_test_barrier(void){++barriers;}
int r5_mapped(void){
 for(unsigned i=0;i<64;++i){rcc[i]=0;dma[i]=0;}for(unsigned i=0;i<16;++i)gpio[i]=0xffffffffu;
 usart.sr=0;usart.dr=0;usart.brr=0;usart.cr1=0;usart.cr2=0;usart.cr3=0;usart.gtpr=0;
 barriers=0;bad=0;rcc[0]=2; /* selected reset HSI ready */
 r5_f439_prepare();
 CHECK((rcc[12]&(1u<<3))&&gpio[6]==(1u<<26));
 CHECK(((gpio[0]>>20)&3u)==1 && !(gpio[1]&(1u<<10)) && !(gpio[2]&(3u<<20)) && !(gpio[3]&(3u<<20)));
 CHECK((gpio[9]&255u)==0x77u); /* actual inherited USART3 AF7 */
 struct r5_io io=r5_f439_io();io.drive(io.context,1);CHECK(gpio[6]==(1u<<10));
 io.drive(io.context,0);CHECK(gpio[6]==(1u<<26)&&barriers==3);
 struct r5_channel s={0};struct pu_clock clock={16000000,1,1,1,1,0};struct r5_timing timing={R5_THVD1410,0,1000,2000};
 CHECK(r5_open(&s,io,&clock,100000,&timing,16,8,0)==R5_OK&&usart.brr==160);
 uint8_t bytes[]={0x55,0xaa};size_t accepted=99;
 CHECK(r5_f439_send(&s,1,bytes,2,PU_POLL,0,1000,&accepted)==R5_OK&&accepted==2&&gpio[6]==(1u<<10));
 CHECK(r5_step(&s,1,1)==R5_BUSY);
 usart.sr=PU_TXE|PU_TC;CHECK(r5_step(&s,1,1)==R5_BUSY&&usart.dr==0x55);
 usart.sr=PU_TXE;CHECK(r5_step(&s,1,101)==R5_BUSY&&usart.dr==0xaa&&gpio[6]==(1u<<10));
 usart.sr=PU_TC;CHECK(r5_step(&s,1,201)==R5_BUSY&&gpio[6]==(1u<<26));
 CHECK(r5_step(&s,1,204)==R5_OK&&!bad);
 r5_values("port",gpio[6],usart.brr,204,barriers);return 0;
}
