#include "firmware.h"
#include <stdint.h>
#define W(a) (*(volatile uint32_t *)(uintptr_t)(a))
static volatile uint32_t initialized
    __attribute__((section(".data.special"), aligned(32))) = 0x7193a5c7u;
static volatile uint32_t tail __attribute__((section(".data.tail"))) =
    0x13579bdfu;
static volatile uint32_t zeroed;
static struct cf_firmware target_service;
static volatile uint32_t guard[8] __attribute__((aligned(32)));
static volatile unsigned fault_seen, copy_seen;
static unsigned char user_request[8] = {1, 2, 3, 4, 5, 6, 7, 8}, copied[8];
extern unsigned char cf_denied_instruction[], cf_denied_resume[];
extern void cf_user_store(volatile uint32_t *, uint32_t);
extern int cf_checks(unsigned);
static volatile unsigned irq_seen;

void cf_pendsv(void)
{
    struct cpf_sample sample = {73, 0xa5, 6};
    if (cf_acquire(&target_service, 1, &sample) == CF_OK)
        irq_seen = 1;
}

static int target_clock(void *context)
{
    (void)context;
    return 1;
}

static void emit(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void end(unsigned status)
{
    unsigned a[2] = {0x20026, status};
    register unsigned r0 __asm__("r0") = 0x20;
    register const void *r1 __asm__("r1") = a;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

void cf_trap(void)
{
    emit("CAP_MCU_TRAP\n");
    end(2);
}

void cf_fault_handler(uint32_t *words)
{
    if (W(0xe000ed28) != 0x82u ||
        words[6] != (uint32_t)(uintptr_t)cf_denied_instruction)
        cf_trap();
    words[6] = (uint32_t)(uintptr_t)cf_denied_resume;
    fault_seen = 1;
    W(0xe000ed28) = 0x82u;
}

void cf_svc_handler(uint32_t *words)
{
    /* The privilege boundary admits only this owned8-byte mailbox. No raw
     * request address is followed. DMA/other writers must be excluded here. */
    copy_seen =
        (unsigned)cf_copy_request(user_request, 8, words[0], words[1], copied);
    __asm__ volatile("movs r0,#0\n\tmsr control,r0\n\tisb" ::: "r0", "memory");
}

static int mpu(void)
{
    if (((W(0xe000ed90) >> 8) & 255) < 3)
        return 0;
    W(0xe000ed94) = 0;
    guard[0] = 17;
    W(0xe000ed98) = 0;
    W(0xe000ed9c) = 0;
    W(0xe000eda0) = (6u << 24) | (1u << 19) | (18u << 1) | 1u; /*512KiB RX*/
    W(0xe000ed98) = 1;
    W(0xe000ed9c) = 0x20000000u;
    W(0xe000eda0) =
        (3u << 24) | (1u << 28) | (1u << 19) | (15u << 1) | 1u; /*64KiB RW,XN*/
    W(0xe000ed98) = 2;
    W(0xe000ed9c) = (uint32_t)(uintptr_t)guard;
    W(0xe000eda0) = (1u << 24) | (1u << 28) | (1u << 19) | (4u << 1) |
                    1u; /*privileged-only32B*/
    W(0xe000ed24) |= 1u << 16;
    W(0xe000ed28) = 0x82u;
#ifndef CF_NO_MPU
    W(0xe000ed94) = 5;
#endif
    __asm__ volatile("dsb\n\tisb\n\tmovs r0,#1\n\tmsr control,r0\n\tisb" ::
                         : "r0", "memory");
    cf_user_store(guard, 99);
    user_request[0] = 9;
    unsigned control;
    __asm__ volatile("mrs %0,control" : "=r"(control));
    W(0xe000ed94) = 0;
    __asm__ volatile("dsb\n\tisb" ::: "memory");
    return fault_seen && guard[0] == 17 && copy_seen && copied[0] == 1 &&
           !(control & 1);
}

void cf_target_main(void)
{
    if (initialized != 0x7193a5c7u || tail != 0x13579bdfu || zeroed)
    {
        emit("CAP_MCU_STARTUP_REJECT\n");
        end(41);
    }
    if (!cf_init(&target_service))
    {
        emit("CAP_MCU_INIT_REJECT\n");
        end(41);
    }
    if (cf_boot(&target_service, target_clock, 0, 1) != CF_OK)
        cf_trap();
    W(0xe000ed04) = 1u << 28;
    __asm__ volatile("dsb\n\tisb" ::: "memory");
    for (unsigned n = 0; n < 1000 && !irq_seen; ++n)
        __asm__ volatile("nop");
    if (!irq_seen || cf_foreground(&target_service, 1) != CF_OK ||
        target_service.core.accepted != 1)
        cf_trap();
    for (unsigned i = 0; i < 8; ++i)
        if (cf_checks(i))
        {
            emit("CAP_MCU_CORE_REJECT\n");
            end(41);
        }
    if (!mpu())
    {
        emit("CAP_MCU_MPU_REJECT\n");
        end(41);
    }
    emit("CAP_MCU_PASS startup core DMA-model MPU-denial SVC-copy\n");
    end(0);
}
