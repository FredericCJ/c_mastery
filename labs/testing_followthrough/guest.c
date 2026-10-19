#include <stdint.h>
volatile uint32_t svc_seen;
static void write0(const char *s){register uint32_t op __asm__("r0")=4;register const char *arg __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(op):"r"(arg):"memory");}
static void stop(void){register uint32_t op __asm__("r0")=0x18;register uint32_t arg __asm__("r1")=0x20026;__asm__ volatile("bkpt 0xab":"+r"(op):"r"(arg):"memory");for(;;){}}
void svc_handler(void){svc_seen=11;}
void reset_handler(void){
 volatile unsigned char p[6]={1,2,0x12,0x34,0xab,0xcd};uint32_t sum=0;svc_seen=0;
 unsigned count=p[1];
 if(p[0]!=1||count>4||2+2*count!=6){write0("GUEST_FAIL:grammar\n");stop();}
 for(unsigned i=0;i<count;i++)sum+=((uint32_t)p[2+2*i]<<8)|p[3+2*i];
 __asm__ volatile("svc 0");
 write0(sum==48641&&svc_seen==11?"GUEST_PASS:BE_SUM=48641:SVC=11\n":"GUEST_FAIL\n");stop();
}
