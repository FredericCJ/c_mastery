/* NUCLEO-F439ZI board harness: one image per BF_MODE and variant.
 *
 * Images, predictions and the operator procedure are defined in
 * tools/f439_board.py (IMAGES) and reviews/v5/expansion/polish_revision/
 * physical/PLAN.md. This file prints raw observations only.
 *
 * Output contract: each line is "BF1 <TAG>" followed by space-separated
 * name=value fields (hex as 0x + 8 digits, otherwise decimal) and CR LF.
 * The reset clock is kept throughout: HSI 16 MHz, no PLL, AHB/APB1/APB2
 * prescalers 1, so APB1 (USART3) and the processor both run at 16 MHz
 * nominal. All bounds are poll or cycle counts, never physical-time claims.
 */
#include "bf_hw.h"
#include "bf_report.h"
#include "power.h"

#include <stdatomic.h>

#define BF_MODE_REPORT 1
#define BF_MODE_IWDG 2
#define BF_MODE_CLOCK 3
#define BF_MODE_STOP 4

#ifndef BF_MODE
#error "BF_MODE selects the experiment image"
#endif
#ifndef BF_BAUD
#define BF_BAUD 115200
#endif
#ifndef BF_STARVE_AFTER
#define BF_STARVE_AFTER 0
#endif
#ifndef BF_SYSTICK_DIV8
#define BF_SYSTICK_DIV8 0
#endif
#ifndef BF_STOP_DEBUG
#define BF_STOP_DEBUG 0
#endif

#define BF_TICK_10MS_RELOAD (BF_HSI_HZ / 100u - 1u)
#define BF_TICK_1S_RELOAD (BF_HSI_HZ - 1u)
#define BF_REPORT_TICKS 100u
#define BF_BURST_BYTES 11520u
#define BF_IWDG_PRESCALER 6u /* LSI / 256 */
#define BF_IWDG_RELOAD 511u
#define BF_IWDG_START_POLLS 100000u
#define BF_WINDOW_TICKS 50u
#define BF_STARVE_WINDOWS ((uint32_t)BF_STARVE_AFTER)
#define BF_PROBE_EVERY_TICKS 2u
#define BF_STOP_CYCLES 10u
#define BF_STOP_PREMISES 31u /* DMA, flash, APB, output and wake are owned */
#define BF_STOP_RAW_STATUS 255u
#define BF_AWAKE_CYCLES (BF_HSI_HZ / 2u)
#define BF_RCC_APB2ENR_SYSCFGEN (UINT32_C(1) << 14)
/* Commands are DLE (0x10) followed by a letter. Printable text such as a
 * host modem manager's AT probe can therefore never trigger a reset. */
#define BF_COMMAND_PREFIX 0x10u

_Noreturn void bf_main(void);

