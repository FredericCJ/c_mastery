#include "cvm.h"
extern unsigned long __stack_top, __data_start, __data_end, __data_load,
    __bss_start, __bss_end;
static struct cvm_cvm vm;
static struct cvm_program program;

static void semihost(unsigned op, const void *arg)
{
    register unsigned r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}

static void stop(int status)
{
    unsigned args[2] = {0x20026u, (unsigned)status};
    semihost(0x20, args);
    for (;;)
    {
    }
}

void reset(void)
{
    /* The fixed ARM ELF/linker ABI defines these symbols as aligned addresses,
     * not as C objects in a common array. Compare integer addresses and access
     * each mapped word under that target-specific startup contract. */
    uintptr_t src = (uintptr_t)&__data_load, dst;
    struct cvm_value result;
    const uint8_t image[] = {'C', 'V', 'M',    '1', 1, 0, 0, 6,     0,
                             0,   0,   OP_INT, 42,  0, 0, 0, OP_RET};
    for (dst = (uintptr_t)&__data_start; dst < (uintptr_t)&__data_end;
         dst += sizeof(uint32_t), src += sizeof(uint32_t))
    {
        *(volatile uint32_t *)dst = *(const uint32_t *)src;
    }
    for (dst = (uintptr_t)&__bss_start; dst < (uintptr_t)&__bss_end;
         dst += sizeof(uint32_t))
    {
        *(volatile uint32_t *)dst = 0;
    }
    cvm_init(&vm, 100, 1000, 0);
    if (cvm_load(image, sizeof image, &program) != CVM_OK ||
        cvm_run(&vm, &program, &result) != CVM_OK || result.tag != CVM_INT ||
        result.integer != 42)
    {
        semihost(4, "CVM_THUMB_FAIL\n");
        stop(1);
    }
    {
        struct cvm_value a, b;
        if (cvm_pair(&vm, cvm_int(7), cvm_int(9), &a) != CVM_OK ||
            cvm_root_push(&vm, a) != CVM_OK || cvm_collect(&vm) != CVM_OK ||
            cvm_field(&vm, a, 1, &b) != CVM_OK || b.integer != 9)
        {
            semihost(4, "CVM_THUMB_GC_FAIL\n");
            stop(1);
        }
    }
    semihost(4, "CVM_THUMB_PASS value=42 rooted_pair=9\n");
    stop(0);
}

void fault(void)
{
    semihost(4, "CVM_THUMB_FAULT\n");
    stop(2);
}

__attribute__((section(".vectors"), used)) void (*const vectors[])(void) = {
    (void (*)(void))&__stack_top,
    reset,
    fault,
    fault,
    fault,
    fault,
    fault,
    0,
    0,
    0,
    0,
    fault,
    fault,
    0,
    fault,
    fault};

void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *a = d;
    const unsigned char *b = s;
    for (size_t i = 0; i < n; i++)
    {
        a[i] = b[i];
    }
    return d;
}

void *memset(void *d, int c, size_t n)
{
    unsigned char *a = d;
    for (size_t i = 0; i < n; i++)
    {
        a[i] = (unsigned char)c;
    }
    return d;
}
