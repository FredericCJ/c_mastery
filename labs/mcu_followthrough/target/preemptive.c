#include "port.h"
struct mf_mf mf_machine;
volatile uint32_t mf_ticks,mf_pend_requests,mf_switches,mf_nested;
volatile uint32_t mf_basic_frames,mf_fp_frames,mf_lazy_before,mf_lazy_after;
volatile uint32_t mf_handler_alignment,mf_stack_min,mf_fp_before_word,mf_fp_after_word;
volatile uint32_t mf_pend_latency_max,mf_isr_ticks_max,mf_fp_instruction_ticks,mf_clock_read_ticks;
volatile uint32_t mf_fp_paint_before,mf_fp_paint_after;
volatile uint32_t mf_fault_count,mf_fault_pc,mf_fault_cfsr;
void mf_port_init(void){mf_trap();}
mf_handle_id mf_port_add(void(*entry)(void),uint32_t *stack,size_t n){(void)entry;(void)stack;(void)n;return 0;}
void mf_port_start(void){mf_trap();}
uint32_t mf_lock(void){return 0;}
void mf_unlock(uint32_t p){(void)p;}
void mf_pend(void){}
enum cmx_result mf_take(cmx_sem *s,cmx_deadline d){(void)s;(void)d;return CMX_UNIMPLEMENTED;}
enum cmx_result mf_give(cmx_sem *s){(void)s;return CMX_UNIMPLEMENTED;}
unsigned mf_mask_enter(unsigned n){(void)n;return 0;}
void mf_mask_leave(unsigned n){(void)n;}
void mf_task_return(void){mf_trap();}
