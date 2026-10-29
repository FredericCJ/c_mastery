#include "uart_depth.h"
void ua_note(const char *);
void ua_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do {if (!(x)) {ua_note("UA_MAP_ASSERT: " #x);return 41;}}while(0)
struct ep_registers {volatile uint32_t sr,dr,brr,cr1,cr2,cr3,gtpr;};
static struct ep_registers u;
static uint32_t addresses[32],values[32];static unsigned used;
volatile uint32_t *pu_test_register(uint32_t a) {
  for(unsigned i=0;i<used;i++)if(addresses[i]==a)return values+i;
  if(used==32)return 0;
  addresses[used]=a;return values+used++;
}
struct ep_registers *pu_test_usart(void){return &u;}
#define R(a) (*pu_test_register(a))
#define STREAM 0x40026058u
int ua_mapped(void) {
  struct pu_driver d={0};struct pu_clock c={42000000,1,1,1,1,0};struct pu_io io=pu_f439_io();
  R(0x40023800u)=2;R(0x40023808u)=0;
  CHECK(pu_open(&d,io,&c,115200,0,1)==PU_OK);
  R(0x40020c24u)=0xffffffffu;pu_f439_pins();
  CHECK(R(0x40020c24u)==0xffffff77u && (R(0x40020c00u)&0xf0000u)==0xa0000u);
  CHECK(pu_f439_dma_submit(&d,1,1,(const uint8_t *)(uintptr_t)0x10000000u,8,0,10)==PU_INVALID);
  CHECK(!d.active && R(STREAM)==0 && u.cr3==0);
  CHECK(pu_f439_dma_submit(&d,1,1,(const uint8_t *)(uintptr_t)0x20030000u,1,0,10)==PU_INVALID);
  CHECK(pu_f439_dma_submit(&d,1,1,(const uint8_t *)(uintptr_t)0x2002ffffu,2,0,10)==PU_INVALID);
  CHECK(pu_f439_dma_submit(&d,1,1,(const uint8_t *)(uintptr_t)0x20000000u,8,0,10)==PU_OK);
  CHECK(R(STREAM)==0x08000441u && R(STREAM+8)==0x40004804u);
  CHECK(R(STREAM+12)==0x20000000u && R(STREAM+4)==8 && u.cr3==128);
  /* A DMA interrupt flag alone does not acknowledge engine quiescence. */
  R(0x40026000u)=1u<<27;R(STREAM+4)=0;u.sr=PU_TXE;
  CHECK(pu_step(&d,io,1,1)==PU_BUSY && d.memory_owned && d.line_owned);
  R(STREAM)&=~1u;
  CHECK(pu_step(&d,io,1,2)==PU_BUSY && !d.memory_owned && d.line_owned && d.active);
  u.sr=PU_TXE|PU_TC;
  CHECK(pu_step(&d,io,1,3)==PU_OK && !d.active && !d.line_owned);
  CHECK(pu_f439_dma_submit(&d,1,2,(const uint8_t *)(uintptr_t)0x20000000u,8,4,10)==PU_OK);
  CHECK(pu_cancel(&d,io,2)==PU_OK && d.memory_owned);
  /* The RAM register model deliberately delays acknowledgment of the stop. */
  R(STREAM)|=1u;u.sr=PU_TXE;
  CHECK(pu_step(&d,io,2,5)==PU_BUSY && d.memory_owned && d.line_owned);
  R(STREAM)&=~1u;
  CHECK(pu_step(&d,io,2,6)==PU_BUSY && !d.memory_owned && d.line_owned);
  u.sr=PU_TC;
  CHECK(pu_step(&d,io,2,7)==PU_CANCELLED && !d.active);
  ua_values("mapped",u.brr,R(0x40020c24u),R(STREAM+12),d.token);return 0;
}