static void print_boot(const struct bf_boot *boot, int uart_status)
{
    static const char *const names[BF_HISTORY] = {"h0", "h1", "h2", "h3"};
    bf_put("BF1 BOOT");
    bf_field_dec("mode", (uint32_t)BF_MODE);
    bf_field_dec("baud", (uint32_t)BF_BAUD);
    bf_field_hex("csr", boot->csr);
    bf_field_dec("boots", boot->boots);
    bf_field_dec("ret", boot->retained_valid);
    bf_field_hex("dbg0", boot->dbgmcu_cr);
    for (uint32_t i = 0u; i < BF_HISTORY; ++i)
    {
        bf_field_hex(names[i], boot->history[i]);
    }
    bf_field_dec("uart", (uint32_t)uart_status);
    bf_line_end();

    bf_write(BF_RCC_APB2ENR, bf_read(BF_RCC_APB2ENR) | BF_RCC_APB2ENR_SYSCFGEN);
    (void)bf_read(BF_RCC_APB2ENR);
    bf_put("BF1 CHK");
    bf_field_dec("lma", boot->lma_ok);
    bf_field_dec("bss", boot->bss_ok);
    bf_field_dec("vec", boot->vectors_ok);
    bf_field_hex("v0", bf_read(UINT32_C(0x08000000)));
    bf_field_hex("v1", bf_read(UINT32_C(0x08000004)));
    bf_field_hex("vtor", bf_read(BF_SCB_VTOR));
    bf_field_hex("memrmp", bf_read(BF_SYSCFG_MEMRMP));
    bf_field_hex("cfsr", boot->fault_cfsr);
    bf_field_hex("hfsr", boot->fault_hfsr);
    bf_field_hex("fpc", boot->fault_pc);
    bf_field_hex("ipsr", boot->unexpected_ipsr);
    bf_line_end();

    bf_put("BF1 CLK");
    bf_field_hex("cr", bf_read(BF_RCC_CR));
    bf_field_hex("pll", bf_read(BF_RCC_PLLCFGR));
    bf_field_hex("cfgr", bf_read(BF_RCC_CFGR));
    bf_field_hex("acr", bf_read(BF_FLASH_ACR));
    bf_field_hex("pwr", bf_read(BF_PWR_CR));
    bf_field_hex("dbg", bf_read(BF_DBGMCU_CR));
    bf_field_hex("fz", bf_read(BF_DBGMCU_APB1_FZ));
    bf_field_hex("uid0", bf_read(BF_UID_BASE));
    bf_field_hex("uid1", bf_read(BF_UID_BASE + 4u));
    bf_field_hex("uid2", bf_read(BF_UID_BASE + 8u));
    bf_line_end();
}

/* E1/E2: periodic report, RX echo; DLE 'B' burst, DLE 'R' reprint,
 * DLE 'S' AIRCR software reset. */
static void burst(void)
{
    uint32_t start = bf_cycles();
    bf_put_repeated((uint8_t)'U', BF_BURST_BYTES);
    uint32_t elapsed = bf_cycles() - start;
    bf_put("BF1 BURST");
    bf_field_dec("bytes", BF_BURST_BYTES);
    bf_field_dec("cycles", elapsed);
    bf_line_end();
}

/* Returns 1 when `value` completes a DLE-prefixed command. */
static int command_byte(uint8_t value, uint8_t error, uint8_t letter,
                        uint32_t *armed)
{
    const int matched = *armed != 0u && error == 0u && value == letter ? 1 : 0;
    *armed = error == 0u && value == BF_COMMAND_PREFIX ? 1u : 0u;
    return matched;
}

static void handle_byte(const struct bf_boot *boot, uint8_t value,
                        uint8_t error)
{
    static uint32_t armed;
    const uint32_t was_armed = armed;
    if (command_byte(value, error, (uint8_t)'S', &armed) != 0)
    {
        bf_put("BF1 ACK S");
        bf_field_hex("cyc", bf_cycles());
        bf_line_end();
        bf_software_reset();
    }
    else if (was_armed != 0u && error == 0u && value == (uint8_t)'B')
    {
        burst();
    }
    else if (was_armed != 0u && error == 0u && value == (uint8_t)'R')
    {
        print_boot(boot, 0);
    }
    else if (armed != 0u)
    {
        /* DLE received: wait for the command letter without echoing. */
    }
    else
    {
        bf_put("BF1 RX");
        bf_field_hex("v", value);
        bf_field_hex("e", error);
        bf_field_hex("cyc", bf_cycles());
        bf_line_end();
    }
}

static _Noreturn void run_report(const struct bf_boot *boot)
{
    uint32_t seq = 0u;
    bf_systick_start(BF_TICK_10MS_RELOAD, 1u);
    uint32_t last = bf_ticks();
    for (;;)
    {
        uint8_t value;
        uint8_t error;
        if (bf_rx_poll(&value, &error) != 0)
        {
            handle_byte(boot, value, error);
        }
        if (bf_ticks() - last >= BF_REPORT_TICKS)
        {
            last += BF_REPORT_TICKS;
            ++seq;
            bf_put("BF1 R");
            bf_field_dec("seq", seq);
            bf_field_hex("csr", boot->csr);
            bf_field_dec("boots", boot->boots);
            bf_field_hex("dbg0", boot->dbgmcu_cr);
            bf_field_dec("tx", bf_tx_failures());
            bf_field_dec("ticks", bf_ticks());
            bf_field_hex("cyc", bf_cycles());
            bf_line_end();
        }
    }
}

