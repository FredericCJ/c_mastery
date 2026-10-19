#include <stdint.h>
static void semi(uint32_t op,const void *p){register uint32_t a __asm__("r0")=op;register const void *b __asm__("r1")=p;__asm__ volatile("bkpt 0xab":"+r"(a):"r"(b):"memory");}
void ep_note(const char *s){semi(4,s);semi(4,"\n");}
static void stop(unsigned n){uint32_t a[2]={0x20026u,n};semi(0x20,a);for(;;){}}
void xf_fault(void){ep_note("EP_GUEST_FAULT");stop(42);}
void xf_capture(uint32_t *a,uint32_t b,unsigned c,uint32_t d){(void)a;(void)b;(void)c;(void)d;xf_fault();}
int ep_access(void);int ep_lifecycle(void);int ep_bounded(void);int ep_mapped(void);int ep_leases(void);int ep_faults(void);
void xf_target_main(void){
 int (*const tests[])(void)={ep_access,ep_lifecycle,ep_bounded,ep_mapped,ep_leases,ep_faults};
 for(unsigned i=0;i<6;i++){int r=tests[i]();if(r)stop((unsigned)r);}
 ep_note("EP_GUEST_PASS actual-Thumb RAM-MDIO-model no-physical-PHY");stop(0);
}
