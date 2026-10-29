#include "h745_flash.h"

static uint32_t read32(void *ctx, uint32_t a)
{
    (void)ctx;
    return *(volatile uint32_t *)(uintptr_t)a;
}

static void write32(void *ctx, uint32_t a, uint32_t v)
{
    (void)ctx;
    *(volatile uint32_t *)(uintptr_t)a = v;
}

static void sync32(void *ctx)
{
    (void)ctx;
    __asm__ volatile("dsb sy" ::: "memory");
}

struct fh_io fh_h745_io(void)
{
    struct fh_io io = {0, read32, write32, sync32};
    return io;
}
