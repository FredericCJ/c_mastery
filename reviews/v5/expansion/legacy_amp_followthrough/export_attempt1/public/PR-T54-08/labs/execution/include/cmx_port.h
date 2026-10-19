#ifndef CMASTERY_EXECUTION_PORT_H
#define CMASTERY_EXECUTION_PORT_H
#include "cmx.h"
extern cmx_kernel cmx_machine;
extern volatile uint32_t cmx_port_ticks, cmx_port_switches, cmx_port_status;
void cmx_port_init(void);
int cmx_port_task(void (*entry)(void), uint32_t *stack, unsigned words,
                  unsigned priority);
void cmx_port_start(void) __attribute__((noreturn));
void cmx_port_yield(void);
uint32_t cmx_port_lock(void);
void cmx_port_unlock(uint32_t saved);
void cmx_port_finish(unsigned code) __attribute__((noreturn));
enum cmx_result cmx_port_take(cmx_sem *, cmx_deadline);
enum cmx_result cmx_port_give(cmx_sem *);
enum cmx_result cmx_port_send(cmx_queue *, uint32_t, cmx_deadline);
enum cmx_result cmx_port_recv(cmx_queue *, uint32_t *, cmx_deadline);
void cmx_port_close(cmx_queue *);
void cmx_fp_set(uint32_t bits);
uint32_t cmx_fp_get(void);
void cmx_board_init(void);
void cmx_amp_service(void);
#endif
