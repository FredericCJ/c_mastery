#include "supervision.h"
#include "sleep.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static volatile uint32_t queued, rescue, mode;

static void text(const char *s)
{
    register uint32_t op __asm__("r0") = 4;
    register const char *arg __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(op) : "r"(arg) : "memory");
}

static _Noreturn void exit_guest(unsigned code)
{
    uint32_t block[] = {UINT32_C(0x20026), code};
    register uint32_t op __asm__("r0") = 0x20;
    register void *arg __asm__("r1") = block;
    __asm__ volatile("bkpt 0xab" : "+r"(op) : "r"(arg) : "memory");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

static _Noreturn void failed(void)
{
    text("CS_TARGET_CHECK:contract\n");
    exit_guest(1);
}

#define CS_STRING_(x) #x
#define CS_STRING(x) CS_STRING_(x)
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            text("CS_TARGET_CHECK:line=" CS_STRING(__LINE__) "\n"); \
            failed();                                               \
        }                                                           \
    } while (0)

static uint32_t mask(void)
{
    uint32_t n;
    __asm__ volatile("mrs %0,primask" : "=r"(n));
    return n;
}

static void irq8(void)
{
    *(volatile uint32_t *)UINT32_C(0x4000000c) = 1;
    *(volatile uint32_t *)UINT32_C(0x40000000) = 0;
    ++queued;
}

static void irq9(void)
{
    *(volatile uint32_t *)UINT32_C(0x4000100c) = 1;
    *(volatile uint32_t *)UINT32_C(0x40001000) = 0;
    ++rescue;
}

static void timer(unsigned index, unsigned count)
{
    volatile uint32_t *t =
        (volatile uint32_t *)(uintptr_t)(UINT32_C(0x40000000) + index * 4096);
    t[0] = 0;
    t[3] = 1;
    t[2] = count;
    t[0] = 9;
}

static void hook(unsigned stage)
{
    if (mode == 0 && stage == 0)
    {
        queued = 1;
    }
    if (mode == 1 && stage == 1)
    {
        *(volatile uint32_t *)UINT32_C(0xe000e200) = 1u << 8;
    }
    if (mode == 3 && stage == 1)
    {
        __asm__ volatile("sev" ::: "memory");
    }
}

static void prepare(void)
{
    __asm__ volatile("cpsid i" ::: "memory");
    *(volatile uint32_t *)UINT32_C(0x40000000) = 0;
    *(volatile uint32_t *)UINT32_C(0x40001000) = 0;
    *(volatile uint32_t *)UINT32_C(0x4000000c) = 1;
    *(volatile uint32_t *)UINT32_C(0x4000100c) = 1;
    *(volatile uint32_t *)UINT32_C(0xe000e280) = 0x300;
    queued = rescue = 0;
    *(volatile uint32_t *)UINT32_C(0xe000e100) = 0x300;
    *(volatile uint32_t *)UINT32_C(0xe000ed10) = 0;
    __asm__ volatile("cpsie i" ::: "memory");
}

static void exercise(void)
{
    struct cs_world world;
    CHECK(cs_init(&world, 2, 1) == CS_OK);
    CHECK(cs_progress(&world, 0) == CS_OK && cs_progress(&world, 1) == CS_OK &&
          cs_window(&world) == CS_OK);
    CHECK(cs_progress(&world, 0) == CS_OK && cs_window(&world) == CS_WAIT);
    CHECK(cs_progress(&world, 1) == CS_OK && cs_window(&world) == CS_OK);
    for (unsigned event = 0; event < 2; ++event)
    {
        for (unsigned interleave = 0; interleave < 3; ++interleave)
        {
            char label[] = "CS_TARGET_CASE:00\n";
            label[15] = (char)('0' + event);
            label[16] = (char)('0' + interleave);
            text(label);
            prepare();
            mode = interleave;
            timer(1, 1000000);
            if (interleave == 2)
            {
                timer(0, 10000);
            }
            unsigned attempts = 0;
            do
            {
                CHECK(cs_idle(event, &queued, hook) == 0);
                ++attempts;
            } while (!queued && !rescue && attempts < 1000000);
            CHECK(queued == 1 && rescue == 0 && mask() == 0);
            text("CS_TARGET_CASE_DONE\n");
        }
        /* Already-pending notification must survive admission; restore caller mask. */
        prepare();
        mode = 4;
        __asm__ volatile("cpsid i" ::: "memory");
        *(volatile uint32_t *)UINT32_C(0xe000e200) = 1u << 8;
        timer(1, 1000000);
        CHECK(cs_idle(event, &queued, hook) == 0 && mask() == 1 &&
              queued == 0 &&
              (*(volatile uint32_t *)UINT32_C(0xe000e200) & 256));
        __asm__ volatile("cpsie i\nisb" ::: "memory");
        CHECK(queued == 1 && rescue == 0);
    }
    text("CS_TARGET_SPURIOUS\n");
    prepare();
    mode = 3;
    timer(1, 1000000);
    CHECK(cs_idle(1, &queued, hook) == 0 && queued == 0 && rescue == 0);
    text("CS_TARGET_SPURIOUS_RETURN\n");
    mode = 2;
    timer(0, 10000);
    unsigned attempts = 0;
    do
    {
        CHECK(cs_idle(1, &queued, hook) == 0);
        ++attempts;
    } while (!queued && !rescue && attempts < 1000000);
    CHECK(queued == 1 && rescue == 0);
    /* QEMU deliberately makes SLEEPDEEP RAZ/WI: this is not a Stop test. */
    prepare();
    *(volatile uint32_t *)UINT32_C(0xe000ed10) = 4;
    CHECK((*(volatile uint32_t *)UINT32_C(0xe000ed10) & 4) == 0);
    *(volatile uint32_t *)UINT32_C(0xe000ed10) = 2;
    CHECK(cs_idle(0, &queued, 0) == 1);
    *(volatile uint32_t *)UINT32_C(0xe000ed10) = 0;
    __asm__ volatile("msr basepri,%0" ::"r"(128) : "memory");
    CHECK(cs_idle(0, &queued, 0) == 1);
    __asm__ volatile("msr basepri,%0" ::"r"(0) : "memory");
    text(
        "CS_TARGET_PASS:epochs=1,0,1 WFI-WFE=before,between,after pending=retained mask=restored spurious=rechecked deep=RAZ-WI sleep-on-exit=refused\n");
    exit_guest(0);
}

void cl_reset(void)
{
    uint32_t *s = &cl_data_load;
    for (uint32_t *d = &cl_data_start; d < &cl_data_end; ++d)
    {
        *d = *s++;
    }
    for (uint32_t *d = &cl_bss_start; d < &cl_bss_end; ++d)
    {
        *d = 0;
    }
    exercise();
}

__attribute__((section(".vectors"), used)) const uintptr_t vectors[26] = {
    [0] = (uintptr_t)&cl_stack_top, [1] = (uintptr_t)cl_reset,
    [2] = (uintptr_t)failed,        [3] = (uintptr_t)failed,
    [4] = (uintptr_t)failed,        [5] = (uintptr_t)failed,
    [6] = (uintptr_t)failed,        [24] = (uintptr_t)irq8,
    [25] = (uintptr_t)irq9};
