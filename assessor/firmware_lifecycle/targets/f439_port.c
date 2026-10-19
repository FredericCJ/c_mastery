#include "f439_port.h"
#ifndef F4_PORT_MUTANT
#define F4_PORT_MUTANT 0
#endif
#define UART 0x40004400u
static int ok(FHIO x){return x.read&&x.write&&x.sync;}
int f4_cold(FHIO x){if(!ok(x))return 0;uint32_t id=x.read(x.ctx,0xE0042000u);if((id&0xFFFu)!=0x419u||((id>>16)!=0x2001u&&(id>>16)!=0x2003u))return 0;
 if((x.read(x.ctx,0x40023800u)&0x15000003u)!=3u||(x.read(x.ctx,0x40023808u)&0xFCFFu)||x.read(x.ctx,0xE000ED94u)||(x.read(x.ctx,0x40023C00u)&0x1F00u)||(x.read(x.ctx,0x40023C14u)&16u))return 0;
 x.write(x.ctx,0x40023844u,x.read(x.ctx,0x40023844u)|(1u<<14));(void)x.read(x.ctx,0x40023844u);x.sync(x.ctx);
 return F4_PORT_MUTANT==1||!(x.read(x.ctx,0x40013800u)&0x107u);
}
int f4_uart_init(FHIO x){if(!ok(x))return 0;
 x.write(x.ctx,0x40023830u,x.read(x.ctx,0x40023830u)|1u);x.write(x.ctx,0x40023840u,x.read(x.ctx,0x40023840u)|(1u<<17));(void)x.read(x.ctx,0x40023840u);x.sync(x.ctx);
 if(x.read(x.ctx,UART+12u))return 0;
 x.write(x.ctx,0x40020000u,(x.read(x.ctx,0x40020000u)&~0xF0u)|0xA0u);
 x.write(x.ctx,0x40020004u,x.read(x.ctx,0x40020004u)&~12u);
 x.write(x.ctx,0x40020008u,x.read(x.ctx,0x40020008u)&~0xF0u);
 x.write(x.ctx,0x4002000Cu,x.read(x.ctx,0x4002000Cu)&~0xF0u);
 x.write(x.ctx,0x40020020u,(x.read(x.ctx,0x40020020u)&~0xFF00u)|0x7700u);
 x.write(x.ctx,UART+16u,0);x.write(x.ctx,UART+20u,0);x.write(x.ctx,UART+8u,139);x.write(x.ctx,UART+12u,0x200Cu);x.sync(x.ctx);return 1;
}
int f4_uart_rx(FHIO x,unsigned char*out,uint32_t*status){if(!ok(x)||!out||!status)return -1;uint32_t sr=x.read(x.ctx,UART);*status=sr;
 /* For PE, the reference manual requires RXNE before the clearing DR read.
  * Any error poisons this request; no claim that all errors were cleared. */
 if(sr&15u){if((sr&32u)||!(sr&1u)){if(F4_PORT_MUTANT!=2)(void)x.read(x.ctx,UART+4u);}return -1;}
 if(!(sr&32u))return 0;*out=(unsigned char)x.read(x.ctx,UART+4u);return 1;
}
int f4_uart_tx(FHIO x,unsigned char b,unsigned budget){if(!ok(x)||!budget)return 0;unsigned n=budget;while(!(x.read(x.ctx,UART)&128u))if(!--n)return 0;x.write(x.ctx,UART+4u,b);n=budget;while(!(x.read(x.ctx,UART)&64u))if(!--n)return 0;x.sync(x.ctx);return 1;}
void f4_fresh(FHIO x){/* Selected caches remain disabled; reset both after mutation. */x.sync(x.ctx);uint32_t acr=x.read(x.ctx,0x40023C00u);x.write(x.ctx,0x40023C00u,acr|0x1800u);x.write(x.ctx,0x40023C00u,acr);x.sync(x.ctx);}