/* E3: two progress epochs; the window feeds the IWDG only when both tasks
 * committed work. A starving image stops task 1 while SysTick keeps running,
 * so a live periodic interrupt alone never feeds the watchdog. */
static void print_window(struct cs_world *world, enum cs_status start,
                         uint32_t window, uint32_t starving, enum cs_status fed)
{
    bf_put("BF1 W");
    bf_field_dec("start", (uint32_t)start);
    bf_field_dec("win", window);
    bf_field_dec("ticks", bf_ticks());
    bf_field_dec("e0", atomic_load(&world->epoch[0]));
    bf_field_dec("e1", atomic_load(&world->epoch[1]));
    bf_field_dec("feed", (uint32_t)fed);
    bf_field_dec("starving", starving);
    bf_field_hex("sr", bf_read(BF_IWDG_SR));
    bf_field_hex("cyc", bf_cycles());
    bf_line_end();
}

static _Noreturn void run_iwdg(void)
{
    static struct cs_world world;
    const struct cs_bus bus = cs_f439_bus();
    enum cs_status init = cs_init(&world, 2u, 1u);
    enum cs_status start = cs_iwdg_start(&bus, BF_IWDG_PRESCALER,
                                         BF_IWDG_RELOAD, BF_IWDG_START_POLLS);
    bf_put("BF1 W START");
    bf_field_dec("init", (uint32_t)init);
    bf_field_dec("start", (uint32_t)start);
    bf_field_dec("starve_after", BF_STARVE_WINDOWS);
    bf_field_hex("sr", bf_read(BF_IWDG_SR));
    bf_field_hex("cyc", bf_cycles());
    bf_line_end();
    bf_systick_start(BF_TICK_10MS_RELOAD, 1u);
    uint32_t last = bf_ticks();
    uint32_t window_start = last;
    uint32_t window = 0u;
    uint32_t starving = 0u;
    for (;;)
    {
        uint32_t now = bf_ticks();
        if (now == last)
        {
            continue;
        }
        last = now;
        (void)cs_progress(&world, 0u);
        if (starving == 0u)
        {
            (void)cs_progress(&world, 1u);
        }
        if (now - window_start < BF_WINDOW_TICKS)
        {
            continue;
        }
        window_start += BF_WINDOW_TICKS;
        ++window;
        if (BF_STARVE_WINDOWS != 0u && window > BF_STARVE_WINDOWS)
        {
            starving = 1u;
        }
        print_window(&world, start, window, starving,
                     cs_iwdg_feed(&world, &bus));
    }
}

/* E4a/b: SysTick period against DWT CYCCNT, then WFE/WFI sleep probes. */
static void tick_snapshot(uint32_t *ticks, uint32_t *cycles)
{
    uint32_t before;
    do
    {
        before = bf_ticks();
        *cycles = bf_tick_cycles();
        *ticks = bf_ticks();
    } while (before != *ticks);
}

/* DBGMCU_CR is changed only for the duration of one sleep and restored
 * immediately on wake, so a debugger attach outside a probe never meets a
 * core sleeping with DBG_SLEEP cleared. */
