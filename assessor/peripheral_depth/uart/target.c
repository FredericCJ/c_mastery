#include "uart.h"
#include <stddef.h>
typedef struct {
  volatile uint32_t sr, dr, brr, cr1, cr2, cr3, gtpr;
} Registers;
_Static_assert(offsetof(Registers, dr) == 4 && offsetof(Registers, cr3) == 20,
               "USART layout");
#ifndef PU_TARGET_MODEL
_Static_assert(sizeof(uintptr_t) == 4, "F439 pointer width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define USART ((Registers *)(uintptr_t)0x40004800u)
#else
volatile uint32_t *pu_test_register(uint32_t);
Registers *pu_test_usart(void);
#define REG(a) (*pu_test_register(a))
#define USART pu_test_usart()
#endif
#ifndef PU_TARGET_MUTANT
#define PU_TARGET_MUTANT 0
#endif
#define STREAM 0x40026058u
static uint32_t read_reg(void *context, unsigned r) {
  (void)context;
  switch (r) {
  case PU_SOURCE: {
    unsigned sws = (REG(0x40023808u) >> 2) & 3u;
    return sws < 3 && ((REG(0x40023800u) >> (sws == 0   ? 1
                                             : sws == 1 ? 17
                                                        : 25)) &
                       1u);
  }
  case PU_GATE:
    return (REG(0x40023840u) >> 18) & 1u;
  case PU_SR:
    return USART->sr;
  case PU_DR:
    return USART->dr;
  case PU_CR1:
    return USART->cr1;
  case PU_DMA_GATE:
    return (REG(0x40023830u) >> 21) & 1u;
  case PU_DMA_BUSY:
    return REG(STREAM) & 1u;
  case PU_DMA_COUNT:
    return REG(STREAM + 4);
  case PU_DMA_FLAGS: {
    uint32_t flags = REG(0x40026000u);
    return ((flags >> 27) & 1u) | ((flags & 0x03400000u) ? 2u : 0u);
  }
  default:
    return 0;
  }
}
static void write_reg(void *context, unsigned r, uintptr_t v) {
  (void)context;
  switch (r) {
  case PU_GATE:
    REG(0x40023840u) |= 1u << 18;
    break;
  case PU_RESET:
    if (v)
      REG(0x40023820u) |= 1u << 18;
    else
      REG(0x40023820u) &= ~(1u << 18);
    break;
  case PU_SR:
    USART->sr = (uint32_t)v;
    break;
  case PU_DR:
    USART->dr = (uint32_t)v;
    break;
  case PU_BRR:
    USART->brr = (uint32_t)v;
    break;
  case PU_CR1:
    USART->cr1 = (uint32_t)v;
    break;
  case PU_CR2:
    USART->cr2 = (uint32_t)v;
    break;
  case PU_CR3:
    USART->cr3 = (uint32_t)v;
    break;
  case PU_DMA_GATE:
    REG(0x40023830u) |= 1u << 21;
    break;
  case PU_DMA_ADDRESS:
    REG(STREAM + 12) = (uint32_t)v;
    break;
  case PU_DMA_COUNT:
    REG(STREAM + 4) = (uint32_t)v;
    break;
  case PU_DMA_CLEAR:
    REG(0x40026008u) = 0x0f400000u;
    break;
  case PU_DMA_CONTROL:
    if (!v)
      REG(STREAM) &= ~1u;
    else {
      REG(STREAM + 8) = 0x40004804u;
      REG(STREAM + 20) = 0;
      REG(STREAM) = ((PU_TARGET_MUTANT == 1 ? 5u : 4u) << 25) | (1u << 10) |
                    (1u << 6) | 1u;
    }
    break;
  default:
    break;
  }
}
PuIo pu_f439_io(void) {
  PuIo io = {0, read_reg, write_reg};
  return io;
}
void pu_f439_pins(void) {
  /* Exclusive GPIOD configuration; PD8 TX, PD9 RX, AF7, push-pull, low speed.
     Board ST-LINK VCP routing must be verified before connecting. */
  REG(0x40023830u) |= 1u << 3;
  (void)REG(0x40023830u);
  REG(0x40020c00u) = (REG(0x40020c00u) & ~(15u << 16)) | (10u << 16);
  REG(0x40020c04u) &= ~(3u << 8);
  REG(0x40020c08u) &= ~(15u << 16);
  REG(0x40020c0cu) = (REG(0x40020c0cu) & ~(15u << 16)) | (1u << 18);
  REG(0x40020c24u) =
      (REG(0x40020c24u) & ~255u) | (PU_TARGET_MUTANT == 2 ? 0x66u : 0x77u);
}
void pu_f439_rts_init(void) {
  /* Dedicated PD12 software-ready line, active low. No automatic RTSE. */
  REG(0x40023830u) |= 1u << 3;
  (void)REG(0x40023830u);
  REG(0x40020c18u) = 1u << 12;
  REG(0x40020c04u) &= ~(1u << 12);
  REG(0x40020c08u) &= ~(3u << 24);
  REG(0x40020c0cu) &= ~(3u << 24);
  REG(0x40020c00u) = (REG(0x40020c00u) & ~(3u << 24)) | (1u << 24);
}
int pu_f439_rts(const PuRx *q) {
  unsigned ready;
  int result = pu_peer_ready(q, &ready);
  if (result)
    return result;
  REG(0x40020c18u) = 1u << (ready ? 28 : 12);
  return PU_OK;
}
int pu_f439_dma_submit(PuDriver *d, uint32_t gen, uint32_t token,
                       const uint8_t *data, unsigned count, uint32_t now,
                       uint32_t budget) {
  uintptr_t p = (uintptr_t)data;
  if (!count || count > 8 || p < 0x20000000u || p >= 0x20030000u ||
      count > 0x20030000u - p)
    return PU_INVALID;
  return pu_submit(d, pu_f439_io(), gen, token, PU_DMA, data, count, now,
                   budget);
}
