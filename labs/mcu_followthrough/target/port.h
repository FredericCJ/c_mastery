#ifndef MF_PORT_H
#define MF_PORT_H
#include "mcu.h"
#include "target.h"
#define MF_REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
extern struct mf_mf mf_machine;
extern volatile uint32_t mf_ticks, mf_pend_requests, mf_switches, mf_nested;
extern volatile uint32_t mf_basic_frames, mf_fp_frames, mf_lazy_before,
    mf_lazy_after;
extern volatile uint32_t mf_handler_alignment, mf_stack_min;
extern volatile uint32_t mf_fp_before_word, mf_fp_after_word;
extern volatile uint32_t mf_pend_latency_max, mf_isr_ticks_max,
    mf_fp_instruction_ticks, mf_clock_read_ticks;
extern volatile uint32_t mf_fp_paint_before, mf_fp_paint_after;
extern volatile uint32_t mf_fault_count, mf_fault_pc, mf_fault_cfsr;
void mf_port_init(void);
mf_handle_id mf_port_add(void (*)(void), uint32_t *, size_t);
void mf_port_start(void);
uint32_t mf_lock(void);
void mf_unlock(uint32_t);
void mf_pend(void);
enum cmx_result mf_take(cmx_sem *, cmx_deadline);
enum cmx_result mf_give(cmx_sem *);
unsigned mf_mask_enter(unsigned);
void mf_mask_leave(unsigned);
void mf_async_probe(struct mf_probe *);
void mf_tick_hook(void);
void mf_task_return(void);
#endif
