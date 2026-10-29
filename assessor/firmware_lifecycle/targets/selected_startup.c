#include "selected_startup.h"
#ifndef ST_MUTANT
#define ST_MUTANT 0
#endif
#define RCC 0x58024400u
#define BOX 0x30040020u
static int io_ok(struct fh_io x){return x.read&&x.write&&x.sync;}
int st_cold_h745(struct fh_io x){
 if(!io_ok(x))return 0;
 uint32_t cr=x.read(x.ctx,RCC),cf=x.read(x.ctx,RCC+0x10u);
 /* HSI64 ready, divide1, PLLs off, system clock=HSI, exact reset prescalers.
  * Do not use HSIDIVF as readiness: it resets0 until a divider transition. */
 if((cr&0x1500001Du)!=5u||(cf&63u)||x.read(x.ctx,RCC+0x18u)||x.read(x.ctx,RCC+0x1Cu)||x.read(x.ctx,RCC+0x20u)||
    (x.read(x.ctx,0xE000ED14u)&0x30000u)||x.read(x.ctx,0xE000ED94u)||(x.read(x.ctx,RCC+0xA0u)&8u))return 0;
 x.write(x.ctx,RCC+0xF4u,x.read(x.ctx,RCC+0xF4u)|2u);(void)x.read(x.ctx,RCC+0xF4u);x.sync(x.ctx);
 /* No option-byte writes. Require fixed M4 bootstrap08100000 and M7
  *08000000, BOOT pin0 and BCM4=0 provisioning independently witnessed. */
 if(ST_MUTANT!=1&&((x.read(x.ctx,0x58000704u)&1u)||!(x.read(x.ctx,0x58000700u)&1u)||
   (x.read(x.ctx,0x52002040u)&65535u)!=0x0800u||(x.read(x.ctx,0x52002048u)&65535u)!=0x0810u))return 0;
 x.write(x.ctx,RCC+0xDCu,x.read(x.ctx,RCC+0xDCu)|0xA0000000u);
 x.write(x.ctx,RCC+0xE0u,x.read(x.ctx,RCC+0xE0u)|(1u<<25));
 (void)x.read(x.ctx,RCC+0xE0u);x.sync(x.ctx);return 1;
}
int st_cold_f439(struct fh_io x){
 if(!io_ok(x))return 0;
 uint32_t cr=x.read(x.ctx,0x40023800u),cf=x.read(x.ctx,0x40023808u);
 /* HSI16, all PLLs off, reset AHB/APB divisors. No invented H7 peer. */
 return (cr&0x15000003u)==3u&&!(cf&0xFCFFu)&&!x.read(x.ctx,0xE000ED94u);
}
static int selection(const struct bs_selection*s){
 return s&&s->sequence&&s->record<2&&s->image[0].core==0&&s->image[1].core==1&&s->image[0].generation&&
 s->image[0].generation==s->image[1].generation;
}
int st_release(struct fh_io x,const struct bs_selection*s){
 if(!io_ok(x)||!selection(s)||(x.read(x.ctx,RCC+0xA0u)&8u))return 0;
 /* Caller already completed cold check and authenticated selection. Shared
  * SRAM is exclusive, uncached and inaccessible to any active peer so far. */
 if(ST_MUTANT==2)x.write(x.ctx,RCC+0xA0u,x.read(x.ctx,RCC+0xA0u)|8u);
 for(unsigned i=0;i<8;++i)x.write(x.ctx,0x30040000u+4u*i,i?0:s->image[0].generation);
 x.write(x.ctx,BOX,s->sequence);x.write(x.ctx,BOX+4u,s->image[0].generation);
 x.write(x.ctx,BOX+8u,s->image[1].base);x.write(x.ctx,BOX+12u,~s->sequence);x.write(x.ctx,BOX+16u,0);
 x.sync(x.ctx);
 if(ST_MUTANT!=2)x.write(x.ctx,RCC+0xA0u,x.read(x.ctx,RCC+0xA0u)|8u);
 x.sync(x.ctx);return 1;
}
int st_peer_matches(struct fh_io x,const struct bs_selection*s){
 if(!io_ok(x)||!selection(s))return 0;x.sync(x.ctx);
 uint32_t n=x.read(x.ctx,BOX),g=x.read(x.ctx,BOX+4u),base=x.read(x.ctx,BOX+8u),inv=x.read(x.ctx,BOX+12u);
 return n==s->sequence&&g==s->image[0].generation&&base==s->image[1].base&&inv==~n;
}
