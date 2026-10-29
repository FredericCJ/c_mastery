/* Original privileged H745 two-image specimen. Cross-build is not board execution. */
#include "target.h"
#include "buffers.h"
#ifdef AF_PIPELINE
#include "cmx_port.h"
#endif
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#if AF_QEMU
static struct af_shared simulated;
#define SH (&simulated)
#else
#define SH ((struct af_shared *)(uintptr_t)AF_SHARED)
#endif
static const unsigned local=AF_M7?0u:1u;
static const struct af_door door={AF_M7?7u:4u,AF_M7?8u:9u,AF_M7?9u:8u};
static uint32_t read_reg(void*c,uint32_t a){(void)c;return REG(a);}
static void write_reg(void*c,uint32_t a,uint32_t v){(void)c;REG(a)=v;}
static void fence(void*c){(void)c;__asm__ volatile("dsb sy":::"memory");}
static const struct af_registers io={0,read_reg,write_reg,fence};
static _Noreturn void fail(uint32_t code){atomic_store(&SH->fault[local],code);
#if AF_QEMU
cmx_port_finish(code);
#else
__asm__ volatile("cpsid i":::"memory");for(;;)__asm__ volatile("wfi");
#endif
}
void af_fault(void){fail(0xf0);}
static uint32_t now(void){return REG(0xe0001004u);}
static void ring(void){
#if !AF_QEMU
uint32_t mask;__asm__ volatile("mrs %0,primask\n\tcpsid i":"=r"(mask)::"memory");(void)af_ring_door(io,door);__asm__ volatile("msr primask,%0"::"r"(mask):"memory");
#endif
}
void af_hsem(void){(void)af_clear_door(io,door);atomic_fetch_add_explicit(&SH->irqs[local],1,memory_order_relaxed);}
void af_systick(void){atomic_fetch_add_explicit(&SH->ticks[local],1,memory_order_relaxed);}
static void machine(void){
 REG(0xe000ed08u)=(uint32_t)(uintptr_t)&af_vectors;
 REG(0xe000edfcU)|=1u<<24;REG(0xe0001000u)|=1;REG(0xe0001004u)=0;
 /* Private SysTick measures a declared tick count, not microseconds. Board
    clock identity must be captured separately; the image does not retune PLLs. */
 REG(0xe000e014u)=63999;REG(0xe000e018u)=0;REG(0xe000e010u)=7;
 #if !AF_QEMU
 unsigned irq=AF_M7?125u:126u;REG(0xe000e280u+4*(irq/32))=1u<<(irq%32);REG(0xe000e100u+4*(irq/32))=1u<<(irq%32);
 (void)af_clear_door(io,door);(void)af_mask_door(io,door,1);
 #endif
}
#if AF_M7
static void memory_policy(void){
#if !AF_QEMU
 /* Cold only: dirty live cache state must never be discarded here. */
 if(REG(0xe000ed14u)&(1u<<16))fail(0xc0);
 REG(0x580244e0u)|=1u<<25;REG(0x580244dcu)|=0xe0000000u;fence(0);
 REG(0xe000ed94u)=0;REG(0xe000ed98u)=7;REG(0xe000ed9cu)=AF_SHARED;
 REG(0xe000eda0u)=(1u<<28)|(3u<<24)|(1u<<19)|(1u<<18)|(12u<<1)|1u;
 REG(0xe000ed94u)=5;fence(0);__asm__ volatile("isb sy":::"memory");
#if AF_CACHED
 REG(0xe000ed84u)=0;fence(0);uint32_t config=REG(0xe000ed80u);
 if((config&7)!=1||((config>>3)&1023)!=3||((config>>13)&32767)>511)fail(0xc1);
 for(unsigned set=0;set<=((config>>13)&32767);set++)for(unsigned way=0;way<4;way++)REG(0xe000ef60u)=(way<<30)|(set<<5);
 fence(0);REG(0xe000ed14u)|=1u<<16;fence(0);__asm__ volatile("isb sy":::"memory");
#endif
#endif
}
static struct af_boot_owner supervisor;
static struct af_buffers buffers;
static struct pd_port port;
static uint32_t dma_started;
static struct af_receipt receipt={.session=1};
static void publish(uint32_t previous){
 if(af_boot_begin(&supervisor,previous,now(),100000000u,1,1)!=AF_OK)fail(0xb0);
 for(unsigned i=0;i<3;i++)if(af_boot_stage(&supervisor,&SH->boot,i,2,AF_LAYOUT)!=AF_OK)fail(0xb1);
 af_ring_init(&SH->request,0);af_ring_init(&SH->response,0);atomic_store(&SH->boot.parked,0);
 if(af_boot_stage(&supervisor,&SH->boot,3,2,AF_LAYOUT)!=AF_OK)fail(0xb2);
}
static void dma_step(void){
 struct pd_io dev=pd_target_io(&port);
 if(buffers.transfer.phase){int r=af_complete(&buffers,dev);if(r==AF_OK){unsigned i=buffers.active;if(af_consume(&buffers,i)!=AF_OK)fail(0xd1);volatile uint32_t *dst=(volatile uint32_t *)(uintptr_t)(0x30042040u+128*i);for(unsigned k=0;k<16;k++)if(dst[k]!=k+buffers.slot[i].ticket.generation)fail(0xd2);if(af_release(&buffers,i)!=AF_OK)fail(0xd3);}else {unsigned expired;if(af_elapsed(dma_started,now(),100000000,&expired)!=AF_OK||expired){(void)af_timeout(&buffers,dev);fail(0xd6);}}return;}
 unsigned i=buffers.transfer.last_token&1u;if(af_fill(&buffers,i)!=AF_OK)fail(0xd4);
 uint32_t base=0x30042000u+128*i;volatile uint32_t *src=(volatile uint32_t *)(uintptr_t)base;for(unsigned k=0;k<16;k++)src[k]=k+buffers.transfer.last_token+1;
 struct pd_extent e={base,base+64,64,64,64,4};dma_started=now();if(af_submit(&buffers,dev,i,&e)!=AF_OK)fail(0xd5);
}
#ifndef AF_PIPELINE
static void primary(void){
 memory_policy();atomic_init(&SH->boot.ready,0);atomic_init(&SH->boot.version,0);atomic_init(&SH->boot.layout,0);atomic_init(&SH->boot.epoch,0);atomic_init(&SH->boot.ack,0);atomic_init(&SH->boot.parked,0);
 for(unsigned i=0;i<2;i++){atomic_init(&SH->progress[i],0);atomic_init(&SH->fault[i],0);atomic_init(&SH->irqs[i],0);atomic_init(&SH->ticks[i],0);}atomic_init(&SH->samples,0);atomic_init(&SH->dropped,0);atomic_init(&SH->done,0);
 struct xd_fence cold={0,1,1,1};if(af_buffers_init(&buffers,&cold)!=AF_OK)fail(0xd0);publish(0);machine();REG(0x580244a0u)|=8;fence(0);__asm__ volatile("cpsie i":::"memory");
 uint32_t next=1,completed=0,last_tick=0,epoch=supervisor.next_epoch;
 for(;;){
  int state=af_boot_status(&supervisor,&SH->boot,now());if(state==AF_STALE||state==AF_INVALID)fail(0xb3);if(state!=AF_OK)continue;
  /* Once boot completes its finite deadline no longer times the service. */
  supervisor.start=now();
  uint32_t tick=atomic_load(&SH->ticks[0]);if(tick!=last_tick){last_tick=tick;atomic_fetch_add(&SH->samples,1);struct af_message m={.epoch=epoch,.sequence=next,.length=4};for(unsigned k=0;k<4;k++)m.bytes[k]=(unsigned char)(next>>(8*k));int r=af_batch(&SH->request,epoch,&m,1);if(r==AF_OK){next++;ring();}else if(r==AF_FULL)atomic_fetch_add(&SH->dropped,1);else fail(0xa0);}
  for(unsigned budget=0;budget<4;budget++){unsigned char bytes[32];size_t len;uint32_t seq;int r=af_receive(&SH->response,epoch,bytes,sizeof bytes,&seq,&len);if(r==AF_EMPTY)break;if(r!=AF_OK||len!=4||seq!=completed+1)fail(0xa1);uint32_t value=0;for(unsigned k=0;k<4;k++)value|=(uint32_t)bytes[k]<<(8*k);if(value!=seq+1)fail(0xa2);int32_t total;if(af_effect(&receipt,1,seq,1,&total)!=AF_OK)fail(0xa3);completed=seq;atomic_store(&SH->done,completed);}
  dma_step();atomic_fetch_add(&SH->progress[0],1);
  /* Controlled cooperative restart only at a drained boundary; an unexpected
     peer reset/timeout fails closed and leaves DMA/storage ownership intact. */
  if(epoch==1&&completed==128&&next==129){
   atomic_store_explicit(&SH->boot.ready,0,memory_order_release);uint32_t start=now();
   while(atomic_load_explicit(&SH->boot.parked,memory_order_acquire)!=epoch){unsigned expired;if(af_elapsed(start,now(),100000000,&expired)!=AF_OK||expired)fail(0xb4);}
   while(buffers.transfer.phase){unsigned expired;if(af_elapsed(start,now(),100000000,&expired)!=AF_OK||expired)fail(0xb5);dma_step();}
   if(af_boot_close(&supervisor,&SH->boot,1,1)!=AF_OK)fail(0xb6);
   publish(epoch);epoch=supervisor.next_epoch;ring();
  }
 }
}
#endif
#else
#ifndef AF_PIPELINE
static void secondary(void){
 machine();__asm__ volatile("cpsie i":::"memory");uint32_t epoch=0,start=now();
 uint32_t previous=atomic_load_explicit(&SH->boot.ready,memory_order_acquire);
 if(previous&&atomic_load_explicit(&SH->boot.ack,memory_order_acquire)==previous)fail(0xb9);
 for(;;){uint32_t ready=atomic_load_explicit(&SH->boot.ready,memory_order_acquire);
  if(!ready){if(epoch)atomic_store_explicit(&SH->boot.parked,epoch,memory_order_release);unsigned expired;if(af_elapsed(start,now(),100000000,&expired)!=AF_OK||expired)fail(0xb7);continue;}
  if(ready!=epoch){if(af_boot_ack(&SH->boot,2,AF_LAYOUT,ready)!=AF_OK)fail(0xb8);epoch=ready;}
  start=now();
  for(unsigned budget=0;budget<4;budget++){unsigned char bytes[32];size_t len;uint32_t seq;/* Retain input when response capacity is exhausted. */
   if(af_queued(&SH->response)==AF_CAPACITY)break;
   int r=af_receive(&SH->request,epoch,bytes,sizeof bytes,&seq,&len);if(r==AF_EMPTY)break;if(r!=AF_OK||len!=4)fail(0xa4);
   uint32_t value=0;for(unsigned k=0;k<4;k++)value|=(uint32_t)bytes[k]<<(8*k);value++;
   struct af_message m={.epoch=epoch,.sequence=seq,.length=4};for(unsigned k=0;k<4;k++)m.bytes[k]=(unsigned char)(value>>(8*k));if(af_batch(&SH->response,epoch,&m,1)!=AF_OK)fail(0xa5);ring();
  }
  atomic_fetch_add(&SH->progress[1],1);
 }
}
#endif
#endif
#ifdef AF_PIPELINE
#include "pipeline.inc"
#endif
void af_entry(void){
#if AF_M7
 primary();
#else
 secondary();
#endif
}