static void probe_sleep(uint32_t probe)
{
    const uint32_t use_wfi = probe & 1u;
    const uint32_t debug = ((probe >> 1) & 1u) != 0u ? BF_DBGMCU_SLEEP : 0u;
    const uint32_t saved = bf_read(BF_DBGMCU_CR);
    const uint32_t during = (saved & ~BF_DBGMCU_SLEEP) | debug;
    bf_write(BF_DBGMCU_CR, during);
    uint32_t start = bf_cycles();
    __asm__ volatile("sev\n\twfe" ::: "memory");
    const uint32_t latched = bf_cycles() - start;
    const uint32_t remaining = bf_read(BF_SYST_CVR);
    start = bf_cycles();
    if (use_wfi != 0u)
    {
        __asm__ volatile("wfi" ::: "memory");
    }
    else
    {
        __asm__ volatile("wfe" ::: "memory");
    }
    const uint32_t slept = bf_cycles() - start;
    bf_write(BF_DBGMCU_CR, saved);
    bf_put("BF1 E");
    bf_field_dec("probe", probe);
    bf_field_dec("wfi", use_wfi);
    bf_field_hex("dbg", during);
    bf_field_hex("restored", bf_read(BF_DBGMCU_CR));
    bf_field_dec("latched", latched);
    bf_field_dec("cvr", remaining);
    bf_field_dec("slept", slept);
    bf_field_dec("ticks", bf_ticks());
    bf_line_end();
}

/* DLE 'P' starts the sleep probes, DLE 'Q' stops them; returns the new
 * probing state. */
static uint32_t probe_command(uint8_t value, uint8_t error, uint32_t *armed,
                              uint32_t probing)
{
    const uint32_t was_armed = *armed;
    *armed = error == 0u && value == BF_COMMAND_PREFIX ? 1u : 0u;
    if (was_armed != 0u && error == 0u && value == (uint8_t)'P')
    {
        return 1u;
    }
    if (was_armed != 0u && error == 0u && value == (uint8_t)'Q')
    {
        return 0u;
    }
    return probing;
}

static _Noreturn void run_clock(void)
{
    bf_systick_start(BF_TICK_1S_RELOAD, BF_SYSTICK_DIV8 != 0 ? 0u : 1u);
    bf_put("BF1 T START");
    bf_field_hex("syst", bf_read(BF_SYST_CSR));
    bf_field_dec("reload", bf_read(BF_SYST_RVR));
    bf_field_hex("cyc", bf_cycles());
    bf_line_end();
    uint32_t last_ticks = 0u;
    uint32_t last_cycles = 0u;
    uint32_t probe = 0u;
    uint32_t probing = 0u;
    uint32_t armed = 0u;
    for (;;)
    {
        uint8_t value;
        uint8_t error;
        if (bf_rx_poll(&value, &error) != 0)
        {
            const uint32_t next = probe_command(value, error, &armed, probing);
            if (next != probing)
            {
                probing = next;
                bf_put("BF1 E STATE");
                bf_field_dec("probing", probing);
                bf_field_hex("dbg", bf_read(BF_DBGMCU_CR));
                bf_line_end();
            }
        }
        uint32_t ticks;
        uint32_t cycles;
        tick_snapshot(&ticks, &cycles);
        if (ticks == last_ticks)
        {
            continue;
        }
        bf_put("BF1 T");
        bf_field_dec("n", ticks);
        bf_field_hex("cyc", cycles);
        bf_field_dec("d", cycles - last_cycles);
        bf_field_dec("skipped", ticks - last_ticks - 1u);
        bf_line_end();
        last_ticks = ticks;
        last_cycles = cycles;
        if (probing != 0u && ticks % BF_PROBE_EVERY_TICKS == 0u)
        {
            probe_sleep(probe);
            ++probe;
        }
    }
}

/* E4c: host-woken Stop cycles. The image waits for DLE 'G' so a debugger can
 * attach after flashing, runs BF_STOP_CYCLES cycles, then stays awake. */
static void stop_status_line(const char *tag)
{
    bf_put(tag);
    bf_field_hex("dbg", bf_read(BF_DBGMCU_CR));
    bf_field_dec("exti", bf_exti9_count());
    bf_field_hex("cyc", bf_cycles());
    bf_line_end();
}

