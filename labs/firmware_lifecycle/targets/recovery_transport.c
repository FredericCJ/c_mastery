#include "recovery_transport.h"
void rt_reset(RTFrame*f){if(f){f->used=0;f->need=12;f->state=0;}}
enum RTResult rt_byte(RTFrame*f,unsigned char c){(void)f;(void)c;return RT_BAD;}
int rt_admit(const RTFrame*f,const FBPolicy*p,const BSLayouts*l,FBImage*out){(void)f;(void)p;(void)l;(void)out;return 0;}
int rt_uart_init(FHIO i,unsigned b){(void)i;(void)b;return -1;}
int rt_uart_rx(FHIO i,unsigned char*b,uint32_t*s){(void)i;(void)b;(void)s;return -1;}
int rt_uart_tx(FHIO i,unsigned char b,unsigned n){(void)i;(void)b;(void)n;return -1;}
int rt_uart_step(FHIO i,RTFrame*f,uint32_t*s){(void)i;(void)f;(void)s;return -1;}
