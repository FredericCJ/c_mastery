#include "channel.h"
static void semi(uint32_t op,const void *p) {
    register uint32_t a __asm__("r0")=op;
    register const void *b __asm__("r1")=p;
    __asm__ volatile("bkpt 0xab" : "+r"(a) : "r"(b) : "memory");
}
static void text(const char *s){semi(4,s);}
void r5_note(const char *s){text(s);text("\n");}
static void number(unsigned n){char b[12];unsigned i=11;b[i]=0;do{b[--i]=(char)('0'+n%10);n/=10;}while(n);text(b+i);}
void r5_values(const char *s,unsigned a,unsigned b,unsigned c,unsigned d) {
    text("OBS rs485_");text(s);text(" ");number(a);text(" ");number(b);
    text(" ");number(c);text(" ");number(d);text("\n");
}
static void stop(unsigned c){uint32_t p[]={0x20026u,c};semi(0x20,p);for(;;){}}
R5Channel r5_target_channel __attribute__((section(".r5_dma"),aligned(4)));
static void placement(void){
    R5Channel *s=&r5_target_channel;
    *s=(R5Channel){0}; /* NOLOAD test region is initialized explicitly. */
    PuClock clock={16000000,1,1,1,1,0};R5Timing timing={R5_THVD1410,0,1000,2000};
    R5Io io=r5_f439_io();uint8_t byte=0x55;size_t accepted=99;
    if(r5_open(s,io,&clock,100000,&timing,16,8,0)!=R5_OK||
       r5_f439_send(s,1,&byte,1,PU_DMA,0,1000,&accepted)!=R5_OK||accepted!=1||
       r5_cancel(s,1,0)!=R5_OK||r5_step(s,1,3)!=R5_OK){text("R5_DMA_PLACEMENT_ASSERT\n");stop(41);}
    text("R5_DMA_PLACEMENT_PASS actual-AHB-address-admission\n");
}
void xf_fault(void){text("R5_GUEST_FAULT\n");stop(42);}
void xf_capture(uint32_t *sp,uint32_t er,unsigned irq,uint32_t msp){(void)sp;(void)er;(void)irq;(void)msp;xf_fault();}
void xf_target_main(void){for(unsigned i=0;i<6;++i){int r=r5_test(i);if(r)stop((unsigned)r);}placement();text("R5_GUEST_PASS actual-Thumb RAM-register-effects no-physical-RS485\n");stop(0);}
