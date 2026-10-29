#include "halfduplex.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CPX_CHECK: %s:%d: %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
struct cpx_pins {unsigned enabled,changes,bytes;};
static void direction(void *context,int on){struct cpx_pins *p=context;p->enabled=(unsigned)on;++p->changes;}
static void write_byte(void *context,unsigned char byte){struct cpx_pins *p=context;(void)byte;CHECK(p->enabled);++p->bytes;}
int main(void){struct cpx_half_duplex h={0};struct cpx_pins p={0};struct cp_message m={0};m.sequence=1;m.length=1;m.payload[0]=42;
 CHECK(cpx_halfduplex_begin(&h,&m,3,direction,&p)==CPF_OK && p.enabled && p.changes==1);
 for(size_t i=0;i<h.tx.length;++i)CHECK(cpx_halfduplex_step(&h,65530,1,1,write_byte,&p)==CPF_PENDING && p.enabled);
 CHECK(cpx_halfduplex_step(&h,65534,1,0,write_byte,&p)==CPF_PENDING && p.enabled);
 CHECK(cpx_halfduplex_step(&h,65535,1,1,write_byte,&p)==CPF_PENDING && p.enabled);
 CHECK(cpx_halfduplex_step(&h,1,1,1,write_byte,&p)==CPF_PENDING && p.enabled);
 CHECK(cpx_halfduplex_step(&h,2,1,1,write_byte,&p)==CPF_OK && !p.enabled && p.changes==2 && p.bytes==11);
 CHECK(cpx_halfduplex_begin(&h,&m,32768,direction,&p)==CPF_INVALID && p.changes==2);
 puts("CPX_PASS TXE/TC distinction, direction hold, turnaround and timestamp wrap");return 0;}
