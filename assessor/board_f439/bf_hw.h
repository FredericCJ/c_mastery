#ifndef BF_HW_H
#define BF_HW_H
/* NUCLEO-F439ZI board harness: register map, boot record and core timing.
 *
 * Author physical-observation firmware, not a learner artifact. Register
 * addresses follow RM0090 Rev21 and the Armv7-M system control space; the
 * same addresses are already used by the reused assessor adapters. Every
 * value printed by the harness is a raw register read: interpretation lives
 * in reviews/v5/expansion/polish_revision/physical/PLAN.md.
 */
#include <stdint.h>

#define BF_HSI_HZ UINT32_C(16000000)

#define BF_RCC_CR UINT32_C(0x40023800)
#define BF_RCC_PLLCFGR UINT32_C(0x40023804)
#define BF_RCC_CFGR UINT32_C(0x40023808)
#define BF_RCC_APB1ENR UINT32_C(0x40023840)
#define BF_RCC_APB2ENR UINT32_C(0x40023844)
#define BF_RCC_CSR UINT32_C(0x40023874)
#define BF_FLASH_ACR UINT32_C(0x40023C00)
#define BF_PWR_CR UINT32_C(0x40007000)
#define BF_IWDG_SR UINT32_C(0x4000300C)
#define BF_USART3_SR UINT32_C(0x40004800)
#define BF_USART3_DR UINT32_C(0x40004804)
#define BF_SYSCFG_MEMRMP UINT32_C(0x40013800)
#define BF_SYSCFG_EXTICR3 UINT32_C(0x40013810)
#define BF_EXTI_IMR UINT32_C(0x40013C00)
#define BF_EXTI_RTSR UINT32_C(0x40013C08)
#define BF_EXTI_FTSR UINT32_C(0x40013C0C)
#define BF_EXTI_PR UINT32_C(0x40013C14)
#define BF_UID_BASE UINT32_C(0x1FFF7A10)
#define BF_NVIC_ISER0 UINT32_C(0xE000E100)
#define BF_SYST_CSR UINT32_C(0xE000E010)
#define BF_SYST_RVR UINT32_C(0xE000E014)
#define BF_SYST_CVR UINT32_C(0xE000E018)
#define BF_SCB_VTOR UINT32_C(0xE000ED08)
#define BF_SCB_AIRCR UINT32_C(0xE000ED0C)
#define BF_SCB_SCR UINT32_C(0xE000ED10)
#define BF_SCB_CFSR UINT32_C(0xE000ED28)
#define BF_SCB_HFSR UINT32_C(0xE000ED2C)
#define BF_DEMCR UINT32_C(0xE000EDFC)
#define BF_DWT_CTRL UINT32_C(0xE0001000)
#define BF_DWT_CYCCNT UINT32_C(0xE0001004)
#define BF_DBGMCU_CR UINT32_C(0xE0042004)
#define BF_DBGMCU_APB1_FZ UINT32_C(0xE0042008)

#define BF_RCC_CSR_RMVF (UINT32_C(1) << 24)
#define BF_SCR_SLEEPDEEP (UINT32_C(1) << 2)
#define BF_DBGMCU_SLEEP UINT32_C(0x1)
#define BF_DBGMCU_STOP UINT32_C(0x2)
#define BF_EXTI9 (UINT32_C(1) << 9)
#define BF_IRQ_EXTI9_5 23u
#define BF_HISTORY 4u

static inline uint32_t bf_read(uint32_t address)
{
    return *(volatile const uint32_t *)(uintptr_t)address;
}

static inline void bf_write(uint32_t address, uint32_t value)
{
    *(volatile uint32_t *)(uintptr_t)address = value;
}

/* Facts captured on entry to bf_main, before anything else runs. */
struct bf_boot
{
    uint32_t csr; /* RCC_CSR before RMVF clears it */
    uint32_t boots; /* boots counted by the retained record */
    uint32_t retained_valid; /* 1: record survived the previous boot */
    uint32_t history[BF_HISTORY]; /* earlier RCC_CSR values, newest first */
    uint32_t fault_cfsr; /* recorded by a fault in the last boot */
    uint32_t fault_hfsr;
    uint32_t fault_pc;
    uint32_t unexpected_ipsr; /* an unexpected exception, if any */
    uint32_t lma_ok; /* .data equals its flash load image */
    uint32_t bss_ok; /* .bss reads as zero */
    uint32_t vectors_ok; /* flash words 0/1 equal stack/reset */
    uint32_t dbgmcu_cr; /* DBGMCU_CR at entry (reset by power-on only) */
};

/* Record RCC_CSR in the retained history for `mode`, then set RMVF so the
 * next boot sees only the reset that caused it. Call first in bf_main. */
void bf_boot_capture(uint32_t mode, struct bf_boot *boot);

void bf_cycles_enable(void);
uint32_t bf_cycles(void);
/* Busy-wait; `cycles` must stay below 2^31 so the wrap test is valid. */
void bf_delay_cycles(uint32_t cycles);

/* SysTick with TICKINT. core_clock: 1 = processor clock, 0 = HCLK/8. */
void bf_systick_start(uint32_t reload, uint32_t core_clock);
uint32_t bf_ticks(void);
uint32_t bf_tick_cycles(void);

_Noreturn void bf_software_reset(void);

/* EXTI line 9 on PD9 (USART3 RX, driven by the ST-LINK VCP), falling edge,
 * interrupt mode, NVIC IRQ 23 enabled. */
void bf_exti9_wake_init(void);
uint32_t bf_exti9_count(void);
/* Stop entry with the same register steps as cs_stop_enter but without its
 * "no debugger in low power" precondition (changed-condition image only). */
void bf_stop_raw(void);

/* Exception handlers named by startup.S. */
void bf_systick_handler(void);
void bf_exti9_5_handler(void);
_Noreturn void bf_fault_record(const uint32_t *frame);
_Noreturn void bf_unexpected_record(uint32_t ipsr);
#endif
