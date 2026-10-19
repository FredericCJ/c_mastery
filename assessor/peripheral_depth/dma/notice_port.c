/* Original H745 HSEM1 mailbox; privileged dedicated-board builds only. */
#include "dma.h"
#include "h745_cache.h"
#ifndef PD_PEER_M7
#define PD_PEER_M7 1
#endif
#ifndef PD_PORT_MUTANT
#define PD_PORT_MUTANT 0
#endif
#ifdef PD_PORT_MODEL
uint32_t pd_port_read(uint32_t);
#define PD_READ(a) pd_port_read(a)
void pd_port_write(uint32_t, uint32_t);
#define PD_WRITE(a, v) pd_port_write(a, v)
uint32_t pd_port_mask(void);
void pd_port_restore(uint32_t);
void pd_port_barrier(void);
PdNotice *pd_port_memory(void);
#else
_Static_assert(sizeof(uintptr_t) == 4, "Selected32-bit target");
#define WORD(a) (*(volatile uint32_t *)(uintptr_t)(a))

#define PD_READ(a) WORD(a)
#define PD_WRITE(a, v) (WORD(a) = (v))
static uint32_t pd_port_mask(void) {
  uint32_t v;
  __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(v)::"memory");
  return v;
}
static void pd_port_restore(uint32_t v) {
  __asm__ volatile("msr primask, %0" ::"r"(v) : "memory");
}
static void pd_port_barrier(void) { __asm__ volatile("dsb sy" ::: "memory"); }
static PdNotice *pd_port_memory(void) {
  return (PdNotice *)(uintptr_t)0x30040020u;
}
#endif
static int operate(unsigned op, uint32_t epoch, uint32_t seq,
                   const PdNotice *message, PdNotice *out, int boot) {
  if (!epoch)
    return PD_INVALID;
  uint32_t mask = pd_port_mask(), id = PD_PEER_M7 ? 3u : 1u;
  if (PD_READ(0x58026484u) != (0x80000000u | (id << 8))) {
    pd_port_restore(mask);
    return PD_BUSY;
  }
  pd_port_barrier();
  if (PD_PORT_MUTANT != 1)
    (void)fw_h745_cache(0x30040020u, 32, H7_INVALIDATE);
  int r;
  if (boot) {
    PdNotice initial = {epoch, 0, 0, 0, 0, 0, 0, 0};
    *pd_port_memory() = initial;
    r = PD_OK;
  } else
    r = pd_notice(pd_port_memory(), op, epoch, seq, message, out);
  if (r == PD_OK && op == PD_NOTICE_TAKE && !boot && PD_PORT_MUTANT != 2)
    (void)fw_h745_cache(out->destination, out->extent, H7_INVALIDATE);
  if (PD_PORT_MUTANT != 3)
    (void)fw_h745_cache(0x30040020u, 32, FW_CLEAN);
  pd_port_barrier();
  PD_WRITE(0x58026404u, id << 8);
  pd_port_barrier();
  pd_port_restore(mask);
  return r;
}
int pd_h745_notice(unsigned op, uint32_t epoch, uint32_t seq,
                   const PdNotice *message, PdNotice *out) {
  return operate(op, epoch, seq, message, out, 0);
}
/* Only trusted cold start: peer held in reset, DMA idle, no outstanding leases.
   Never use this operation as timeout recovery. */
int pd_h745_notice_cold_start(uint32_t epoch) {
  return operate(0, epoch, 0, 0, 0, 1);
}
