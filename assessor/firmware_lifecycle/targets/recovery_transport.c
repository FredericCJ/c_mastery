#include "recovery_transport.h"
#ifndef RT_MUTANT
#define RT_MUTANT 0
#endif
#define UART 0x40004800u
#define GPIO 0x58020C00u
#define RCC 0x58024400u
static int io_ok(FHIO x){return x.read&&x.write&&x.sync;}
void rt_reset(RTFrame*f){if(f){f->used=0;f->need=12;f->state=0;}}
enum RTResult rt_byte(RTFrame*f,unsigned char c){
 if(!f)return RT_BAD;if(f->state||f->used>=f->need||f->need>RT_CAPACITY||f->need<12){f->state=2;return RT_BAD;}
 f->bytes[f->used++]=c;
 if(f->used==12){const unsigned char*p=f->bytes;unsigned n=(unsigned)p[8]|(unsigned)p[9]<<8;
  if(p[0]!='B'||p[1]!='6'||p[2]!='U'||p[3]!='P'||p[4]!=1||p[5]>1||p[6]>1||p[7]||p[10]||p[11]||n<546||n>2080){f->state=2;return RT_BAD;}f->need=12+n+64;}
 if(f->used==f->need){f->state=1;return RT_READY;}return RT_MORE;
}
int rt_admit(const RTFrame*f,const FBPolicy*p,const BSLayouts*l,FBImage*out){
 if(!f||!p||!l||!out||f->state!=1||f->used!=f->need||f->need<622||f->need>RT_CAPACITY)return 0;
 unsigned core=f->bytes[5],slot=f->bytes[6],n=(unsigned)f->bytes[8]|(unsigned)f->bytes[9]<<8;
 if(core>1||slot>1||f->need!=12+n+64)return 0;
 if(RT_MUTANT==1){out->version=1;return 1;}
 return fb_admit(p,l->core[core][slot],core,f->bytes+12,n,f->bytes+12+n,out);
}
int rt_uart_init(FHIO x,unsigned budget){
 if(!io_ok(x)||!budget||(x.read(x.ctx,RCC+0x54u)&7u))return -1;
 x.write(x.ctx,RCC+0xE0u,x.read(x.ctx,RCC+0xE0u)|8u);x.write(x.ctx,RCC+0xE8u,x.read(x.ctx,RCC+0xE8u)|(1u<<18));(void)x.read(x.ctx,RCC+0xE8u);x.sync(x.ctx);
 if(x.read(x.ctx,UART)||x.read(x.ctx,UART+4u)||x.read(x.ctx,UART+8u)||x.read(x.ctx,UART+0x2Cu))return -1;
 x.write(x.ctx,GPIO+0x24u,(x.read(x.ctx,GPIO+0x24u)&~255u)|0x77u);
 x.write(x.ctx,GPIO+4u,x.read(x.ctx,GPIO+4u)&~0x300u);
 x.write(x.ctx,GPIO+8u,(x.read(x.ctx,GPIO+8u)&~0xF0000u)|0x50000u);
 x.write(x.ctx,GPIO+12u,(x.read(x.ctx,GPIO+12u)&~0xF0000u)|0x50000u);
 x.write(x.ctx,GPIO,(x.read(x.ctx,GPIO)&~0xF0000u)|0xA0000u);
 x.write(x.ctx,UART+0xCu,556u);x.write(x.ctx,UART+0x20u,15u);x.write(x.ctx,UART+0x18u,8u);x.write(x.ctx,UART,13u);
 for(unsigned i=0;i<budget;++i)if((x.read(x.ctx,UART+0x1Cu)&0x600000u)==0x600000u)return 1;
 x.write(x.ctx,UART,0);x.sync(x.ctx);return 0;
}
int rt_uart_rx(FHIO x,unsigned char*out,uint32_t*status){
 if(!io_ok(x)||!out||!status)return -1;uint32_t s=x.read(x.ctx,UART+0x1Cu);*status=s;
 if(RT_MUTANT!=2&&(s&15u)){x.write(x.ctx,UART+0x20u,s&15u);x.write(x.ctx,UART+0x18u,8u);return -1;}
 if(!(s&32u))return 0;*out=(unsigned char)x.read(x.ctx,UART+0x24u);return 1;
}
int rt_uart_tx(FHIO x,unsigned char byte,unsigned budget){
 if(!io_ok(x)||!budget)return -1;unsigned i;
 for(i=0;i<budget;++i)if(x.read(x.ctx,UART+0x1Cu)&128u)break;if(i==budget)return 0;
 x.write(x.ctx,UART+0x28u,byte);
 for(i=0;i<budget;++i)if(x.read(x.ctx,UART+0x1Cu)&64u)return 1;return 0;
}
int rt_uart_step(FHIO x,RTFrame*f,uint32_t*s){
 if(!f||!s||f->state)return -1;unsigned char byte;int r=rt_uart_rx(x,&byte,s);if(r<0){f->state=2;return -1;}if(!r)return 1;enum RTResult z=rt_byte(f,byte);return z==RT_READY?2:z==RT_BAD?-1:0;
}
