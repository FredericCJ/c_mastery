#include "target.h"
#ifndef EN_TARGET_MUTANT
#define EN_TARGET_MUTANT 0
#endif
#define DEMCR UINT32_C(0xe000edfc)
#define CTRL UINT32_C(0xe0001000)
#define COUNT UINT32_C(0xe0001004)
#define RCC UINT32_C(0x40023800)
#define ETH UINT32_C(0x40028000)
#ifdef EN_TARGET_MODEL
uint32_t en_port_load(uint32_t);
void en_port_store(uint32_t,uint32_t);
void en_port_barrier(void);
#else
static uint32_t en_port_load(uint32_t a){return *(volatile uint32_t *)(uintptr_t)a;}
static void en_port_store(uint32_t a,uint32_t v){*(volatile uint32_t *)(uintptr_t)a=v;}
static void en_port_barrier(void){__asm__ volatile("dsb sy":::"memory");}
#endif
static void update(uint32_t a,uint32_t mask,uint32_t value){en_port_store(a,(en_port_load(a)&~mask)|value);}
int en_target_open(EnTarget *t){
 if(!t)return ED_INVALID;
 if(t->opened)return ED_BUSY;
 t->old_demcr=en_port_load(DEMCR);en_port_store(DEMCR,t->old_demcr|0x1000000u);en_port_barrier();
 if(!(en_port_load(DEMCR)&0x1000000u)){en_port_store(DEMCR,t->old_demcr);return ED_IO;}
 t->old_dwt=en_port_load(CTRL);
 if(t->old_dwt&0x2000000u){en_port_store(DEMCR,t->old_demcr);return ED_INVALID;}
 /* Own the counter for this session. Do not reset CYCCNT or other features. */
 en_port_store(CTRL,t->old_dwt|1u);en_port_barrier();
 if(!(en_port_load(CTRL)&1u)){en_port_store(CTRL,t->old_dwt);en_port_store(DEMCR,t->old_demcr);return ED_IO;}
 t->opened=1;return ED_OK;
}
static int settle(void *p){
 EnTarget *t=p;
 if(!t || !t->opened || !(en_port_load(DEMCR)&0x1000000u) || !(en_port_load(CTRL)&1u))return 1;
 en_port_barrier(); /* Complete the preceding peripheral write before timing. */
 uint32_t start=en_port_load(COUNT);
 /* 720 active cycles / maximum180MHz =4us, exceeding4/2.5MHz=1.6us
  * even at the slow10Mb/s selected RMII MAC clock. Reference clock must run.
  * A frozen/absent counter fails after a finite independent read budget. */
 for(unsigned i=0;i<2048;i++)if((uint32_t)(en_port_load(COUNT)-start)>=(EN_TARGET_MUTANT==1?1u:720u))return 0;
 return 1;
}
static int read_register(void *p,unsigned offset,uint32_t *v){
 EnTarget *t=p;if(!t || !t->opened || !v || (offset&3u) || offset>0x101cu)return 1;
 *v=en_port_load(ETH+offset);return 0;
}
static int write_register(void *p,unsigned offset,uint32_t v){
 EnTarget *t=p;if(!t || !t->opened || (offset&3u) || offset>0x101cu)return 1;
 en_port_store(ETH+offset,v);return 0;
}
static void barrier(void *p){(void)p;en_port_barrier();}
EnBus en_target_bus(EnTarget *t){EnBus b={t,read_register,write_register,barrier,settle};return b;}
int en_target_cold(EnTarget *t){
 if(!t || !t->opened)return ED_INVALID;
 /* A cold preparation operation, never a live-driver recovery shortcut. */
 if(en_port_load(RCC+0x30)&0x0e000000u)return ED_BUSY;
 update(RCC+0x30,0,0x47u);(void)en_port_load(RCC+0x30);en_port_barrier();
 update(RCC+0x44,0,0x4000u);(void)en_port_load(RCC+0x44);en_port_barrier();
 update(RCC+0x10,0,0x02000000u);en_port_barrier();
 update(0x40013804u,0,EN_TARGET_MUTANT==2?0:0x800000u);
 static const unsigned ports[]={0,0,0,1,2,2,2,6,6};
 static const unsigned pins[]={1,2,7,13,1,4,5,11,13};
 for(unsigned i=0;i<9;i++){
  uint32_t base=0x40020000u+ports[i]*0x400u;unsigned n=pins[i],shift=n*2;
  update(base+4,1u<<n,0);update(base+8,3u<<shift,2u<<shift);update(base+12,3u<<shift,0);
  unsigned a=n<8?0x20:0x24,s=(n%8)*4;
  update(base+a,15u<<s,(EN_TARGET_MUTANT==3?10u:11u)<<s);
  update(base,3u<<shift,2u<<shift);
 }
 update(RCC+0x30,0,0x0e000000u);(void)en_port_load(RCC+0x30);en_port_barrier();
 update(RCC+0x10,0x02000000u,0);en_port_barrier();return ED_OK;
}
int en_target_close(EnTarget *t){
 if(!t || !t->opened)return ED_INVALID;
 en_port_store(CTRL,t->old_dwt);en_port_barrier();en_port_store(DEMCR,t->old_demcr);en_port_barrier();t->opened=0;return ED_OK;
}
