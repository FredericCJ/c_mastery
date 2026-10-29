#include "boot.h"

static uint32_t read_word(void *c, uint32_t a)
{
    (void)c;
    return *(volatile uint32_t *)(uintptr_t)a;
}

static void write_word(void *c, uint32_t a, uint32_t v)
{
    (void)c;
    *(volatile uint32_t *)(uintptr_t)a = v;
}

static void sync_word(void *c)
{
    (void)c;
    __asm__ volatile("dsb sy" ::: "memory");
}

static void instruction_sync(void *c)
{
    (void)c;
    __asm__ volatile("isb sy" ::: "memory");
}

struct cb_io cb_target_io(void)
{
    struct cb_io x;
    x.registers.ctx = 0;
    x.registers.read = read_word;
    x.registers.write = write_word;
    x.registers.sync = sync_word;
    x.isb = instruction_sync;
    return x;
}
