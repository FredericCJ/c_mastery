#include "bf_hw.h"

#include <stddef.h>

#define BF_RETAINED_MAGIC UINT32_C(0x42463130)
#define BF_RETAINED_SALT UINT32_C(0xA5A5A5A5)
#define BF_AIRCR_SYSRESETREQ UINT32_C(0x05FA0004)
#define BF_DEMCR_TRCENA (UINT32_C(1) << 24)
#define BF_SYST_ENABLE_TICKINT UINT32_C(0x3)
#define BF_SYST_CLKSOURCE UINT32_C(0x4)
#define BF_RCC_APB1ENR_PWREN (UINT32_C(1) << 28)
#define BF_RCC_APB2ENR_SYSCFGEN (UINT32_C(1) << 14)
#define BF_EXTICR3_EXTI9_MASK (UINT32_C(0xF) << 4)
#define BF_EXTICR3_EXTI9_PORT_D (UINT32_C(0x3) << 4)
#define BF_PWR_CR_STOP_BITS UINT32_C(0xE03)

/* Survives reset in .noinit; validity is corruption detection only. */
struct bf_retained
{
    uint32_t magic;
    uint32_t mode;
    uint32_t boots;
    uint32_t history[BF_HISTORY];
    uint32_t fault_cfsr;
    uint32_t fault_hfsr;
    uint32_t fault_pc;
    uint32_t unexpected_ipsr;
    uint32_t check;
};

static volatile struct bf_retained retained __attribute__((section(".noinit")));
/* Known initialised object: makes the .data load-image check non-vacuous. */
static volatile uint32_t data_canary = UINT32_C(0x5AA5C33C);
static volatile uint32_t ticks;
static volatile uint32_t tick_cycles;
static volatile uint32_t exti9_events;

extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];
extern const uint32_t _sidata[];
extern uint32_t _estack[];
extern void bf_reset(void);

static uint32_t retained_check(void)
{
    uint32_t value = BF_RETAINED_SALT ^ retained.magic ^ retained.mode;
    value ^= retained.boots ^ retained.fault_cfsr ^ retained.fault_hfsr;
    value ^= retained.fault_pc ^ retained.unexpected_ipsr;
    for (uint32_t i = 0u; i < BF_HISTORY; ++i)
    {
        value ^= retained.history[i] + i;
    }
    return value;
}

static void retained_reset(uint32_t mode)
{
    retained.magic = BF_RETAINED_MAGIC;
    retained.mode = mode;
    retained.boots = 0u;
    for (uint32_t i = 0u; i < BF_HISTORY; ++i)
    {
        retained.history[i] = 0u;
    }
    retained.fault_cfsr = 0u;
    retained.fault_hfsr = 0u;
    retained.fault_pc = 0u;
    retained.unexpected_ipsr = 0u;
}

/* Startup postconditions, checked before any .data object is written. */
static void check_startup(struct bf_boot *boot)
{
    boot->lma_ok = data_canary == UINT32_C(0x5AA5C33C) ? 1u : 0u;
    const volatile uint32_t *load = _sidata;
    for (const volatile uint32_t *run = _sdata; run < _edata; ++run, ++load)
    {
        if (*run != *load)
        {
            boot->lma_ok = 0u;
        }
    }
    boot->bss_ok = 1u;
    for (const volatile uint32_t *word = _sbss; word < _ebss; ++word)
    {
        if (*word != 0u)
        {
            boot->bss_ok = 0u;
        }
    }
    uint32_t stack = (uint32_t)(uintptr_t)_estack;
    uint32_t entry = (uint32_t)(uintptr_t)bf_reset;
    boot->vectors_ok = (bf_read(UINT32_C(0x08000000)) == stack &&
                        bf_read(UINT32_C(0x08000004)) == entry)
                           ? 1u
                           : 0u;
}

void bf_boot_capture(uint32_t mode, struct bf_boot *boot)
{
    boot->csr = bf_read(BF_RCC_CSR);
    boot->dbgmcu_cr = bf_read(BF_DBGMCU_CR);
    check_startup(boot);
    boot->retained_valid =
        (retained.magic == BF_RETAINED_MAGIC && retained.mode == mode &&
         retained.check == retained_check())
            ? 1u
            : 0u;
    if (boot->retained_valid == 0u)
    {
        retained_reset(mode);
    }
    boot->fault_cfsr = retained.fault_cfsr;
    boot->fault_hfsr = retained.fault_hfsr;
    boot->fault_pc = retained.fault_pc;
    boot->unexpected_ipsr = retained.unexpected_ipsr;
    for (uint32_t i = 0u; i < BF_HISTORY; ++i)
    {
        boot->history[i] = retained.history[i];
    }
    for (uint32_t i = BF_HISTORY - 1u; i > 0u; --i)
    {
        retained.history[i] = retained.history[i - 1u];
    }
    retained.history[0] = boot->csr;
    retained.boots = retained.boots + 1u;
    boot->boots = retained.boots;
    retained.fault_cfsr = 0u;
    retained.fault_hfsr = 0u;
    retained.fault_pc = 0u;
    retained.unexpected_ipsr = 0u;
    retained.check = retained_check();
    bf_write(BF_RCC_CSR, bf_read(BF_RCC_CSR) | BF_RCC_CSR_RMVF);
}

