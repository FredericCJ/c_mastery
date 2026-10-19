#ifndef CMASTERY_SMP_DEPTH_PERCPU_H
#define CMASTERY_SMP_DEPTH_PERCPU_H
#include <stddef.h>
#include <stdint.h>
enum {SP_OK,SP_INVALID,SP_RANGE,SP_ALIAS,SP_SMALL};
enum {SP_KERNEL,SP_IDLE,SP_IRQ,SP_EMERGENCY,SP_GDT,SP_TSS,SP_IDT,SP_TASK1,SP_FAULT,SP_PARTS};
typedef struct {uint64_t low,high;unsigned cpu,kind;} SpSpan;
typedef struct {unsigned cpus;SpSpan spans[4*SP_PARTS];uint64_t end;} SpLayout;
/* Explicit storage/ABI model, not proof that numeric addresses are mapped. */
int sp_layout(uint64_t base,uint64_t available,unsigned cpus,uint64_t stack_bytes,unsigned stack_alignment,SpLayout *out);
int sp_validate(const SpSpan *,size_t count);
/* Fixed declared call chain plus one entry frame on one selected stack.
 * Different IST stacks have separate budgets; do not sum unrelated stacks. */
int sp_budget(const uint64_t *frames,size_t count,uint64_t hardware,uint64_t saved,unsigned alignment,uint64_t extended,uint64_t *out);
#endif