static void wait_for_go(void)
{
    uint32_t armed = 0u;
    for (;;)
    {
        stop_status_line("BF1 S READY");
        const uint32_t start = bf_cycles();
        while (bf_cycles() - start < BF_HSI_HZ)
        {
            uint8_t value;
            uint8_t error;
            if (bf_rx_poll(&value, &error) != 0 &&
                command_byte(value, error, (uint8_t)'G', &armed) != 0)
            {
                return;
            }
        }
    }
}

static void stop_cycle(const struct cs_bus *bus, uint32_t cycle)
{
    struct cs_stop stop;
    uint32_t entered = BF_STOP_RAW_STATUS;
    uint32_t finished = BF_STOP_RAW_STATUS;
    (void)cs_stop_init(&stop);
    bf_put("BF1 S ENTER");
    bf_field_dec("n", cycle);
    bf_field_hex("dbg", bf_read(BF_DBGMCU_CR));
    bf_field_dec("exti", bf_exti9_count());
    bf_field_hex("cyc", bf_cycles());
    bf_line_end(); /* returns after TC: the output premise holds */
    (void)bf_read(BF_USART3_SR);
    (void)bf_read(BF_USART3_DR);
    bf_write(BF_EXTI_PR, BF_EXTI9);
    const uint32_t start = bf_cycles();
    if (BF_STOP_DEBUG == 0)
    {
        entered = (uint32_t)cs_f439_stop(&stop, BF_STOP_PREMISES);
        finished = (uint32_t)cs_stop_finish(&stop, bus, 0u, 0u, 0u);
    }
    else
    {
        bf_stop_raw();
    }
    const uint32_t elapsed = bf_cycles() - start;
    const uint32_t sr = bf_read(BF_USART3_SR);
    const uint32_t dr = bf_read(BF_USART3_DR);
    bf_put("BF1 S WOKE");
    bf_field_dec("n", cycle);
    bf_field_dec("entered", entered);
    bf_field_dec("finished", finished);
    bf_field_dec("d", elapsed);
    bf_field_hex("sr", sr);
    bf_field_hex("dr", dr);
    bf_field_dec("exti", bf_exti9_count());
    bf_field_hex("cr", bf_read(BF_RCC_CR));
    bf_field_hex("cfgr", bf_read(BF_RCC_CFGR));
    bf_line_end();
}

static _Noreturn void run_stop(void)
{
    const struct cs_bus bus = cs_f439_bus();
    bf_write(BF_DBGMCU_CR, BF_STOP_DEBUG != 0 ? BF_DBGMCU_STOP : 0u);
    bf_exti9_wake_init();
    wait_for_go();
    for (uint32_t cycle = 1u; cycle <= BF_STOP_CYCLES; ++cycle)
    {
        stop_cycle(&bus, cycle);
        bf_delay_cycles(BF_AWAKE_CYCLES);
    }
    for (;;)
    {
        stop_status_line("BF1 S DONE");
        bf_delay_cycles(BF_HSI_HZ);
    }
}

_Noreturn void bf_main(void)
{
    struct bf_boot boot;
    bf_boot_capture((uint32_t)BF_MODE, &boot);
    bf_cycles_enable();
    if (BF_MODE == BF_MODE_REPORT)
    {
        /* Park/report image: keep debug access through later images that
         * end in WFI, independent of power-on history (dbg0 keeps the
         * value found at boot). */
        bf_write(BF_DBGMCU_CR, bf_read(BF_DBGMCU_CR) | BF_DBGMCU_SLEEP);
    }
    const int uart_status = bf_uart_open((uint32_t)BF_BAUD);
    print_boot(&boot, uart_status);
    switch (BF_MODE)
    {
        case BF_MODE_REPORT:
            run_report(&boot);
        case BF_MODE_IWDG:
            run_iwdg();
        case BF_MODE_CLOCK:
            run_clock();
        case BF_MODE_STOP:
            run_stop();
        default:
            for (;;)
            {
                __asm__ volatile("wfi");
            }
    }
}