void bf_cycles_enable(void)
{
    bf_write(BF_DEMCR, bf_read(BF_DEMCR) | BF_DEMCR_TRCENA);
    bf_write(BF_DWT_CYCCNT, 0u);
    bf_write(BF_DWT_CTRL, bf_read(BF_DWT_CTRL) | UINT32_C(1));
}

uint32_t bf_cycles(void)
{
    return bf_read(BF_DWT_CYCCNT);
}

void bf_delay_cycles(uint32_t cycles)
{
    uint32_t start = bf_cycles();
    while (bf_cycles() - start < cycles)
    {
    }
}

void bf_systick_start(uint32_t reload, uint32_t core_clock)
{
    bf_write(BF_SYST_CSR, 0u);
    bf_write(BF_SYST_RVR, reload);
    bf_write(BF_SYST_CVR, 0u);
    bf_write(BF_SYST_CSR, BF_SYST_ENABLE_TICKINT |
                              (core_clock != 0u ? BF_SYST_CLKSOURCE : 0u));
}

uint32_t bf_ticks(void)
{
    return ticks;
}

uint32_t bf_tick_cycles(void)
{
    return tick_cycles;
}

void bf_systick_handler(void)
{
    tick_cycles = bf_cycles();
    ticks = ticks + 1u;
}

_Noreturn void bf_software_reset(void)
{
    __asm__ volatile("dsb" ::: "memory");
    bf_write(BF_SCB_AIRCR, BF_AIRCR_SYSRESETREQ);
    __asm__ volatile("dsb" ::: "memory");
    for (;;)
    {
    }
}

void bf_exti9_wake_init(void)
{
    bf_write(BF_RCC_APB2ENR, bf_read(BF_RCC_APB2ENR) | BF_RCC_APB2ENR_SYSCFGEN);
    (void)bf_read(BF_RCC_APB2ENR);
    bf_write(BF_SYSCFG_EXTICR3,
             (bf_read(BF_SYSCFG_EXTICR3) & ~BF_EXTICR3_EXTI9_MASK) |
                 BF_EXTICR3_EXTI9_PORT_D);
    bf_write(BF_EXTI_RTSR, bf_read(BF_EXTI_RTSR) & ~BF_EXTI9);
    bf_write(BF_EXTI_FTSR, bf_read(BF_EXTI_FTSR) | BF_EXTI9);
    bf_write(BF_EXTI_PR, BF_EXTI9);
    bf_write(BF_EXTI_IMR, bf_read(BF_EXTI_IMR) | BF_EXTI9);
    bf_write(BF_RCC_APB1ENR, bf_read(BF_RCC_APB1ENR) | BF_RCC_APB1ENR_PWREN);
    (void)bf_read(BF_RCC_APB1ENR);
    bf_write(BF_NVIC_ISER0, UINT32_C(1) << BF_IRQ_EXTI9_5);
}

uint32_t bf_exti9_count(void)
{
    return exti9_events;
}

void bf_exti9_5_handler(void)
{
    if ((bf_read(BF_EXTI_PR) & BF_EXTI9) != 0u)
    {
        bf_write(BF_EXTI_PR, BF_EXTI9);
        exti9_events = exti9_events + 1u;
    }
}

void bf_stop_raw(void)
{
    uint32_t primask;
    __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(primask)::"memory");
    uint32_t pwr = bf_read(BF_PWR_CR);
    uint32_t scr = bf_read(BF_SCB_SCR);
    bf_write(BF_PWR_CR, pwr & ~BF_PWR_CR_STOP_BITS);
    bf_write(BF_SCB_SCR, scr | BF_SCR_SLEEPDEEP);
    __asm__ volatile("dsb\n\twfi\n\tnop\n\tnop\n\tnop" ::: "memory");
    bf_write(BF_SCB_SCR, scr);
    bf_write(BF_PWR_CR, pwr);
    __asm__ volatile("msr primask, %0" ::"r"(primask) : "memory");
}

/* A fault is unexpected in every harness mode: keep the evidence for the
 * next boot and stop. The frame is read only if it lies in SRAM. */
_Noreturn void bf_fault_record(const uint32_t *frame)
{
    uintptr_t at = (uintptr_t)frame;
    retained.fault_cfsr = bf_read(BF_SCB_CFSR);
    retained.fault_hfsr = bf_read(BF_SCB_HFSR);
    retained.fault_pc =
        (at >= UINT32_C(0x20000000) && at <= UINT32_C(0x2001F000) - 32u)
            ? frame[6]
            : UINT32_C(0xFFFFFFFF);
    retained.check = retained_check();
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

_Noreturn void bf_unexpected_record(uint32_t ipsr)
{
    retained.unexpected_ipsr = ipsr;
    retained.check = retained_check();
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
